/**************************************************************************************************

 DESCRIPTION:
  --

 CREATED: 09/02/2015, by Paolo Achdjian

 FILE: DeviceList.h

***************************************************************************************************/
#ifndef __DEVICE_LIST_H__
#define __DEVICE_LIST_H__

#include "ZDObject.h"

struct DeviceList {
	ZDO_DeviceAnnce_t device;
	struct DeviceList * next;
};


uint8 getCount(void);
struct DeviceList * deviceListHead(void);
struct DeviceList * deviceListLast(void);
void deviceListAdd(ZDO_DeviceAnnce_t* device);


#endif
