/**************************************************************************************************

 DESCRIPTION:
  --

 CREATED: 30/04/2016, by Paolo Achdjian

 FILE: DeviceManager.h

***************************************************************************************************/

#ifndef __DEVICE_MANAGER__H__
#define __DEVICE_MANAGER__H__

#include "ZDObject.h"
void deviceManagerInit(void);
void addDevice(ZDO_DeviceAnnce_t * add);
uint8 deviceNwkAddrLookup( uint8* extAddr, uint16* nwkAddr);
ZDO_DeviceAnnce_t * deviceEntryGet(uint8 index);

#define DEVICE_MANAGER_TABLE_SIZE	40

#endif