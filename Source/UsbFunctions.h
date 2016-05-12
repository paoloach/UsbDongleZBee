/**************************************************************************************************
  Filename:       UsbFunctions.h


  Copyright 2014 Paolo Achdjian

**************************************************************************************************/


#ifndef __USB_FUNCTIONS__H__
#define __USB_FUNCTIONS__H__

#include "ZDObject.h"
#include "zcl.h"

#define BULK_SIZE_OUT 64
#define BULK_SIZE_IN 128
#define MAX_DATE_SIZE_3 128
#define MAX_DATE_SIZE_2 64

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
#define ATTRIBUTE_VALUE_REQ_ERROR	0x40

struct GenericDataMsg{
	uint8 msgCode;
};

struct BindTableResponseEntry{
	uint16 srcAddr;
 	byte srcEP; 
 	uint16 clusterID;
 	uint16 dstAddr;
 	byte dstEP; 
};

struct BindTableResponse {
	struct GenericDataMsg generticDataMsg;
	uint8 elementSize;
	struct BindTableResponseEntry list[];
};

struct ReqAttributeValueMsg {
	uint8       messageCode;
	uint16      nwkAddr;
	uint8       endpoint;
	uint16		cluster;
	uint8		numAttributes;
	uint16      attributeId[];
};

void Usb_ProcessLoop(void);
void usbSendAnnunce(ZDO_DeviceAnnce_t * device);
void usbSendSimpleDescriptor(ZDO_SimpleDescRsp_t * simpleDesc);
void usbSendAttributeResponseMsg(zclReadRspCmd_t * readRspCmd, uint16 cluster, afAddrType_t * address );
void usbSendAttributeResponseMsgError(struct ReqAttributeValueMsg *, ZStatus_t status);
void sendUsb(const uint8 * data, uint8 len);
void sendFifo(void);
void usbSendDataChunk( uint8 type, uint8 * data, uint8 len);


#endif