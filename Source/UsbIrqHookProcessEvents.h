/**************************************************************************************************

 DESCRIPTION:
  --

 CREATED: 17/08/2016, by Paolo Achdjian

 FILE: UsbIrqHookProcessEvents.h

***************************************************************************************************/
#include "OSAL.h"

#ifndef __USB_IRQ_HOOK_PRECESS_EVENTS__H__
#define __USB_IRQ_HOOK_PRECESS_EVENTS__H__


typedef void (*IsrMethod)(osal_event_hdr_t *);

struct UsbISR {
	osal_event_hdr_t msg;
	IsrMethod isr;
	uint16	time;
};


struct ReqActiveEndpointsEvent {
	struct UsbISR	isr;
	union  {
		uint16      nwkAddr;
		uint8		data[2];
	};
};

struct ReqAttributeMsg {
	struct UsbISR	isr;
	afAddrType_t afAddrType;
	uint16	cluster;
	uint8  numAttr;   
	uint16 attrID[];
};

struct SendCmdMsg{
	struct UsbISR	isr;
	afAddrType_t afAddrType;
	uint16		cluster;
	uint16      cmdClusterId;
	uint8		dataLen;
	uint8 		data[];
};

struct BindTableRequestMsg{
	struct UsbISR	isr;
	zAddrType_t afAddrType;
};

struct WriteAttributeValueMsg {
	struct UsbISR	isr;
	afAddrType_t afAddrType;
	uint16 		 cluster;
	zclWriteCmd_t writeCmd;
};

struct BindRequestMsg {
	struct UsbISR	isr;
	 zAddrType_t destAddr;
	 uint8 outClusterAddr[Z_EXTADDR_LEN];
	 uint8 outClusterEP;     
	 uint16 clusterID; 
	 zAddrType_t inCluster;
	 uint8 inClusterEP;
};

#endif