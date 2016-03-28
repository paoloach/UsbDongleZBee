/**************************************************************************************************

 DESCRIPTION:
  --

 CREATED: 03/03/2015, by Paolo Achdjian

 FILE: BindTableList.h

***************************************************************************************************/
#ifndef __BIND_TABLE_LIST_H
#define __BIND_TABLE_LIST_H

#include "UsbFunctions.h"

struct BindTableList  {
	struct BindResponse device;
	struct BindTableList * next;
};

struct BindTableList * deviceListHead(void);
struct BindTableList * deviceListLast(void);
void deviceListAdd(apsBindingItem_t * device);


#endif
