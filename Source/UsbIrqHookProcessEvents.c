/**************************************************************************************************

 DESCRIPTION:
  --

 CREATED: 17/08/2016, by Paolo Achdjian

 FILE: UsbIrqHookProcessEvents.c

***************************************************************************************************/

#include "usb_framework.h"
#include "usb_interrupt.h"
#include "mac_mcu.h"
#include "UsbFifoData.h"
#include "UsbFunctions.h"
#include "TimerEvents.h"
#include "ZComDef.h"
#include "OSAL.h"
#include "OnBoard.h"
#include "UsbIrqHookProcessEvents.h"
#include "ZigBeeUsbBridge_data.h"


extern byte zusbTaskId;
extern uint16  currentDeviceElement;
extern uint8  usbOn;

struct ReqAttributeValueMsg {
	uint16      nwkAddr;
	uint8       endpoint;
	uint16		cluster;
	uint8		numAttributes;
};

struct SendCmdUsbMsg {
	uint16      nwkAddr;
	uint8       endpoint;
	uint16		cluster;
	uint16      cmdClusterId;
	uint8		dataLen;
};

struct WriteAttributeValueUsbMsg {
	uint16      nwkAddr;
	uint8       endpoint;
	uint16		cluster;
	uint16      attributeId;
    uint8       dataType;
	uint8       dataValueLen;
	uint8       dataValue[];
};

struct ReqIeeeAddrMsg {
	struct UsbISR	isr;
	union  nwkAddr {
		uint16      nwkAddr;
		uint8		data[2];
	};
	uint8		requestType;
	uint8 		startIndex;
};

static void attributeValue(osal_event_hdr_t *);
static void eventSendFifo(osal_event_hdr_t *);
static void eventReset(osal_event_hdr_t *);
static void eventActiveEP(osal_event_hdr_t *);
static void eventReqIeeeAddr(osal_event_hdr_t *);
static void eventDeviceInfo(osal_event_hdr_t *);
static void eventSendCmd(osal_event_hdr_t *);
static void eventReqAllNodes(osal_event_hdr_t *);
static void eventBindReq(osal_event_hdr_t *);
static void eventWriteValue(osal_event_hdr_t *);
static void eventBindRequest(osal_event_hdr_t *);
static void eventUnbindRequest(osal_event_hdr_t *);
static struct UsbISR * createMsgForBind(void);

// ************************ USB interrupt event processing *************************
void usbirqHookProcessEvents(void)
{
	T1CNTL=0;
	if (usbirqData.eventMask & USBIRQ_EVENT_EP5IN){
		if (isFifoEmpty()==0){
			struct UsbISR * msg = (struct UsbISR *)osal_msg_allocate(sizeof(struct UsbISR) );
			msg->msg.event = EVENT_USB_ISR;
			msg->isr = eventSendFifo;
			osal_msg_send(zusbTaskId, (uint8 *)msg);
		}
	}
	if (usbirqData.eventMask & USBIRQ_EVENT_EP2OUT){
		 uint8 oldEndpoint = USBFW_GET_SELECTED_ENDPOINT();
	     USBFW_SELECT_ENDPOINT(2);

		uint8 length = USBFW_GET_OUT_ENDPOINT_COUNT_LOW();
		if (length > MAX_DATE_SIZE_2){
			length = MAX_DATE_SIZE_2;
		}
  	    

   		if (length) {
			struct UsbISR * msg =NULL;
			uint8 code = USBF2;
			switch( code){
			case ENABLE_INFO_MESSAGE:
				usbOn=1;
				break;
			case REQ_RESET:
					msg = (struct UsbISR *)osal_msg_allocate(sizeof(struct UsbISR) );
					msg->msg.event = EVENT_USB_ISR;
					msg->isr = eventReset;
					break;
			case REQ_ALL_NODES:
					msg = (struct UsbISR *)osal_msg_allocate(sizeof(struct UsbISR) );
					msg->msg.event = EVENT_USB_ISR;
					msg->isr = eventReqAllNodes;
					break;

			case REQ_BIND_TABLE: {
					uint8 addr[2];
					addr[0] = USBF2;
					addr[1] = USBF2;
					struct BindTableRequestMsg * msgReq = (struct BindTableRequestMsg *)osal_msg_allocate(sizeof(struct BindTableRequestMsg) );
					msg = &(msgReq->isr);
					msg->isr = eventBindReq;
					msg->msg.event = EVENT_USB_ISR;
					msgReq->afAddrType.addrMode = Addr16Bit;
					msgReq->afAddrType.addr.shortAddr = *(uint16 *)(addr);
					}
					break;
			case REQ_ACTIVE_EP:{
					struct ReqActiveEndpointsEvent * msgEP = (struct ReqActiveEndpointsEvent *)osal_msg_allocate(sizeof(struct ReqActiveEndpointsEvent) );
					msg = &(msgEP->isr);
					msg->isr = eventActiveEP;
					msg->msg.event = EVENT_USB_ISR;
					msgEP->data[0] = USBF2;
					msgEP->data[1] = USBF2;
					}
					break;
			case REQ_ADD_BIND_TABLE_ENTRY:
					msg = createMsgForBind();
					msg->isr = eventBindRequest;
					break;
			case REQ_REMOVE_BIND_TABLE_ENTRY:
					msg = createMsgForBind();
					msg->isr = eventUnbindRequest;
					break;
			case REQ_IEEE_ADDRESS:{
					struct ReqIeeeAddrMsg * msgReq = (struct ReqIeeeAddrMsg *)osal_msg_allocate(sizeof(struct ReqIeeeAddrMsg) );
					msg = &(msgReq->isr);
					msg->isr = eventReqIeeeAddr;
					msg->msg.event = EVENT_USB_ISR;
					msgReq->data[0] = USBF2;			
					msgReq->data[1] = USBF2;	
					msgReq->requestType = USBF2;
					msgReq->startIndex = USBF2;
					break;
					}
			case WRITE_ATTRIBUTE_VALUE:{
					struct WriteAttributeValueUsbMsg usbMsg;
					uint8  * data = (uint8 *)(&usbMsg);
					uint8  i;
					for(i=0; i < sizeof(struct WriteAttributeValueUsbMsg); i++){
						*data = USBF2;
						data++;
					}
					struct WriteAttributeValueMsg * msgCmd = (struct WriteAttributeValueMsg *)osal_msg_allocate(sizeof(struct WriteAttributeValueMsg) +sizeof(zclWriteRec_t) + usbMsg.dataValueLen  );
					msg = &(msgCmd->isr);
					msg->isr = eventWriteValue;
					msg->msg.event = EVENT_USB_ISR;
					
					msgCmd->afAddrType.addrMode=afAddr16Bit;
					msgCmd->afAddrType.addr.shortAddr=usbMsg.nwkAddr;
					msgCmd->afAddrType.endPoint=usbMsg.endpoint;
					msgCmd->cluster = usbMsg.cluster;
					msgCmd->writeCmd.numAttr=1;
					msgCmd->writeCmd.attrList->attrID = usbMsg.attributeId;
					msgCmd->writeCmd.attrList->dataType=usbMsg.dataType;
					data = ((uint8 *)msgCmd) + sizeof(struct WriteAttributeValueMsg) +sizeof(zclWriteRec_t);
					msgCmd->writeCmd.attrList->attrData = data;
					for(i=0; i < usbMsg.dataValueLen; i++){
						*data = USBF2;
						data++;
					}
					}
					break;
			case SEND_CMD:{
					struct SendCmdUsbMsg usbMsg;
					uint8  * data = (uint8 *)(&usbMsg);
					uint8  i;
					for(i=0; i < sizeof(struct SendCmdUsbMsg); i++){
						*data = USBF2;
						data++;
					}
					struct SendCmdMsg * msgCmd = (struct SendCmdMsg *)osal_msg_allocate(sizeof(struct SendCmdMsg) +usbMsg.dataLen  );
					msg = &(msgCmd->isr);
					msg->isr = eventSendCmd;
					msg->msg.event = EVENT_USB_ISR;
					msgCmd->cluster =usbMsg.cluster;
					msgCmd->cmdClusterId = usbMsg.cmdClusterId;
					msgCmd->afAddrType.addr.shortAddr= usbMsg.nwkAddr;
					msgCmd->afAddrType.addrMode = afAddr16Bit;
					msgCmd->afAddrType.endPoint = usbMsg.endpoint;
					msgCmd->dataLen = usbMsg.dataLen;
					
					data = (uint8 *)(msgCmd->data);
					for(i=0; i < usbMsg.dataLen; i++){
						*data = USBF2;
						data++;
					}	
					
					}
					break;
			case REQ_ATTRIBUTE_VALUES: {
				    struct ReqAttributeValueMsg attr;
					uint8  * data = (uint8 *)(&attr);
					uint8  i;
					for(i=0; i < sizeof(struct ReqAttributeValueMsg); i++){
						*data = USBF2;
						data++;
					}
					struct ReqAttributeMsg * msgAttr = (struct ReqAttributeMsg *)osal_msg_allocate(sizeof(struct ReqAttributeMsg) +attr.numAttributes* sizeof(uint16)  );
					msg = &(msgAttr->isr);
					msg->isr = attributeValue;
					msg->msg.event = EVENT_USB_ISR;
					
					msgAttr->afAddrType.addr.shortAddr = attr.nwkAddr;
					msgAttr->afAddrType.addrMode = afAddr16Bit;
					msgAttr->afAddrType.endPoint = attr.endpoint;
					
					msgAttr->numAttr = attr.numAttributes;
					data = (uint8 *)&msgAttr->attrID;
					for (uint8 i=0; i < attr.numAttributes; i++){
						*data = USBF2;
						data++;
						*data = USBF2;
						data++;
					}
					msgAttr->cluster = attr.cluster;
					osal_msg_send(zusbTaskId, (uint8 *)msg);
					break;
				}
			case REQ_DEVICE_INFO:{
				struct ReqDeviceInformationEvent * msgEP = (struct ReqDeviceInformationEvent *)osal_msg_allocate(sizeof(struct ReqDeviceInformationEvent) );
				msg = &(msgEP->isr);
				msg->isr = eventDeviceInfo;
				msg->msg.event = EVENT_USB_ISR;
				msgEP->data[0] = USBF2;
				msgEP->data[1] = USBF2;
				}
				break;
					
			}
			if (msg != NULL) {
				uint8 low = T1CNTL;
				uint8 hi = T1CNTH;
				msg->time=BUILD_UINT16(low,hi);
				osal_msg_send(zusbTaskId, (uint8 *)msg);
			}
			/*uint8 __generic *pTemp = rxData;
      		do {
         		*(pTemp++) = USBF2;
      		} while (--length);*/
   		}
      
		USBFW_ARM_OUT_ENDPOINT();
		USBFW_SELECT_ENDPOINT(oldEndpoint);
	}
}

static struct UsbISR * createMsgForBind(void) {
	uint8 * data;
	uint8  i;
	struct BindRequestMsg * msgBind = (struct BindRequestMsg *)osal_msg_allocate(sizeof(struct BindRequestMsg) );
	msgBind->isr.msg.event = EVENT_USB_ISR;
	msgBind->destAddr.addrMode = Addr16Bit;
	data = (uint8 *)msgBind->destAddr.addr.shortAddr;
	*data = USBF2;
	data++;
	*data = USBF2;
	data = (uint8 *)msgBind->outClusterAddr;
	for (i=0; i < Z_EXTADDR_LEN+3; i++){
		*data = USBF2;
		data++;
	}

	msgBind->inCluster.addrMode = Addr64Bit;
	data = (uint8 *)msgBind->inCluster.addr.extAddr;
	for(i=0; i< Z_EXTADDR_LEN; i++){
		*data = USBF2;
		data++;
	}
	msgBind->inClusterEP = USBF2;
	return 	&(msgBind->isr);			
}

void eventDeviceInfo(osal_event_hdr_t * hdrEvent){
	struct ReqDeviceInformationEvent * msgEP = (struct ReqDeviceInformationEvent *)hdrEvent;
	usbLog(0, "Request device info for %X", msgEP->nwkAddr);
	associated_devices_t * device= AssocGetWithShort( msgEP->nwkAddr);
	if (device == NULL){
		usbLog(0, "device %X not found", msgEP->nwkAddr );
	} else {
		usbSendDeviceInfo(device);
	}	
}


void eventReqIeeeAddr(osal_event_hdr_t * hdrEvent) {
	struct ReqIeeeAddrMsg * msg = (struct ReqIeeeAddrMsg *)hdrEvent;
	usbLog(0, "Request IEEE address for %X",msg->nwkAddr);
	ZDP_IEEEAddrReq( msg->nwkAddr, msg->requestType, msg->startIndex, 0); 
}

void eventActiveEP(osal_event_hdr_t * hdrEvent) {
	zAddrType_t destAddr;
	struct ReqActiveEndpointsEvent * msgEP = (struct ReqActiveEndpointsEvent *)hdrEvent;
	usbLog(0, "%u, Request active endpoint for %X",msgEP->isr.time, msgEP->nwkAddr);

	destAddr.addrMode = Addr16Bit;
	destAddr.addr.shortAddr = msgEP->nwkAddr;
	afStatus_t ret = ZDP_NWKAddrOfInterestReq(&destAddr,msgEP->nwkAddr, Active_EP_req, 0);
	if (ret != ZSuccess){
		usbSendActiveEPError(msgEP->nwkAddr, ret);
	}
}

void attributeValue(osal_event_hdr_t * hdrEvent){
	ZStatus_t status;
	struct ReqAttributeMsg * reqAttributeMsg  = (struct ReqAttributeMsg *)hdrEvent;

	usbLog(0, "Send cmd ZCL_CMD_READ to %X:%x  cluster: %X numAttr: %d", reqAttributeMsg->afAddrType.addr.shortAddr, reqAttributeMsg->afAddrType.endPoint, reqAttributeMsg->cluster,reqAttributeMsg->numAttr); 

	status = zcl_SendCommand( ENDPOINT, &reqAttributeMsg->afAddrType, reqAttributeMsg->cluster, ZCL_CMD_READ, FALSE, ZCL_FRAME_CLIENT_SERVER_DIR, FALSE, 0, 0,  reqAttributeMsg->numAttr * 2, (uint8 *)reqAttributeMsg->attrID );
	if (status != ZSuccess){
		usbSendAttributeResponseMsgError(reqAttributeMsg, status);
	}

}

void eventSendFifo(osal_event_hdr_t * noUsed){
	sendFifo();
}

void eventReset(osal_event_hdr_t * hdrEvent) {
	SystemReset();
}

void eventReqAllNodes(osal_event_hdr_t * msg) {
	usbLog(0, "%u: Request all nodes",((struct UsbISR *)msg)->time);
	currentDeviceElement=0;
	requestAllDevices();
}

void eventBindReq(osal_event_hdr_t * hdrEvent) {
	uint8 first=0;
	struct BindTableRequestMsg * msg = (struct BindTableRequestMsg *)hdrEvent;
	
	usbLog(0,"%u, Request bind table for %X",msg->isr.time, msg->afAddrType.addr.shortAddr);

	ZDP_MgmtBindReq( &(msg->afAddrType),first, 0 );
}

void eventBindRequest(osal_event_hdr_t * hdrEvent) {
	struct BindRequestMsg * msg = (struct BindRequestMsg *)hdrEvent;
	
	ZDP_BindReq(&(msg->destAddr),  msg->outClusterAddr, msg->outClusterEP, msg->clusterID, &(msg->inCluster),  msg->inClusterEP, 0);
}

void eventUnbindRequest(osal_event_hdr_t * hdrEvent) {
	struct BindRequestMsg * msg = (struct BindRequestMsg *)hdrEvent;
	
	ZDP_UnbindReq(&(msg->destAddr),  msg->outClusterAddr, msg->outClusterEP, msg->clusterID, &(msg->inCluster),  msg->inClusterEP, 0);
}

void eventSendCmd(osal_event_hdr_t * hdrEvent) {
	struct SendCmdMsg * msgCmd = (struct SendCmdMsg * )hdrEvent;
	
	zcl_SendCommand(ENDPOINT, &msgCmd->afAddrType, msgCmd->cluster, msgCmd->cmdClusterId, TRUE, ZCL_FRAME_CLIENT_SERVER_DIR, 0, 0, 0, msgCmd->dataLen, msgCmd->data );
}

void eventWriteValue(osal_event_hdr_t * hdrEvent) {
	struct WriteAttributeValueMsg * msgCmd = (struct WriteAttributeValueMsg * )hdrEvent;
	
	zcl_SendWriteRequest(ENDPOINT, &(msgCmd->afAddrType), msgCmd->cluster,&(msgCmd->writeCmd),  ZCL_CMD_WRITE_NO_RSP, ZCL_FRAME_CLIENT_SERVER_DIR, 0, 0);
}