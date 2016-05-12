/**************************************************************************************************
  Filename:       zcl_functions.c


  Copyright 2014 Paolo Achdjian

**************************************************************************************************/

#include "zcl_functions.h"
#include "UsbFunctions.h"


/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * CONSTANTS
 */
/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * GLOBAL VARIABLES
 */

/*********************************************************************
 * GLOBAL FUNCTIONS
 */

/*********************************************************************
 * LOCAL VARIABLES
 */

/*********************************************************************
 * LOCAL FUNCTIONS
 */
static uint8 ZCLProcessInReadRspCmd( zclIncomingMsg_t *pInMsg );
static uint8 ZCLProcessInWriteRspCmd( zclIncomingMsg_t *pInMsg );
static uint8 ZCLProcessInDefaultRspCmd( zclIncomingMsg_t *pInMsg );

/*********************************************************************
 * @fn      zclCoordinatort_ProcessZCLIncomingMsg
 *
 * @brief   Process ZCL Foundation incoming message
 *
 * @param   pInMsg - pointer to the received message
 *
 * @return  none
 */
void zclCoordinatort_ProcessZCLIncomingMsg( zclIncomingMsg_t *pInMsg){
	switch ( pInMsg->zclHdr.commandID ) {
    case ZCL_CMD_READ_RSP:
		ZCLProcessInReadRspCmd( pInMsg );
		break;
    case ZCL_CMD_WRITE_RSP:
		ZCLProcessInWriteRspCmd( pInMsg );
		break;
    // See ZCL Test Applicaiton (zcl_testapp.c) for sample code on Attribute Reporting
    case ZCL_CMD_CONFIG_REPORT:
		//zclSampleLight_ProcessInConfigReportCmd( pInMsg );
		break;
   
    case ZCL_CMD_CONFIG_REPORT_RSP:
		//zclSampleLight_ProcessInConfigReportRspCmd( pInMsg );
		break;
    
    case ZCL_CMD_READ_REPORT_CFG:
		//zclSampleLight_ProcessInReadReportCfgCmd( pInMsg );
		break;
    
    case ZCL_CMD_READ_REPORT_CFG_RSP:
		//zclSampleLight_ProcessInReadReportCfgRspCmd( pInMsg );
		break;
    
    case ZCL_CMD_REPORT:
		//zclSampleLight_ProcessInReportCmd( pInMsg );
		break;
    case ZCL_CMD_DEFAULT_RSP:
		ZCLProcessInDefaultRspCmd( pInMsg );
		break;
    default:
		break;
	}
  
	if ( pInMsg->attrCmd )
		osal_mem_free( pInMsg->attrCmd );
}

/*********************************************************************
 * @fn      ZCLProcessInReadRspCmd
 *
 * @brief   Process the "Profile" Read Response Command
 *
 * @param   pInMsg - incoming message to process
 *
 * @return  none
 */
static uint8 ZCLProcessInReadRspCmd( zclIncomingMsg_t *pInMsg ){
	usbSendAttributeResponseMsg((zclReadRspCmd_t *)pInMsg->attrCmd, pInMsg->clusterId, &(pInMsg->srcAddr));
	return TRUE; 
}

/*********************************************************************
 * @fn      coordinator_ZCLProcessInWriteRspCmd
 *
 * @brief   Process the "Profile" Write Response Command
 *
 * @param   pInMsg - incoming message to process
 *
 * @return  none
 */
static uint8 ZCLProcessInWriteRspCmd( zclIncomingMsg_t *pInMsg ){
  zclWriteRspCmd_t *writeRspCmd;
  static uint8 index;

  writeRspCmd = (zclWriteRspCmd_t *)pInMsg->attrCmd;
  for (index = 0; index < writeRspCmd->numAttr; index++)
  {
    // Notify the device of the results of the its original write attributes
    // command.
  }

  return TRUE; 
}

/*********************************************************************
 * @fn      ZCLProcessInDefaultRspCmd
 *
 * @brief   Process the "Profile" Default Response Command
 *
 * @param   pInMsg - incoming message to process
 *
 * @return  none
 */
static uint8 ZCLProcessInDefaultRspCmd( zclIncomingMsg_t *pInMsg )
{
  // zclDefaultRspCmd_t *defaultRspCmd = (zclDefaultRspCmd_t *)pInMsg->attrCmd;
   
  // Device is notified of the Default Response command.
  (void)pInMsg;
  
  return TRUE; 
}

