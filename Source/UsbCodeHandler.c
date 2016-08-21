

#include "usb_firmware_library_headers.h"
#include "zigbee_functions.h"
#include "zigbee_requests.h"
#include "UsbCodeHandler.h"



/***********************************************************************************
* @fn           ZigBeeInitUSB
*
* @brief        USB init function.
*
* @param        none
*
* @return       none
*/
void ZigBeeInitUSB(void){
	// Init USB library
 	 usbfwInit();

	// Initialize the USB interrupt handler with bit mask containing all processed USBIRQ events
	usbirqInit(USBIRQ_EVENT_SUSPEND | USBIRQ_EVENT_RESUME | USBIRQ_EVENT_RESET | USBIRQ_EVENT_SETUP | USBIRQ_EVENT_EP5IN | USBIRQ_EVENT_EP2OUT);

	// Enable pullup on D+
	HAL_USB_PULLUP_ENABLE();
}

/** 
*  Class requests with OUT data phase for endpoint 0
* No data out
*/
 void usbcrHookProcessOut(void)  {
    usbfwData.ep0Status = EP_STALL;
 }
 
/** 
*  Class requests with IN data phase for endpoint 0
* No data in
*/
 void usbcrHookProcessIn(void)  {
    usbfwData.ep0Status = EP_STALL;
 }
 
/** 
*  vendor requests with OUT data phase for endpoint 0
*  Supported commands:
*    requestDevices 
*/
 void usbvrHookProcessOut(void)  {
	usbfwData.ep0Status = EP_STALL;  // Default state usend in case of error
	
	switch(usbSetupHeader.request){
	case REQUEST_DEVICES:
		  // First the endpoint status is EP_IDLE...
		if (usbfwData.ep0Status == EP_IDLE) {
        	// This command doesn't need any data
			if (usbSetupHeader.length == 0){
				requestDevices();
			} 
		} 
		break;
	default:
		 // unknow request
		usbfwData.ep0Status = EP_STALL; 
	}
 }
 

/** 
*  vendor requests with IN data phase for endpoint 0
* No data in
*/
 void usbvrHookProcessIn(void)  {
    usbfwData.ep0Status = EP_STALL;
 }

// ************************  unsupported/unhandled standard requests ***************
void usbsrHookSetDescriptor(void) {
	usbfwData.ep0Status = EP_STALL;
}

void usbsrHookSynchFrame(void) {
	usbfwData.ep0Status = EP_STALL;
}

void usbsrHookClearFeature(void) {
	usbfwData.ep0Status = EP_STALL;
}
void usbsrHookSetFeature(void) {
	usbfwData.ep0Status = EP_STALL;
}
void usbsrHookModifyGetStatus(uint8 recipient, uint8 index, uint16 __xdata *pStatus) {
}

// ************************ USB standard request event processing ******************
void usbsrHookProcessEvent(uint8 event, uint8 index)
{
   // Process relevant events, one at a time.
   switch (event) {
   case USBSR_EVENT_CONFIGURATION_CHANGING : //(the device configuration is about to change)
      break;
   case USBSR_EVENT_CONFIGURATION_CHANGED :// (the device configuration has changed)
      break;
   case USBSR_EVENT_INTERFACE_CHANGING ://(the alternate setting of the given interface is about to change)
      break;
   case USBSR_EVENT_INTERFACE_CHANGED : //(the alternate setting of the given interface has changed)
      break;
   case USBSR_EVENT_REMOTE_WAKEUP_ENABLED ://(remote wakeup has been enabled by the host)
      break;
   case USBSR_EVENT_REMOTE_WAKEUP_DISABLED ://(remote wakeup has been disabled by the host)
      break;
   case USBSR_EVENT_EPIN_STALL_CLEARED ://(the given IN endpoint's stall condition has been cleared the host)
      break;
   case USBSR_EVENT_EPIN_STALL_SET ://(the given IN endpoint has been stalled by the host)
      break;
   case USBSR_EVENT_EPOUT_STALL_CLEARED ://(the given OUT endpoint's stall condition has been cleared the host)
      break;
   case USBSR_EVENT_EPOUT_STALL_SET ://(the given OUT endpoint has been stalled by the PC)
      break;
   }
}




