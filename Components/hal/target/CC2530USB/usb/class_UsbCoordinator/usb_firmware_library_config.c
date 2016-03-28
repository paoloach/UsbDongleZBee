/***********************************************************************************

    Filename:     usb_firmware_library_config.c

    Description:  USB library configuration.

***********************************************************************************/

/// \addtogroup module_usb_firmware_library_config
/// @{
#define USBFIRMWARELIBRARYCONFIG_C ///< Modifies the behavior of "EXTERN" in usb_interrupt.h
#include "usb_firmware_library_headers.h"

//-----------------------------------------------------------------------------
// READ THIS!!
//
// This file configures the USB Firmware Library.
// To use the library, make a copy of this file, rename it to "usb_firmware_library_config.c", and
// put it in the project catalog. Then edit the code below as needed:
//-----------------------------------------------------------------------------

// Declaration of global USB descriptor pointers
USB_DESCRIPTOR_MARKER usbDescriptorMarker= {
    (uint8 __code *)&usbDescStart,
    (uint8 __code *)&usbDescEnd,
    (DESC_LUT_INFO __code *) &usbDescLut,
    (DESC_LUT_INFO __code *) &usbDescLutEnd,
    (DBLBUF_LUT_INFO __code *) &usbDblbufLut,
    (DBLBUF_LUT_INFO __code *) &usbDblbufLutEnd
};

/// @}



