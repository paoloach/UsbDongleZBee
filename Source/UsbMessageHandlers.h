/**************************************************************************************************
  Filename:       UsbMessageHandlers.h


  Copyright 2014 Paolo Achdjian

**************************************************************************************************/

#ifndef __USB_MESSAGE_HANDLERS__H_
#define __USB_MESSAGE_HANDLERS__H_

#include "hal_types.h"
#include "osal.h"
#include "af.h"
#include "zcl.h"
#include "AddrMgr.h"

void usbNullMessage(uint8 * data);
void reset(uint8 * data);
void usbReqSimpleDesc(uint8 * data);
void usbReqActiveEndpoint(uint8 * data);
void usbReqAttributeValue(uint8 * data);
void usbSendCmdCluster(uint8 * data);
void usbWriteAttributeValue(uint8 * data);
void requestAllDevices(uint8 * data);
void requestAllDevices2(uint8 * data);
void usbReqBindTable(uint8 * dat);
void usbAddBindTable(uint8 * dat);
void usbRemoveBindTable(uint8 * dat);


void addrMgrUserCB( uint8  update,  AddrMgrEntry_t* newEntry,  AddrMgrEntry_t* oldEntry);

struct ReqAttributeMsg {
	osal_event_hdr_t hdrEvent;
	afAddrType_t afAddrType;
	uint16	cluster;
	zclReadCmd_t readCmd;
};
#endif