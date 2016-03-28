/**************************************************************************************************

 DESCRIPTION:
  --

 CREATED: 27/11/2015, by Paolo Achdjian

 FILE: endpointRequestList.c

***************************************************************************************************/
#include "endpointRequestList.h"
#include "TimerEvents.h"
#include "OSAL_Memory.h"
#include "ZDProfile.h"

static struct EndpointRequest * head;
static zAddrType_t zAddrType;
static byte taskId;

void endpointRequestTaskId(byte task_id){
	taskId = task_id;
}

void addEndpointRequest(uint16 nwkAddr, byte endpoint, 	byte security) {
	struct EndpointRequest * next = (struct EndpointRequest *)osal_mem_alloc(sizeof(struct EndpointRequest));
	if (next == NULL){
		return;
	}
	next->endpoint = endpoint;
	next->nwkAddr = nwkAddr;
	next->security = security;
	next->next = head;
	head = next;
}

void sendOneEndpointRequest(void){
	if (head == NULL){
		return;
	}
	zAddrType.addrMode = SADDR_MODE_SHORT;
	zAddrType.addr.shortAddr = head->nwkAddr;
	ZDP_SimpleDescReq(&zAddrType, head->nwkAddr,  head->endpoint, head->security);
	head = head->next;
	if (head != NULL){
		osal_start_timerEx( taskId, ENDPOINT_REQUEST_MSG, ENDPOINT_REQUEST_TIMEOUT );
	}
}

