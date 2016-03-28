/**************************************************************************************************

 DESCRIPTION:
  --

 CREATED: 27/11/2015, by Paolo Achdjian

 FILE: endpointRequestList.h

***************************************************************************************************/
#ifndef __ENDPOINT_REQUEST_LIST__H__
#define __ENDPOINT_REQUEST_LIST__H__

#include "hal_types.h"
#include "ZComDef.h"

#define ENDPOINT_REQUEST_TIMEOUT   1000     // Every 100 ms

struct EndpointRequest {
	uint16 nwkAddr;
	byte endpoint;
	byte security;
	struct EndpointRequest * next;
};

void endpointRequestTaskId(byte taskId);
void addEndpointRequest(uint16 nwkAddr, byte endpoint, 	byte security);
void sendOneEndpointRequest(void);

#endif
