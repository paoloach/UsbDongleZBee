/**************************************************************************************************
  Filename:       ZigBeeUsbBrudge.c


  Copyright 2014 Paolo Achdjian

**************************************************************************************************/

/*********************************************************************
 * INCLUDES
 */
#include "OSAL.h"
#include "OSAL_Memory.h"
#include "AF.h"
#include "ZDApp.h"
#include "ZDObject.h"
#include "ZDProfile.h"
#include "UsbCodeHandler.h"
#include "ZCL_HA.h"
#include "zcl.h"
#include "ZigBeeUsbBridge_data.h"
#include "zcl_functions.h"
#include "TimerEvents.h"
#include "UsbFunctions.h"
#include "AddrMgr.h"

#include "ZigBeeUsbBridge.h"
#include "ZdoMessageHandlers.h"
#include "UsbIrqHookProcessEvents.h"

#if !defined( WIN32 )
  #include "OnBoard.h"
#endif

/* HAL */
#include "hal_lcd.h"
#include "hal_led.h"
#include "hal_key.h"
#include "hal_uart.h"
#include "stdio.h"
#include "endpointRequestList.h"
#include "DeviceManager.h"


#define DEVICE_VERSION     0
#define FLAGS              0


/*********************************************************************
 * LOCAL VARIABLES
 */
byte zusbTaskId;   // Task ID for internal task/event processing
                          // This variable will be received when
                          // zusbInit() is called.

/*********************************************************************
 * LOCAL FUNCTIONS
 */
void zusbMessageMSGCB( afIncomingMSGPacket_t *pckt );
void zusbSendTheMessage( void );

 /*********************************************************************
 * NETWORK LAYER CALLBACKS
 */

/*********************************************************************
 * PUBLIC FUNCTIONS
 */

#define MAX_INCLUSTERS       4
const cId_t InClusterList[MAX_INCLUSTERS] ={
  ZCL_CLUSTER_ID_GEN_BASIC,
  ZCL_CLUSTER_ID_GEN_IDENTIFY,
};

#define OUTCLUSTERS       2
const cId_t OutClusterList[OUTCLUSTERS] ={
  ZCL_CLUSTER_ID_GEN_BASIC,
  ZCL_CLUSTER_ID_GEN_IDENTIFY
};

SimpleDescriptionFormat_t simpleDesc = {
  ENDPOINT,                  			//  int Endpoint;
  ZCL_HA_PROFILE_ID,                	//  uint16 AppProfId[2];
  ZCL_HA_DEVICEID_ON_OFF_LIGHT,     	//  uint16 AppDeviceId[2];
  DEVICE_VERSION,           			//  int   AppDevVer:4;
  FLAGS,                    			//  int   AppFlags:4;
  MAX_INCLUSTERS,              //  byte  AppNumInClusters;
  (cId_t *)InClusterList, 	//  byte *pAppInClusterList;
  OUTCLUSTERS	,           //  byte  AppNumInClusters;
  (cId_t *)OutClusterList 	//  byte *pAppInClusterList;
};

/*********************************************************************
 * @fn      zusbInit
 *
 * @brief   Initialization function for the Generic App Task.
 *          This is called during initialization and should contain
 *          any application specific initialization (ie. hardware
 *          initialization/setup, table initialization, power up
 *          notificaiton ... ).
 *
 * @param   task_id - the ID assigned by OSAL.  This ID should be
 *                    used to send messages and set timers.
 *
 * @return  none
 */
void zusbAppInit( byte task_id ){
	zusbTaskId = task_id;
  
	deviceManagerInit();
 
	endpointRequestTaskId(zusbTaskId);
	zclHA_Init( &simpleDesc );
  	// Register the Application to receive the unprocessed Foundation command/response messages
 	zcl_registerForMsg( zusbTaskId );
	
	ZDO_RegisterForZDOMsg( zusbTaskId, End_Device_Bind_rsp );
	ZDO_RegisterForZDOMsg( zusbTaskId, Mgmt_Bind_rsp );
	ZDO_RegisterForZDOMsg( zusbTaskId, Match_Desc_rsp );
	ZDO_RegisterForZDOMsg( zusbTaskId, Device_annce);
	ZDO_RegisterForZDOMsg( zusbTaskId, Active_EP_rsp);
	ZDO_RegisterForZDOMsg( zusbTaskId, Simple_Desc_rsp);
	
	T1CTL=1;
}

/*********************************************************************
 * @fn      GenericApp_ProcessEvent
 *
 * @brief   Generic Application Task event processor.  This function
 *          is called to process all events for the task.  Events
 *          include timers, messages and any other user defined events.
 *
 * @param   task_id  - The OSAL assigned task ID.
 * @param   events - events to process.  This is a bit map and can
 *                   contain more than one event.
 *
 * @return  none
 */
  uint16 osal_heap_block_cnt( void );
UINT16 zusbProcessEvent( byte task_id, UINT16 events ){
	(void)task_id;  // Intentionally unreferenced parameter
	UINT16 result=0;
	 
	int16 blocks =  osal_heap_block_cnt();
		
	if ( events & SYS_EVENT_MSG ) {
		osal_event_hdr_t * hdrEvent = (osal_event_hdr_t *) osal_msg_receive( zusbTaskId );
		switch(hdrEvent->event){
		case ZCL_INCOMING_MSG:
			usbLog(0,"Incoming ZCL Foundation command/response messages");
			zclCoordinatort_ProcessZCLIncomingMsg( (zclIncomingMsg_t *)hdrEvent );
			break;
		case ZDO_CB_MSG:{
			zdoIncomingMsg_t * zdoMsg =  (zdoIncomingMsg_t *)hdrEvent;
			usbLog(0,"Incoming ZDO message from %X for cluster %s ",zdoMsg->srcAddr.addr.shortAddr,clusterRequestToString(zdoMsg->clusterID));
			ZDOMessageHandlerFactory(zdoMsg->clusterID)(zdoMsg);
			}
			break;
		case AF_INCOMING_MSG_CMD:
			zusbMessageMSGCB((afIncomingMSGPacket_t *) hdrEvent );
			break;
		case AF_DATA_CONFIRM_CMD:
			break;
		case EVENT_USB_ISR:
			{
			struct UsbISR * isr = (struct UsbISR*)hdrEvent;
			isr->isr(hdrEvent);
			}
			break;
		}
		 osal_msg_deallocate( (uint8 *)hdrEvent );
	    result = (events ^ SYS_EVENT_MSG);
		goto end;
	}

	if (events & USB_ANNUNCE_MSG){
		requestAllDevices();
		result =  (events ^ USB_ANNUNCE_MSG);
		goto end;
	}
	
	if (events & USB_ANNUNCE2_MSG){
		requestAllDevices2(NULL);
		result =  (events ^ USB_ANNUNCE2_MSG);
		goto end;
	}
	
	if (events & ENDPOINT_REQUEST_MSG){
		sendOneEndpointRequest();
		
		result = (events ^ ENDPOINT_REQUEST_MSG);
		goto end;
	}
	
end:
	blocks = osal_heap_block_cnt()-blocks;
	if (blocks != 0)
		usbLog(0, "zusbProcessEvent process delta block: %d", blocks);
	return result;
}



/*********************************************************************
 * LOCAL FUNCTIONS
 */

/*********************************************************************
 * @fn      zusbMessageMSGCB
 *
 * @brief   Data message processor callback.  This function processes
 *          any incoming data - probably from other devices.  So, based
 *          on cluster ID, perform the intended action.
 *
 * @param   none
 *
 * @return  none
 */
void zusbMessageMSGCB( afIncomingMSGPacket_t *pkt )
{
  switch ( pkt->clusterId )
  {
    case GENERICAPP_CLUSTERID:
      // "the" message
      break;
  }
}


/*********************************************************************
*********************************************************************/
