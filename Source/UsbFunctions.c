/**************************************************************************************************
  Filename:       UsbFunctions.c


  Copyright 2014 Paolo Achdjian

**************************************************************************************************/

/*********************************************************************
 * INCLUDES
 */

#include "osal.h"
#include "OSAL_Memory.h"
#include "UsbFunctions.h"
#include "usb_framework.h"
#include "usb_interrupt.h"
#include "usb_suspend.h"
#include "zcl.h"

#include "UsbMessageHandlers.h"
#include "hal_usbdongle_cfg.h"
#include "TimerEvents.h"
#include "UsbFifoData.h"
 


/*********************************************************************
 * LOCAL CONSTANTS
 */
#define Z_EXTADDR_LEN   8
#define ANNUNCE 0x01
#define REQ_SIMPLE_DESC 0x02
#define SIMPLE_DESC 0x03
#define REQ_ACTIVE_EP 0x04
#define ACTIVE_EP 0x05
#define REQ_ATTRIBUTE_VALUES 0x06
#define ATTRIBUTE_VALUES 0x07
#define SEND_CMD 0x08
#define WRITE_ATTRIBUTE_VALUE 0x09
#define REQ_ALL_NODES 0x0A
#define ALL_NODES 0x0B
#define REQ_BIND_TABLE 0x0C
#define BIND_TABLE 0x0D
#define REQ_ADD_BIND_TABLE_ENTRY 0x0E
#define REQ_REMOVE_BIND_TABLE_ENTRY 0x0F


#define FIFO_SIZE    5
#define MAX_CLUSTERS 20
#define MAX_DATA_SIZE 40
	 
	 
extern byte zusbTaskId;

/*********************************************************************
 * LOCAL TYPES
 */

struct GenericDataMsg{
	uint8 msgCode;
};
	 
struct BindResponse {
	struct GenericDataMsg generticDataMsg;
	uint8 srcAddr[Z_EXTADDR_LEN]; // Src address
  	byte srcEP;                   // Endpoint/interface of source device
  	uint16 clusterID;             // Cluster ID
  	zAddrType_t dstAddr;          // Destination address
  	byte dstEP;                   // Endpoint/interface of dest device
};

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

struct ReadAttributeResponseMsg {
	struct GenericDataMsg generticDataMsg;
	uint8 type;
	uint16 clusterId;
	uint16 networkAddr;
	uint8 endpoint;
	uint16 panId;
	uint16 attrID;
	uint8 status;
	uint8 dataLen;
	uint8 data[MAX_DATA_SIZE];
};




/*********************************************************************
 * MACROS
 */
#define ANNUNCE_SEND_TIMEOUT   100     // Every 100 ms

/*********************************************************************
 * LOCAL TYPEDEFS
 */
typedef void (*UsbMessageHandler)(uint8 * data);

/*********************************************************************
 * GLOBAL VARIABLES
 */

/*********************************************************************
 * EXTERNAL VARIABLES
 */


/*********************************************************************
 * LOCAL VARIABLES
 */
#define MAX_DATE_SIZE_2	64
#define MAX_DATA_SIZE_3 128
static uint8 * tmpData;
struct AnnunceDataMsg annunceDataMsg;
static uint8 oldEndpoint;
static uint8 length;
static uint8 rxData[BULK_SIZE_OUT];
static uint16 * dest;
static uint16 * maxDest;
static uint16 * src;
static uint8 len;
static struct SimpleDescrMsg * simpleDescrMsg;
static struct ReadAttributeResponseMsg * readAttributeResponseMsg;
static int  currentDeviceElement=0;
static struct BindResponse bindResponse;
static AddrMgrEntry_t addrMgrEntry;
static struct UsbFifoData * usbFifoHead;

/*********************************************************************
 * LOCAL FUNCTIONS
 */
static void sendUsb(const uint8 * data, uint8 len);
static bool usbOutProcess(void);
static UsbMessageHandler parseDataOut(void);
static void sendFifo(void);

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
	
	if (usbOutProcess()){
		parseDataOut()(rxData);
	}
	
	sendFifo();
}

/***********************************************************************************
* @brief        send data from fifo to usb, if possibile
*
* @param        none
*
* @return       none
*/
static void sendFifo(void) {
	usbFifoHead = getUsbFifoHead();
	if (usbFifoHead != NULL && USBFW_IN_ENDPOINT_DISARMED()){
		oldEndpoint = USBFW_GET_SELECTED_ENDPOINT();
		USBFW_SELECT_ENDPOINT(3);

		tmpData =    usbFifoHead->data;
		len = usbFifoHead->dataLen;
		if (len > MAX_DATE_SIZE_3){
			len = MAX_DATE_SIZE_3;
		}
		do {
       		USBF3 = *tmpData;
			tmpData++;
			len--;
   		} while (len>0);
   		USBFW_ARM_IN_ENDPOINT();
		usbFifoPop();
 		USBFW_SELECT_ENDPOINT(oldEndpoint);	
	}
}

/**
* send a chunk of data to the host at endpoint 2
*/
void sendUsb(const uint8 * data, uint8 len) {
	oldEndpoint = USBFW_GET_SELECTED_ENDPOINT();
	USBFW_SELECT_ENDPOINT(3);

	// If the IN endpoint is ready to accept data.
	if (len > MAX_DATE_SIZE_3){
		len = MAX_DATE_SIZE_3;
	}
	if (USBFW_IN_ENDPOINT_DISARMED()) {
		do {
        	USBF3 = *data;
			data++;
			len--;
      	} while (len>0);
      	USBFW_ARM_IN_ENDPOINT();
	} else {
		usbFifoDataPush(data, len);
	}
 	USBFW_SELECT_ENDPOINT(oldEndpoint);
}

/***********************************************************************************
* @fn           usbOutProcess
*
* @brief        Read bulk data from endpoint 2.
*
* @param        none
*
* @return       true if there is some data
*/

static bool usbOutProcess(void){
	bool result = false;
    // If new packet is ready in USB FIFO
	HAL_DISABLE_INTERRUPTS();

    oldEndpoint = USBFW_GET_SELECTED_ENDPOINT();
    USBFW_SELECT_ENDPOINT(2);

    if (USBFW_OUT_ENDPOINT_DISARMED() ) {
        // Get length of USB packet, this operation must not be interrupted.
        length = USBFW_GET_OUT_ENDPOINT_COUNT_LOW();
        length+= USBFW_GET_OUT_ENDPOINT_COUNT_HIGH() >> 8;
        // Read from USB FIFO
		usbfwReadFifo(&USBF2, BULK_SIZE_OUT, rxData);
        USBFW_ARM_OUT_ENDPOINT();
		result = true;
    }
    USBFW_SELECT_ENDPOINT(oldEndpoint);
    HAL_ENABLE_INTERRUPTS();
	return result;
}

/***********************************************************************************
* @fn           parseDataOut
*
* @brief        Parse the usb out message and return the right pointer function
*
* @param        none
*
* @return       the pointer function
*/
UsbMessageHandler parseDataOut(void) {
	switch(rxData[0]){
	case REQ_SIMPLE_DESC:
		return usbReqSimpleDesc;
	case REQ_ACTIVE_EP:
		HAL_TOGGLE_LED1();
		return usbReqActiveEndpoint;
	case REQ_ATTRIBUTE_VALUES:
		return usbReqAttributeValue;
	case SEND_CMD:
		return usbSendCmdCluster;
	case WRITE_ATTRIBUTE_VALUE:
		return usbWriteAttributeValue;
	case REQ_ALL_NODES:
		currentDeviceElement=  0;
		return requestAllDevices;
	case REQ_BIND_TABLE:
		return usbReqBindTable;
	case REQ_ADD_BIND_TABLE_ENTRY:
		return usbAddBindTable;
	case REQ_REMOVE_BIND_TABLE_ENTRY:
		return usbRemoveBindTable;
	default:
		return usbNullMessage;
	}
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

void usbSendAttributeResponseMsg(zclReadRspStatus_t * readResponse, uint16 cluster, afAddrType_t * address ) {
	readAttributeResponseMsg = osal_mem_alloc(sizeof(struct  ReadAttributeResponseMsg));
	
	readAttributeResponseMsg->generticDataMsg.msgCode= ATTRIBUTE_VALUES;
	readAttributeResponseMsg->clusterId = cluster;
	readAttributeResponseMsg->networkAddr =address->addr.shortAddr;
	readAttributeResponseMsg->endpoint =address->endPoint;
	readAttributeResponseMsg->panId  = address->panId;
	readAttributeResponseMsg->type = readResponse->dataType;
	readAttributeResponseMsg->attrID = readResponse->attrID;
	readAttributeResponseMsg->status = readResponse->status;
	if (readAttributeResponseMsg->status == ZCL_STATUS_SUCCESS){
		readAttributeResponseMsg->dataLen = zclGetAttrDataLength(readResponse->dataType, readResponse->data);
		if (readAttributeResponseMsg->dataLen > MAX_DATA_SIZE){
			readAttributeResponseMsg->dataLen = MAX_DATA_SIZE;
		}
		osal_memcpy(readAttributeResponseMsg->data, readResponse->data, readAttributeResponseMsg->dataLen);
	} else {
		readAttributeResponseMsg->dataLen=0;
	}
	
	sendUsb((const uint8 *)readAttributeResponseMsg, sizeof(struct ReadAttributeResponseMsg));
	osal_mem_free(readAttributeResponseMsg);
}


void requestAllDevices(uint8 * notUsed){
	if (currentDeviceElement < NWK_MAX_ADDRESSES){
		while(1){
			addrMgrEntry.index=currentDeviceElement;
			addrMgrEntry.user=ADDRMGR_USER_DEFAULT;
			currentDeviceElement++;
			if (AddrMgrEntryGet(&addrMgrEntry)==TRUE){
				annunceDataMsg.genericDataMsg.msgCode = ANNUNCE;
				annunceDataMsg.nwkAddr = addrMgrEntry.nwkAddr;
				osal_memcpy(annunceDataMsg.extAddr, addrMgrEntry.extAddr, Z_EXTADDR_LEN);
				annunceDataMsg.capabilities = 0;
				sendUsb((uint8 *)&annunceDataMsg, sizeof(annunceDataMsg));
				osal_start_timerEx( zusbTaskId, USB_ANNUNCE_MSG, ANNUNCE_SEND_TIMEOUT );
			} else{
				if (currentDeviceElement >=NWK_MAX_ADDRESSES){
					currentDeviceElement=0;
					osal_stop_timerEx(zusbTaskId, USB_ANNUNCE_MSG);
					return;
				}
			}
		}
	} else {
		osal_stop_timerEx(zusbTaskId, USB_ANNUNCE_MSG);
		currentDeviceElement =0;
	}
}

void usbSendBindTables( apsBindingItem_t list[], uint8 bindingListCount){
	apsBindingItem_t * iter = list;
	apsBindingItem_t * end = list+bindingListCount;
	for (; iter < end; iter++){
		bindResponse.generticDataMsg.msgCode = BIND_TABLE;
		bindResponse.clusterID = iter->clusterID;
		bindResponse.dstAddr = iter->dstAddr;
		bindResponse.dstEP = iter->dstEP;
		osal_memcpy(&(bindResponse.srcAddr), &(iter->srcAddr), Z_EXTADDR_LEN);
		bindResponse.srcEP = iter->srcEP;
		sendUsb((const uint8 *)&bindResponse, sizeof(struct BindResponse));
	}
}