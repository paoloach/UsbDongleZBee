/**************************************************************************************************
  Filename:       ZdoMessageHandlers.h


  Copyright 2014 Paolo Achdjian

**************************************************************************************************/

#include "ZDObject.h"
#include "ZdoMessageHandlers.h"
#include "UsbFunctions.h"
#include "hal_board.h"
#include "hal_types.h"
#include "endpointRequestList.h"
#include "DeviceManager.h"

/*********************************************************************
 * LOCAL VARIABLES
 */
static ZDO_DeviceAnnce_t device;
static ZDO_SimpleDescRsp_t simpleDesc;
static uint8 *msg;
static struct BindTableResponse * bindTableResponse;
static uint8 len;
static uint16 dataSize;
static uint8 i;

/*********************************************************************
 * LOCAL FUNCTIONS
 */

static void notHandledMessage(zdoIncomingMsg_t * msg);
static void annunceMessage(zdoIncomingMsg_t * msg);
static void activeEndpointResponseMessage(zdoIncomingMsg_t * msg);
static void simpleDecriptorMessage(zdoIncomingMsg_t * msg);
static void mgmtBindResponseMessage(zdoIncomingMsg_t * msg);


ZDOMessageHandler ZDOMessageHandlerFactory(cId_t clusterId) {
	ZDOMessageHandler zdoMessageHandler;
	
	switch(clusterId){
		case Device_annce:
			zdoMessageHandler = annunceMessage;
			break;
		case Active_EP_rsp:
			zdoMessageHandler = activeEndpointResponseMessage;
			break;
		case Simple_Desc_rsp:
			zdoMessageHandler = simpleDecriptorMessage;
			break;
		case Mgmt_Bind_rsp:
			zdoMessageHandler = mgmtBindResponseMessage;
			break;
		default:
			zdoMessageHandler = notHandledMessage;
	}
	return zdoMessageHandler;
}


static void notHandledMessage(zdoIncomingMsg_t * msg) {
	usbLog(0,"handling unknown cluster");
}


static void annunceMessage(zdoIncomingMsg_t * msg) {
	usbLog(0,"handling annunce message");
	
	ZDO_ParseDeviceAnnce( msg, &device );
	addDevice(&device);
	usbSendAnnunce(&device);

}


static void activeEndpointResponseMessage(zdoIncomingMsg_t * msg) {
	usbLog(0,"handling active EP rsp");
	ZDO_ActiveEndpointRsp_t * EPList = ZDO_ParseEPListRsp(msg );
	for (int i=0; i < EPList->cnt; i++){
		addEndpointRequest( EPList->nwkAddr, EPList->epList[i], 0);
	}
	sendOneEndpointRequest();
	osal_mem_free(EPList);
}

static void simpleDecriptorMessage(zdoIncomingMsg_t * msg) {
	ZDO_ParseSimpleDescRsp( msg, &simpleDesc );
	usbLog(0,"handling simple desc rsp");
	if (simpleDesc.status == ZDP_SUCCESS){
		usbSendSimpleDescriptor(&simpleDesc);
	}
}

static void mgmtBindResponseMessage(zdoIncomingMsg_t * inMsg) {
	uint8 valid;
	struct BindTableResponseEntry *list;
	
	usbLog(0,"handling Mgmt bind rsp");
	
	msg =  inMsg->asdu;
	if (*msg++ == ZSuccess ){
		msg++;
		msg++;
		len = *msg++;
	} else {
		return;
	}

	dataSize = len * sizeof( struct BindTableResponseEntry );
	if (dataSize > (MAX_DATE_SIZE_5 -2)){
		dataSize = (MAX_DATE_SIZE_5-2) -(MAX_DATE_SIZE_5-2)%sizeof( struct BindTableResponseEntry );
	}
	bindTableResponse = (struct BindTableResponse *)osal_mem_alloc((sizeof ( struct BindTableResponse ) + dataSize ));
	if (bindTableResponse==NULL){
		return;
	}
	bindTableResponse->generticDataMsg.msgCode = BIND_TABLE;
	bindTableResponse->elementSize=0;
	list = bindTableResponse->list;
	dataSize=sizeof(struct BindTableResponse );
	for (i=0; i< len; i++){
		valid=1;
		if (deviceNwkAddrLookup(msg, &list->srcAddr)==FALSE){
			valid=0;
		}	
		msg += Z_EXTADDR_LEN;
		list->srcEP = *msg++;
		list->clusterID = BUILD_UINT16( msg[0], msg[1] );
		msg += 2;
		if (*msg++ == Addr64Bit){
			if (deviceNwkAddrLookup(msg, &list->dstAddr)==FALSE){
				valid=0;
			}
			msg += Z_EXTADDR_LEN;
		} else {
			list->dstAddr = BUILD_UINT16( msg[0], msg[1] );
			msg += 2;
		}
		list->dstEP = *msg++;
		if (valid){
			bindTableResponse->elementSize++;
			list++;
			dataSize += sizeof(struct BindTableResponseEntry );
			if (dataSize + sizeof(struct BindTableResponseEntry) > MAX_DATE_SIZE_5){
				sendUsb((uint8 *)bindTableResponse, dataSize);
				dataSize =sizeof(struct BindTableResponse );
				list = bindTableResponse->list;
			}
		}
	}
	sendUsb((uint8 *)bindTableResponse, dataSize);
	
	osal_mem_free(bindTableResponse);
	
}