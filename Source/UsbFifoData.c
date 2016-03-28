/**************************************************************************************************

 DESCRIPTION:
  --

 CREATED: 03/03/2015, by Paolo Achdjian

 FILE: UsbFifoData.c

***************************************************************************************************/
#include "UsbFifoData.h"
#include "OSAL_Memory.h"
#include "OSAL.h"


static struct UsbFifoData * head=NULL;
static struct UsbFifoData * last;

struct UsbFifoData * getUsbFifoHead(void) {
	return head;
}

void usbFifoDataPush(const uint8 * data, uint8 len) {
	struct UsbFifoData * element = osal_mem_alloc(sizeof(struct UsbFifoData) + len);
	if (element == NULL){
		return;
	}
	
	element->dataLen = len;
	element->next = NULL;
	osal_memcpy(&element->data, data, len);
	if (last == NULL){
		head = element;
		last = element;
	} else {
		last->next = element;
		last = element;
	}
}

void usbFifoPop(void) {
	if (head == NULL){
		return;
	}
	struct UsbFifoData * toRemove = head;
	head = head->next;
	if (head == NULL){
		last = NULL;
	}
	osal_mem_free(toRemove);
}