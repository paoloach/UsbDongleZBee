/**************************************************************************************************

 DESCRIPTION:
  --

 CREATED: 11/02/2015, by Paolo Achdjian

 FILE: TimerEvents.h

***************************************************************************************************/

// Application Events (OSAL) - These are bit weighted definitions.
#define GENERICAPP_SEND_MSG_EVT       0x0001
// USB Annunce message
#define USB_ANNUNCE_MSG				  0x0002
#define ENDPOINT_REQUEST_MSG		  0x0004
#define SEND_FIFO_DATA				  0x0008
#define USB_ANNUNCE2_MSG			  0x0010
#define MEM_INFO					  0x0020
#define EVENT_USB_ISR					0xE0

