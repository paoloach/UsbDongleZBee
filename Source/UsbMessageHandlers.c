/**************************************************************************************************
  Filename:       UsbMessageHandlers.c


  Copyright 2014 Paolo Achdjian

**************************************************************************************************/

/*********************************************************************
 * INCLUDES
 */
#include "UsbMessageHandlers.h"
#include "ZComDef.h"
#include "ZDProfile.h"
#include "zcl.h"
#include "ZigBeeUsbBridge_data.h"
#include "hal_board_cfg.h"
#include "BindingTable.h"
#include "ZDProfile.h"
#include "AddrMgr.h"
#include "UsbFunctions.h"

/*********************************************************************
 * LOCAL CONSTANTS
 */

/*********************************************************************
 * LOCAL TYPES
 */
struct ReqSimpleDescMsg {
	uint8       messageCode;
	uint16      nwkAddr;
};

struct ReqActiveEndpointsMsg {
	uint8       messageCode;
	uint16      nwkAddr;
};



struct SendCmdMsg {
	uint8       messageCode;
	uint16      nwkAddr;
	uint8       endpoint;
	uint16		cluster;
	uint16      cmdClusterId;
	uint8		dataLen;
	uint8 		data[];
};

struct WriteAttributeValueMsg {
	uint8       messageCode;
	uint16      nwkAddr;
	uint8       endpoint;
	uint16		cluster;
	uint16      attributeId;
    uint8       dataType;
	uint8       dataValueLen;
	uint8       dataValue[12];
};

struct BindTableRequestMsg {
	uint8		messageCode;
	uint16		shortAddress;
};

struct BindTableEntry {
	uint8 srcEndpoint;	// source endpoint
	uint8 dstGroupMode; // Destination address type; 0 - Normal address index, 1 - Group address
  	uint16 dstIdx;		// dstGroupMode = 0 - Address Manager index
						// dstGroupMode = 1 - Group Address
	uint8 dstEP;		// destination endpoint
 	uint8 numClusterIds;// clusters in the below list
 	uint16 clusterIdList[MAX_BINDING_CLUSTER_IDS]; 
                  
};

struct BindRequest {
	uint8 messageCode;
	uint16 destAddr;
	uint8 outClusterAddr[Z_EXTADDR_LEN]; // Src address
	uint8 outClusterEP;                  			 // Endpoint/interface of source device
	uint16 clusterID;            			 // Cluster ID
	uint8 inClusterAddr[Z_EXTADDR_LEN];          // Destination address
	uint8 inClusterEP;
};



/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * LOCAL TYPEDEFS
 */

/*********************************************************************
 * GLOBAL VARIABLES
 */

/*********************************************************************
 * EXTERNAL VARIABLES
 */


/*********************************************************************
 * LOCAL VARIABLES
 */
static struct ReqActiveEndpointsMsg * reqActiveEndpointsMsg;
static struct ReqAttributeValueMsg * reqAttributeValueMsg;
static struct SendCmdMsg * sendcmdMsg;
static zAddrType_t destAddr;
static afAddrType_t afAddrType;
static zclWriteCmd_t *  zclWriteCmd;
static zclReadCmd_t *readCmd;
static struct WriteAttributeValueMsg * writeAttributeValueMsg;
static struct BindTableRequestMsg * bindTableRequestMsg;
static struct BindRequest * bindRequest;
static ZStatus_t result;


/*********************************************************************
 * LOCAL FUNCTIONS
 */

/*********************************************************************
 * PUBLIC FUNCTIONS
 */

/***********************************************************************************
* @fn           usbNullMessage
*
* @brief        null message
*
* @param        uint8 * data
*
* @return       void
*/
void usbNullMessage(uint8 * data) {
}

/***********************************************************************************
* @fn           usbReqSimpleDesc
*
* @brief        send a request to the device and endpoint in the parameter to send the zigbee simple descriptor
*
* @param        uint8 * data
*
* @return       void
*/
void usbReqSimpleDesc(uint8 * data) {
	//reqSimpleDescMsg = (struct ReqSimpleDescMsg *)data;
	
/*	getSimpleDesc = (GetSimpleDescriptor_t *)zb500Data;
			zAddrType.addrMode = SADDR_MODE_SHORT;
			zAddrType.addr.shortAddr = getSimpleDesc->nwkAddr;
			logDebug( "getting simple descr  for endpoint: %d\n",getSimpleDesc->endPoint);
			afStatus_t status = ZDP_SimpleDescReq(&zAddrType, getSimpleDesc->nwkAddr, getSimpleDesc->endPoint, 0);*/
}

/***********************************************************************************
* @fn           usbReqActiveEndpoint
*
* @brief        send a request to the device in the parameter to send the active endpoints
*
* @param        uint8 * data
*
* @return       void
*/
void usbReqActiveEndpoint(uint8 * data) {
	reqActiveEndpointsMsg = (struct ReqActiveEndpointsMsg *)data;
	destAddr.addrMode = Addr16Bit;
	destAddr.addr.shortAddr = reqActiveEndpointsMsg->nwkAddr;
	
	afStatus_t ret = ZDP_NWKAddrOfInterestReq(&destAddr, reqActiveEndpointsMsg->nwkAddr, Active_EP_req, 0);
	if (ret != afStatus_SUCCESS){
		HAL_TOGGLE_LED1();
	}
}

/***********************************************************************************
* @fn           usbReqActiveEndpoint
*
* @brief        send a request to the device in the parameter to request an attribute value
*
* @param        uint8 * data
*
* @return       void
*/
void usbReqAttributeValue(uint8 * data) {
	reqAttributeValueMsg = (struct ReqAttributeValueMsg *)data;
	
	afAddrType.addr.shortAddr = reqAttributeValueMsg->nwkAddr;
	afAddrType.addrMode = afAddr16Bit;
	afAddrType.endPoint = reqAttributeValueMsg->endpoint;
	
	readCmd = (zclReadCmd_t *)osal_mem_alloc( sizeof ( zclReadCmd_t ) +reqAttributeValueMsg->numAttributes* sizeof(uint16) );
	readCmd->numAttr = reqAttributeValueMsg->numAttributes;
	for (uint8 i=0; i < reqAttributeValueMsg->numAttributes; i++){
		readCmd->attrID[i] = reqAttributeValueMsg->attributeId[i];
	}
	
	ZStatus_t result =  zcl_SendRead( 
							ENDPOINT,
							&afAddrType,
                            reqAttributeValueMsg->cluster,
							readCmd,
                            ZCL_FRAME_CLIENT_SERVER_DIR,
							FALSE,
							0);
	if (result != ZSuccess){
		usbSendAttributeResponseMsgError(reqAttributeValueMsg, result);
	}
}
/***********************************************************************************
* @brief        return to USB the bind table
*
* @param        uint8 * data
*
* @return       void
*/
void usbReqBindTable(uint8 * dat){
	uint8 startIndex=0;
	bindTableRequestMsg = (struct BindTableRequestMsg *)dat;
	destAddr.addrMode = Addr16Bit;
	destAddr.addr.shortAddr = bindTableRequestMsg->shortAddress;
	ZDP_MgmtBindReq( &destAddr, startIndex, 0 );
}

/***********************************************************************************
* @brief        add the bind table the given new entry
*
* @param        uint8 * data
*
* @return       void
*/
void usbAddBindTable(uint8 * dat){
	zAddrType_t inCluster;
	bindRequest = (struct BindRequest *)dat;
	
	inCluster.addrMode = Addr64Bit;
	osal_cpyExtAddr(inCluster.addr.extAddr, bindRequest->inClusterAddr);
	
	destAddr.addrMode = Addr16Bit;
	destAddr.addr.shortAddr = bindRequest->destAddr;
	
	result = ZDP_BindReq(&destAddr,  bindRequest->outClusterAddr, bindRequest->outClusterEP, bindRequest->clusterID, &inCluster,  bindRequest->inClusterEP, 0);
	if (result != SUCCESS){
		HAL_TOGGLE_LED1();
	}
}

/***********************************************************************************
* @brief        remove from the bind table the entry
*
* @param        uint8 * data
*
* @return       void
*/
void usbRemoveBindTable(uint8 * dat){
	zAddrType_t inCluster;
	bindRequest = (struct BindRequest *)dat;
	
	inCluster.addrMode = Addr64Bit;
	osal_cpyExtAddr(inCluster.addr.extAddr, bindRequest->inClusterAddr);
	
	destAddr.addrMode = Addr16Bit;
	destAddr.addr.shortAddr = bindRequest->destAddr;
	
	result = ZDP_UnbindReq(&destAddr,  bindRequest->outClusterAddr, bindRequest->outClusterEP, bindRequest->clusterID, &inCluster,  bindRequest->inClusterEP, 0);

}

/***********************************************************************************
* @brief        send a comand to cluster of  the device in the parameter 
*
* @param        uint8 * data
*
* @return       void
*/
void usbSendCmdCluster(uint8 * data) {
	sendcmdMsg = (struct SendCmdMsg *)data;
	
	afAddrType.addr.shortAddr = sendcmdMsg->nwkAddr;
	afAddrType.addrMode = afAddr16Bit;
	afAddrType.endPoint = sendcmdMsg->endpoint;
	
	zcl_SendCommand(ENDPOINT, &afAddrType, sendcmdMsg->cluster, sendcmdMsg->cmdClusterId, TRUE, ZCL_FRAME_CLIENT_SERVER_DIR, 0, 0, 0, sendcmdMsg->dataLen, sendcmdMsg->data );
}

/***********************************************************************************
* @brief        send a new attribute value to the cluster in the device showed 
*
* @param        uint8 * data aka WriteAttributeValueMsg *
*
* @return       void
*/
void usbWriteAttributeValue(uint8 * data) {
	writeAttributeValueMsg = (struct WriteAttributeValueMsg * )data;
		
	afAddrType.addr.shortAddr = sendcmdMsg->nwkAddr;
	afAddrType.addrMode = afAddr16Bit;
	afAddrType.endPoint = sendcmdMsg->endpoint;
	
	zclWriteCmd = osal_mem_alloc(sizeof(zclWriteCmd_t) + sizeof(zclWriteRec_t));
	zclWriteCmd->numAttr = 1;
	zclWriteCmd->attrList[0].attrID = writeAttributeValueMsg->attributeId;
	zclWriteCmd->attrList[0].dataType = writeAttributeValueMsg->dataType;
	zclWriteCmd->attrList[0].attrData = osal_mem_alloc(writeAttributeValueMsg->dataValueLen);
	osal_memcpy(zclWriteCmd->attrList[0].attrData, writeAttributeValueMsg->dataValue, writeAttributeValueMsg->dataValueLen);
	
	zcl_SendWriteRequest(ENDPOINT, &afAddrType, writeAttributeValueMsg->cluster,zclWriteCmd,  ZCL_CMD_WRITE_NO_RSP, ZCL_FRAME_CLIENT_SERVER_DIR, 0, 0);
	
	osal_mem_free(zclWriteCmd->attrList[0].attrData);
	osal_mem_free(zclWriteCmd);
}

