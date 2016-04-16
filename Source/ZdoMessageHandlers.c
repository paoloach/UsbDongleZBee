/**************************************************************************************************
  Filename:       ZdoMessageHandlers.h


  Copyright 2014 Paolo Achdjian

**************************************************************************************************/

#include "ZDObject.h"
#include "ZdoMessageHandlers.h"
#include "UsbFunctions.h"
#include "hal_board.h"
#include "endpointRequestList.h"

/*********************************************************************
 * LOCAL VARIABLES
 */
static ZDO_DeviceAnnce_t device;
static ZDO_SimpleDescRsp_t simpleDesc;
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
}


static void annunceMessage(zdoIncomingMsg_t * msg) {
	ZDO_ParseDeviceAnnce( msg, &device );
	usbSendAnnunce(&device);
}


static void activeEndpointResponseMessage(zdoIncomingMsg_t * msg) {
	ZDO_ActiveEndpointRsp_t * EPList = ZDO_ParseEPListRsp(msg );
	for (int i=0; i < EPList->cnt; i++){
		addEndpointRequest( EPList->nwkAddr, EPList->epList[i], 0);
	}
	sendOneEndpointRequest();
	osal_mem_free(EPList);
}

static void simpleDecriptorMessage(zdoIncomingMsg_t * msg) {
	ZDO_ParseSimpleDescRsp( msg, &simpleDesc );
	if (simpleDesc.status == ZDP_SUCCESS){
		usbSendSimpleDescriptor(&simpleDesc);
	}
}

static void mgmtBindResponseMessage(zdoIncomingMsg_t * msg) {
	ZDO_MgmtBindRsp_t * response = ZDO_ParseMgmtBindRsp(msg);
	if (response != NULL){
		if (response->status == ZSuccess){
			usbSendBindTables(response->list, response->bindingListCount);
		}
		if (response->startIndex+response->bindingListCount < response->bindingCount){
			uint8 startIndex=response->startIndex+response->bindingListCount;
			ZDP_MgmtBindReq( &(msg->srcAddr), startIndex, 0 );
		}
		osal_mem_free(response);
	}
}