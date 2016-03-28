/**************************************************************************************************

 DESCRIPTION:
  --

 CREATED: 03/03/2015, by Paolo Achdjian

 FILE: BindTableList.c

***************************************************************************************************/

#include "BindTableList.h"

static struct BindTableList * head=NULL;
static struct BindTableList * last=NULL;

struct BindTableList * deviceListHead(void){
	return head;
}
struct BindTableList * deviceListLast(void) {
	return last;
}
void deviceListAdd(apsBindingItem_t * device){
	/*
	if (head == NULL){
		head = osal_mem_alloc(sizeof(struct BindTableList));
		last = head;
		last->next = NULL;
		osal_memcpy(&(head->device), device, sizeof(struct BindTableList));
	} else {
		if (notExist(device)){
			struct DeviceList * new = osal_mem_alloc(sizeof(struct BindTableList));
			last->next = new;
			last = new;
			last->next = NULL;
			osal_memcpy(&(last->device), device, sizeof(ZDO_DeviceAnnce_t));
		}
	}
	*/
}

