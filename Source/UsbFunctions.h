/**************************************************************************************************
  Filename:       UsbFunctions.h


  Copyright 2014 Paolo Achdjian

**************************************************************************************************/


#ifndef __USB_FUNCTIONS__H__
#define __USB_FUNCTIONS__H__

#include "ZDObject.h"
#include "zcl.h"
#include "UsbIrqHookProcessEvents.h"

#define MAX_DATE_SIZE_5 64
#define MAX_DATE_SIZE_2 64

#define ENDPOINT_LOG 4
#define USB_LOG_FIFO USBF4
#define ENDPOINT_LOG_SIZE 64

#define Z_EXTADDR_LEN   8
#define ANNUNCE 0x01
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
#define REQ_RESET				    0x10
#define INFO_MESSAGE				0x20
#define ATTRIBUTE_VALUE_REQ_ERROR	0x40
#define ACTIVE_EP_REQ_ERROR			0x41


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

typedef void (*UsbMessageHandler)(uint8 * data);

void Usb_ProcessLoop(void);
void handleUsbOut(void);
UsbMessageHandler parseDataOut(void);
void requestAllDevices(void);
void usbSendAnnunce(ZDO_DeviceAnnce_t * device);
void usbSendSimpleDescriptor(ZDO_SimpleDescRsp_t * simpleDesc);
void usbSendAttributeResponseMsg(zclReadRspCmd_t * readRspCmd, uint16 cluster, afAddrType_t * address );
void usbSendAttributeResponseMsgError(struct ReqAttributeMsg *, ZStatus_t status);
void sendUsb(const uint8 * data, uint8 len);
void sendFifo(void);
void usbSendActiveEPError(uint16 nwkAddr, uint8 errorCode);
void usbLog(uint16 nwkId, const char * msg,...);
void usbLogString(const char * msg);
char * clusterRequestToString(uint16 clusterId);
char * convertUint16ToHex(uint16 num);
void requestAllDevices2(uint8 * notUsed);

#endif