/**************************************************************************************************
  Filename:       ZigBeeUsbBrudge.c


  Copyright 2014 Paolo Achdjian

**************************************************************************************************/

/*********************************************************************
 * INCLUDES
 */
#include "OSAL.h"
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
#include "UsbMessageHandlers.h"
#include "AddrMgr.h"

#include "ZigBeeUsbBridge.h"
#include "ZdoMessageHandlers.h"

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
	 

/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * CONSTANTS
 */
#define DEVICE_VERSION     0
#define FLAGS              0

/*********************************************************************
 * LOCAL VARIABLES
 */
byte zusbTaskId;   // Task ID for internal task/event processing
                          // This variable will be received when
                          // zusbInit() is called.
static zdoIncomingMsg_t * zdoMsg;

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
  
 	ZigBeeInitUSB();
	endpointRequestTaskId(zusbTaskId);
	zclHA_Init( &simpleDesc );
  	// Register the Application to receive the unprocessed Foundation command/response messages
 	zcl_registerForMsg( zusbTaskId );
	
	ZDO_RegisterForZDOMsg( zusbTaskId, End_Device_Bind_rsp );
	ZDO_RegisterForZDOMsg( zusbTaskId, Match_Desc_rsp );
	ZDO_RegisterForZDOMsg( zusbTaskId, Device_annce);
	ZDO_RegisterForZDOMsg( zusbTaskId, Active_EP_rsp);
	ZDO_RegisterForZDOMsg( zusbTaskId, Simple_Desc_rsp);
	
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
UINT16 zusbProcessEvent( byte task_id, UINT16 events ){
	afIncomingMSGPacket_t *MSGpkt;
	afDataConfirm_t *afDataConfirm;

	// Data Confirmation message fields
	byte sentEP;
	ZStatus_t sentStatus;
	byte sentTransID;       // This should match the value sent
	(void)task_id;  // Intentionally unreferenced parameter

	if ( events & SYS_EVENT_MSG ) {
		MSGpkt = (afIncomingMSGPacket_t *)osal_msg_receive( zusbTaskId );
		while ( MSGpkt ){
			switch ( MSGpkt->hdr.event ){
				case ZCL_INCOMING_MSG:
					// Incoming ZCL Foundation command/response messages
					zclCoordinatort_ProcessZCLIncomingMsg( (zclIncomingMsg_t *)MSGpkt );
					break;
				case ZDO_CB_MSG:
					zdoMsg =  (zdoIncomingMsg_t *)MSGpkt;
					ZDOMessageHandlerFactory(zdoMsg->clusterID)(zdoMsg);
					break;
          		case AF_DATA_CONFIRM_CMD:
		          // This message is received as a confirmation of a data packet sent.
		          // The status is of ZStatus_t type [defined in ZComDef.h]
		          // The message fields are defined in AF.h
					afDataConfirm = (afDataConfirm_t *)MSGpkt;
					sentEP = afDataConfirm->endpoint;
					sentStatus = afDataConfirm->hdr.status;
					sentTransID = afDataConfirm->transID;
					(void)sentEP;
					(void)sentTransID;

					// Action taken when confirmation is received.
					if ( sentStatus != ZSuccess ) {
						// The data wasn't delivered -- Do something
					}
					break;
				case AF_INCOMING_MSG_CMD:
					zusbMessageMSGCB( MSGpkt );
					break;
		        default:
					break;
			}
		osal_msg_deallocate( (uint8 *)MSGpkt );
      	MSGpkt = (afIncomingMSGPacket_t *)osal_msg_receive( zusbTaskId );
		}
    return (events ^ SYS_EVENT_MSG);
	}

	if (events & USB_ANNUNCE_MSG){
		requestAllDevices(NULL);
		return (events ^ USB_ANNUNCE_MSG);
	}
	
	if (events & ENDPOINT_REQUEST_MSG){
		sendOneEndpointRequest();
		return (events ^ ENDPOINT_REQUEST_MSG);
	}
	return 0;
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
