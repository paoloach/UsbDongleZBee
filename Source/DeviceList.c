/**************************************************************************************************

 DESCRIPTION:
  --

 CREATED: 09/02/2015, by Paolo Achdjian

 FILE: DeviceList.c

***************************************************************************************************/

#include "DeviceList.h"
#include "OSAL_Memory.h"
#include "OSAL.h"

static uint8 notExist(ZDO_DeviceAnnce_t* device);

static struct DeviceList * head=NULL;
static struct DeviceList * last=NULL;

uint8 getCount(void){
	if (head == NULL){
		return 0;
	} else {
		uint8 count=0;		
		struct DeviceList * tmp = head;
		while (tmp != NULL){
			tmp = tmp->next;
			count++;
		}
		return count;
	}
}

struct DeviceList * deviceListHead(void){
	return head;
}

struct DeviceList * deviceListLast(void){
	return last;
}

void deviceListAdd(ZDO_DeviceAnnce_t* device){
	if (head == NULL){
		head = osal_mem_alloc(sizeof(struct DeviceList));
		last = head;
		last->next = NULL;
		osal_memcpy(&(head->device), device, sizeof(ZDO_DeviceAnnce_t));
	} else {
		if (notExist(device)){
			struct DeviceList * new = osal_mem_alloc(sizeof(struct DeviceList));
			last->next = new;
			last = new;
			last->next = NULL;
			osal_memcpy(&(last->device), device, sizeof(ZDO_DeviceAnnce_t));
		}
	}
}

static uint8 notExist(ZDO_DeviceAnnce_t* device) {
	struct DeviceList * iter = head;
	while (iter != NULL){
		if (iter->device.nwkAddr != 0){
			if (iter->device.nwkAddr != device->nwkAddr){
				iter = iter->next;
				continue;
			}else
				return 1;
		} else {
			if (osal_memcmp(iter->device.extAddr, device->extAddr, Z_EXTADDR_LEN) == FALSE){
				iter = iter->next;
				continue;
			}else
				return 1;
		}
	}
	return 0;
}