/**************************************************************************************************

 DESCRIPTION:
  --

 CREATED: 03/03/2015, by Paolo Achdjian

 FILE: UsbFifoData.c

***************************************************************************************************/
#include "UsbFifoData.h"
#include "OSAL_Memory.h"
#include "OSAL.h"
#include "ioCC2530.h"
#include "hal_usbdongle_cfg.h"

static struct UsbFifoData * head=NULL;
static struct UsbFifoData * last;
static uint8 count=0;

struct UsbFifoData * getUsbFifoHead(void) {
	return head;
}

uint8 isFifoEmpty(void) {
	return head == NULL;
}

void usbFifoDataPushWithType(uint8 type,const uint8 * data, uint8 len) {
	struct UsbFifoData * element = osal_mem_alloc(sizeof(struct UsbFifoData) + len+1);
	if (element == NULL){
		return;
	}
	
	element->dataLen = len+1;
	element->next = NULL;
	element->data[0] = type;
	osal_memcpy(&element->data[1], data, len);
	if (last == NULL){
		head = element;
		last = element;
	} else {
		last->next = element;
		last = element;
	}
}

void usbFifoDataPush(const uint8 * data, uint8 len) {
	struct UsbFifoData * element = osal_mem_alloc(sizeof(struct UsbFifoData) + len);
	if (element == NULL){
		HAL_TURN_ON_LED1();
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