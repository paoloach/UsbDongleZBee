/**************************************************************************************************

 DESCRIPTION:
  --

 CREATED: 03/03/2015, by Paolo Achdjian

 FILE: UsbFifoData.h

***************************************************************************************************/
#ifndef __USB_FIFO_DATA__H__
#define __USB_FIFO_DATA__H__

#include "hal_types.h"



struct UsbFifoData {
	uint8 dataLen;
	struct UsbFifoData * next;
	uint8 data[];
};

void usbFifoDataPush(const uint8 * data, uint8 len);
void usbFifoDataPushWithType(uint8 type, const uint8 * data, uint8 len);
struct UsbFifoData * getUsbFifoHead(void);
void usbFifoPop(void);

#endif