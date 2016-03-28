/**************************************************************************************************
  Filename:       UsbFunctions.h


  Copyright 2014 Paolo Achdjian

**************************************************************************************************/


#ifndef __USB_FUNCTIONS__H__
#define __USB_FUNCTIONS__H__

#include "ZDObject.h"
#include "zcl.h"

/*********************************************************************
 * TYPES
 */

/*********************************************************************
 * FUNCTIONS
 */



void Usb_ProcessLoop(void);
void usbSendAnnunce(ZDO_DeviceAnnce_t * device);
void usbSendSimpleDescriptor(ZDO_SimpleDescRsp_t * simpleDesc);
void usbSendAttributeResponseMsg(zclReadRspStatus_t *pInMsg, uint16 cluster, afAddrType_t * address );
void usbSendBindTables( apsBindingItem_t list[], uint8 bindingListCount);

#endif