/**************************************************************************************************
  Filename:       UsbFunctions.c


  Copyright 2014 Paolo Achdjian

**************************************************************************************************/

/*********************************************************************
 * INCLUDES
 */

#include "osal.h"
#include "AddrMgr.h"
#include "OSAL_Memory.h"
#include "UsbFunctions.h"
#include "usb_framework.h"
#include "usb_interrupt.h"
#include "usb_suspend.h"
#include "zcl.h"
#include "string.h"
#include "stdio.h"

#include "hal_usbdongle_cfg.h"
#include "TimerEvents.h"
#include "UsbFifoData.h"
#include "DeviceManager.h"
#include "OSAL_Memory.h"
 


/*********************************************************************
 * LOCAL CONSTANTS
 */



#define FIFO_SIZE    5
#define MAX_CLUSTERS 20
#define MAX_DATA_SIZE 40
	 
	 
extern byte zusbTaskId;

/*********************************************************************
 * LOCAL TYPES
 */
struct AnnunceDataMsg {
	struct GenericDataMsg genericDataMsg;
	uint16      nwkAddr;
 	uint8       extAddr[Z_EXTADDR_LEN];
 	uint8       capabilities;
};

struct SimpleDescrMsg {
	struct GenericDataMsg genericDataMsg;
	uint16      nwkAddr;
	uint8       endpoint;
	uint16      appProfId;
	uint16      appDeviceId;
	uint8		appDevVer;
	uint8       numInClusters;
	uint8		numOutClusters;
	uint16		clustersList[MAX_CLUSTERS];
};

struct AttributeResponse {
	uint16 attrID;
	uint8 status;
	uint8 dataType;
	uint8 data[];
};

struct ReadAttributeResponseMsg {
	struct GenericDataMsg generticDataMsg;
	uint8 type;
	uint16 clusterId;
	uint16 networkAddr;
	uint8 endpoint;
	uint16 panId;
	uint8  numAttributes;
	struct AttributeResponse attributes[];
};

struct ReadAttributeResponseErrorMsg {
	struct GenericDataMsg generticDataMsg;
	uint16 networkAddr;
	uint8 endpoint;
	uint16 clusterId;
	uint8  zStatus;
	uint8  attrCount;
	uint16 attrID[];
};

struct ActiveEPReqErrorMsg {
	struct GenericDataMsg generticDataMsg;
	uint16 networkAddr;
	uint8  errorCode;
};

struct InfoMessage {
	struct GenericDataMsg generticDataMsg;
	uint16 nwkAddr;
};


/*********************************************************************
 * MACROS
 */
#define ANNUNCE_SEND_TIMEOUT   100     // Every 100 ms
#define SEND_FIFO_DATA_TIME   100     // Every 100 ms

/*********************************************************************
 * LOCAL TYPEDEFS
 */




/*********************************************************************
 * LOCAL VARIABLES
 */
static uint8 * tmpData;
struct AnnunceDataMsg annunceDataMsg;
static uint16 * dest;
static uint16 * maxDest;
static uint16 * src;
static uint8 len;
static struct SimpleDescrMsg * simpleDescrMsg;
uint16  currentDeviceElement=0;
static struct UsbFifoData * usbFifoHead;



/*********************************************************************
 * LOCAL FUNCTIONS
 */

//static bool usbOutProcess(void);

/*********************************************************************
 * PUBLIC FUNCTIONS
 */
void Usb_ProcessLoop(void) {

	// Handle reset signaling on the bus
	if (USBIRQ_GET_EVENT_MASK() & USBIRQ_EVENT_RESET) {
		USBIRQ_CLEAR_EVENTS(USBIRQ_EVENT_RESET);
		usbfwResetHandler();
	}

	// Handle packets on EP0
	if (USBIRQ_GET_EVENT_MASK() & USBIRQ_EVENT_SETUP)  {
		USBIRQ_CLEAR_EVENTS(USBIRQ_EVENT_SETUP);
		usbfwSetupHandler();
	}

	// Handle USB suspend
	if (USBIRQ_GET_EVENT_MASK() & USBIRQ_EVENT_SUSPEND)  {
		// Clear USB suspend interrupt
		USBIRQ_CLEAR_EVENTS(USBIRQ_EVENT_SUSPEND);

	    // Take the chip into PM1 until a USB resume is deteceted.
    	usbsuspEnter();
		
    	// Running again; first clear RESUME interrupt
    	USBIRQ_CLEAR_EVENTS(USBIRQ_EVENT_RESUME);
	}
	
}



/***********************************************************************************
* @brief        send data from fifo to usb, if possibile
*
* @param        none
*
* @return       none
*/
void sendFifo(void) {
	HAL_DISABLE_INTERRUPTS();
	usbFifoHead = getUsbFifoHead();
	if (usbFifoHead != NULL && USBFW_IN_ENDPOINT_DISARMED()){
		uint8 oldEndpoint = USBFW_GET_SELECTED_ENDPOINT();
		USBFW_SELECT_ENDPOINT(5);

		tmpData =    usbFifoHead->data;
		len = usbFifoHead->dataLen;
		if (len > MAX_DATE_SIZE_5){
			len = MAX_DATE_SIZE_5;
		}
		do {
       		USBF5 = *tmpData;
			tmpData++;
			len--;
   		} while (len>0);
   		USBFW_ARM_IN_ENDPOINT();
		usbFifoPop();
 		USBFW_SELECT_ENDPOINT(oldEndpoint);	
	}
	HAL_ENABLE_INTERRUPTS();
}

/**
* send a chunk of data to the host at endpoint 2
*/
void sendUsb(const uint8 * data, uint8 len) {
	HAL_DISABLE_INTERRUPTS();
	if (isFifoEmpty()){
		uint8 oldEndpoint = USBFW_GET_SELECTED_ENDPOINT();
		USBFW_SELECT_ENDPOINT(5);
		
		// If the IN endpoint is ready to accept data.
		if (len > MAX_DATE_SIZE_5){
			len = MAX_DATE_SIZE_5;
		}
		if (USBFW_IN_ENDPOINT_DISARMED()) {
			do {
				USBF5 = *data;
				data++;
				len--;
			} while (len>0);
			USBFW_ARM_IN_ENDPOINT();
		} else {
			usbFifoDataPush(data, len);
		}
		USBFW_SELECT_ENDPOINT(oldEndpoint);
	} else {
		usbFifoDataPush(data, len);
	}
	HAL_ENABLE_INTERRUPTS();
}

void usbLog(uint16 nwkAddr, const char * msg, ...) {
	char * buffer = (char *) osal_mem_alloc(128);
	char * iter = buffer;
	va_list args;
	va_start (args, format);
	vsprintf (buffer, msg, args);
	va_end (args);
	
	nwkAddr = osal_heap_block_cnt();
	
	int8 len = strlen(buffer)+1;
	if (len > ENDPOINT_LOG_SIZE-3){
		len = ENDPOINT_LOG_SIZE-3;
	}

	uint8 oldEndpoint = USBFW_GET_SELECTED_ENDPOINT();
	USBFW_SELECT_ENDPOINT(ENDPOINT_LOG);
		
	while(!USBFW_IN_ENDPOINT_DISARMED());
	USB_LOG_FIFO = INFO_MESSAGE;
	USB_LOG_FIFO = ((char *)(&nwkAddr))[0];
	USB_LOG_FIFO = ((char *)(&nwkAddr))[1];
	for(; len >= 0; len --){
		USB_LOG_FIFO = *iter;
		iter++;
	}
	USBFW_ARM_IN_ENDPOINT();
    USBFW_SELECT_ENDPOINT(oldEndpoint);
	osal_mem_free(buffer);
}

/**
  send to the usb host the network address of the device annunced
*/

void usbSendAnnunce(ZDO_DeviceAnnce_t * device) {
	annunceDataMsg.genericDataMsg.msgCode = ANNUNCE;
	annunceDataMsg.nwkAddr = device->nwkAddr;
	osal_memcpy(annunceDataMsg.extAddr, device->extAddr, Z_EXTADDR_LEN);
	annunceDataMsg.capabilities = device->capabilities;
	sendUsb((uint8 *)&annunceDataMsg, sizeof(annunceDataMsg));
}

void usbSendSimpleDescriptor(ZDO_SimpleDescRsp_t * simpleDesc) {
	simpleDescrMsg = osal_mem_alloc(sizeof(struct  SimpleDescrMsg));
	if (simpleDescrMsg != NULL){
		simpleDescrMsg->genericDataMsg.msgCode= SIMPLE_DESC;
		simpleDescrMsg->nwkAddr = simpleDesc->nwkAddr;
		simpleDescrMsg->endpoint = simpleDesc->simpleDesc.EndPoint;
		simpleDescrMsg->appProfId = simpleDesc->simpleDesc.AppProfId;
		simpleDescrMsg->appDeviceId = simpleDesc->simpleDesc.AppDeviceId;
		simpleDescrMsg->appDevVer = simpleDesc->simpleDesc.AppDevVer;
		simpleDescrMsg->numInClusters = simpleDesc->simpleDesc.AppNumInClusters;
		simpleDescrMsg->numOutClusters = simpleDesc->simpleDesc.AppNumOutClusters;
		dest = simpleDescrMsg->clustersList;
		maxDest = dest + MAX_CLUSTERS;
		src = simpleDesc->simpleDesc.pAppInClusterList;
		len = simpleDesc->simpleDesc.AppNumInClusters;
		while (len>0 && dest < maxDest){
			*dest++ = *src++;
			len--;
		}
		src = simpleDesc->simpleDesc.pAppOutClusterList;
		len = simpleDesc->simpleDesc.AppNumOutClusters;
		while (len>0&& dest < maxDest){
			*dest++ = *src++;
			len--;
		}
		sendUsb((uint8 *)simpleDescrMsg, sizeof(struct SimpleDescrMsg));
		osal_mem_free(simpleDescrMsg);
	}
}

void usbSendAttributeResponseMsgError(struct ReqAttributeMsg * attributesValue, ZStatus_t status){
	struct ReadAttributeResponseErrorMsg  * response;
	
	uint8 dataSize = sizeof(struct ReadAttributeResponseErrorMsg)+2*attributesValue->numAttr;
	response = osal_mem_alloc(dataSize);
	if (response != NULL){
		response->clusterId = attributesValue->cluster;
		response->endpoint = attributesValue->afAddrType.endPoint;
		response->networkAddr = attributesValue->afAddrType.addr.shortAddr;
		response->zStatus = status;
		response->attrCount = attributesValue->numAttr;
		response->generticDataMsg.msgCode = ATTRIBUTE_VALUE_REQ_ERROR;
		osal_memcpy(response->attrID, attributesValue->attrID, 2*attributesValue->numAttr);
		sendUsb((const uint8 *)response,dataSize);
		
		osal_mem_free(response);
	}
}

void usbSendActiveEPError(uint16 nwkAddr, uint8 errorCode) {
	struct ActiveEPReqErrorMsg  errorMsg;
	errorMsg.generticDataMsg.msgCode = ACTIVE_EP_REQ_ERROR;
	errorMsg.networkAddr = nwkAddr;
	errorMsg.errorCode = errorCode;
	sendUsb( (const uint8 *)&errorMsg, sizeof(struct ActiveEPReqErrorMsg ) );
}

void usbSendAttributeResponseMsg(zclReadRspCmd_t * readRspCmd, uint16 cluster, afAddrType_t * address ) {
	zclReadRspStatus_t * iter = readRspCmd->attrList;
	zclReadRspStatus_t * iterSend = readRspCmd->attrList;
	zclReadRspStatus_t * iterEnd = readRspCmd->attrList+readRspCmd->numAttr;
	struct AttributeResponse  * attributeResponse;
	uint16 dataSize = sizeof(struct ReadAttributeResponseMsg);
	uint8  i;
	uint8	 attrSize;
	uint8  tmpNumAttributes=0;
	uint16 tmpDataSize=0;
	usbLog(0,"usbSendAttributeResponseMsg");
	
	for (; iter < iterEnd; iter++){
		attrSize = zclGetAttrDataLength(iter->dataType, iter->data);
		tmpDataSize =dataSize +attrSize+sizeof(struct AttributeResponse);
		if (tmpDataSize > MAX_DATE_SIZE_5){
			if (tmpNumAttributes>0){
				struct ReadAttributeResponseMsg * response = osal_mem_alloc(dataSize);
				if (response == NULL){
					return;
				}
				response->generticDataMsg.msgCode= ATTRIBUTE_VALUES;
				response->clusterId=cluster;
				response->networkAddr =address->addr.shortAddr;
				response->endpoint =address->endPoint;
				response->panId  = address->panId;
				response->numAttributes = tmpNumAttributes;
				attributeResponse = response->attributes;
				for (i=0; i < tmpNumAttributes; i++){
					attrSize = zclGetAttrDataLength(iterSend->dataType, iterSend->data);
					attributeResponse->attrID = iterSend->attrID;
					attributeResponse->dataType = iterSend->dataType;
					attributeResponse->status = iterSend->status;
					osal_memcpy(&attributeResponse->data, iterSend->data, attrSize);
					attributeResponse = (struct AttributeResponse *)(((uint8 *)attributeResponse) + sizeof(struct AttributeResponse) + attrSize);
					iterSend++;
				}
				sendUsb((const uint8 *)response, dataSize);
				osal_mem_free(response);
				tmpNumAttributes=1;
				dataSize =  sizeof(struct ReadAttributeResponseMsg) + attrSize;
			}
		} else {
			tmpNumAttributes++;
			dataSize = tmpDataSize;
		}
	}
	struct ReadAttributeResponseMsg * response = osal_mem_alloc(dataSize);
	if (response == NULL){
		return;
	}
	response->generticDataMsg.msgCode= ATTRIBUTE_VALUES;
	response->clusterId=cluster;
	response->networkAddr =address->addr.shortAddr;
	response->endpoint =address->endPoint;
	response->panId  = address->panId;
	response->numAttributes = tmpNumAttributes;
	attributeResponse = response->attributes;
	for (i=0; i < tmpNumAttributes; i++){
		attrSize = zclGetAttrDataLength(iterSend->dataType, iterSend->data);
		attributeResponse->attrID = iterSend->attrID;
		attributeResponse->dataType = iterSend->dataType;
		attributeResponse->status = iterSend->status;
		osal_memcpy(&attributeResponse->data, iterSend->data, attrSize);
		attributeResponse = (struct AttributeResponse *)(((uint8 *)attributeResponse) + sizeof(struct AttributeResponse) + attrSize);
		iterSend++;
	}
	sendUsb((const uint8 *)response, dataSize);
	osal_mem_free(response);
}

void requestAllDevices2(uint8 * notUsed){
	if (currentDeviceElement < DEVICE_MANAGER_TABLE_SIZE){
		while(currentDeviceElement < DEVICE_MANAGER_TABLE_SIZE){
			ZDO_DeviceAnnce_t * device = deviceEntryGet(currentDeviceElement);
			if (device != NULL){
				usbLog(0,"Find device %X from device Entry",device->nwkAddr );
				annunceDataMsg.genericDataMsg.msgCode = ANNUNCE;
				annunceDataMsg.nwkAddr = device->nwkAddr;
				sAddrExtCmp(annunceDataMsg.extAddr, device->extAddr);
				annunceDataMsg.capabilities = device->capabilities;
				sendUsb((uint8 *)&annunceDataMsg, sizeof(annunceDataMsg));
				osal_start_timerEx( zusbTaskId, USB_ANNUNCE2_MSG, ANNUNCE_SEND_TIMEOUT );
				currentDeviceElement++;
				return;
			}
			currentDeviceElement++;
		}
	} else {
		osal_stop_timerEx(zusbTaskId, USB_ANNUNCE2_MSG);
		currentDeviceElement =0;
	}
}

void requestAllDevices(void){
	usbLog(0,"Request all devices");
	if (currentDeviceElement < NWK_MAX_ADDRESSES){
		while(1){
			AddrMgrEntry_t addrMgrEntry; 
			addrMgrEntry.index=currentDeviceElement;
			addrMgrEntry.user=ADDRMGR_USER_DEFAULT;
			
			if (AddrMgrEntryGet(&addrMgrEntry)==TRUE && addrMgrEntry.nwkAddr != 0xFFFF){
				usbLog(0, "find device %X into addrMgr", addrMgrEntry.nwkAddr);
				annunceDataMsg.genericDataMsg.msgCode = ANNUNCE;
				annunceDataMsg.nwkAddr = addrMgrEntry.nwkAddr;
				osal_memcpy(annunceDataMsg.extAddr, addrMgrEntry.extAddr, Z_EXTADDR_LEN);
				annunceDataMsg.capabilities = 0;
				sendUsb((uint8 *)&annunceDataMsg, sizeof(annunceDataMsg));
				osal_start_timerEx( zusbTaskId, USB_ANNUNCE_MSG, ANNUNCE_SEND_TIMEOUT );
				currentDeviceElement++;
				return;
			} else{
				if (currentDeviceElement >=NWK_MAX_ADDRESSES){
					currentDeviceElement=0;
					osal_start_timerEx( zusbTaskId, USB_ANNUNCE2_MSG, ANNUNCE_SEND_TIMEOUT );
					return;
				}
			}
			currentDeviceElement++;
		}
	} else {
		osal_stop_timerEx(zusbTaskId, USB_ANNUNCE_MSG);
		currentDeviceElement =0;
	}
}



char * clusterRequestToString(uint16 clusterId){
	switch(clusterId){
	case NWK_addr_req:
		return "NWKAddrReq";
	case IEEE_addr_req:
		return "IEEEAddrReq";
	case Simple_Desc_req:
		return "SimpleDecReq";
	case Active_EP_req:
		return "ActiveEpReq";
	case Device_annce:
		return "Device_annce";
	case End_Device_Timeout_req:
		return "End_Device_Timeout_req";
	case End_Device_Bind_req:
		return "End_Device_Bind_req";
	case Bind_req:
		return "Bind_req";
	case Unbind_req:
		return "Unbind_req";
		
	case NWK_addr_rsp:
		return "NWK_addr_rsp";
	case IEEE_addr_rsp:
		return "IEEE_addr_rsp";
	case Simple_Desc_rsp:
		return "Simple_Desc_rsp";
	case End_Device_Timeout_rsp:
		return "End_Device_Timeout_rsp";
	case Bind_rsp:
		return "Bind_rsp";
	case End_Device_Bind_rsp:
		return "End_Device_Bind_rsp";
	case Unbind_rsp:
		return "Unbind_rsp";
	case Mgmt_Bind_req:
		return "MgmtBindReq";
	case Mgmt_Bind_rsp:
		return "MgmtBindRsp";
	default:
		return convertUint16ToHex(clusterId);
	};
}

static char convertDigit(uint8 digit){
	digit &= 0x0F;
	if (digit <= 9)
		return '0' + digit;
	else
		return 'A' + digit;
}

char * convertUint16ToHex(uint16 num) {
	static char unknownValueBuffer[5];
	unknownValueBuffer[4] = 0;
	unknownValueBuffer[3] = convertDigit(num & 0x000F);
	num = num >> 4;
	unknownValueBuffer[2] = convertDigit(num & 0x000F);
	num = num >> 4;
	unknownValueBuffer[1] = convertDigit(num & 0x000F);
	num = num >> 4;
	unknownValueBuffer[0] = convertDigit(num & 0x000F);
	return unknownValueBuffer;
}


