/**************************************************************************************************

 DESCRIPTION:
  --

 CREATED: 09/03/2016, by Paolo Achdjian

 FILE: zcl_LinkQuality.c

***************************************************************************************************/
#include "zcl_LinkQuality.h"
#include "AddrMgr.h"
#include "OSAL_Memory.h"
struct LinkQuality * head;


void updateLinkQuality(afIncomingMSGPacket_t * inPck){
	uint8 * extAddr;
	if (inPck->srcAddr.addrMode == afAddr16Bit){
		AddrMgrEntry_t entry;
		
		entry.nwkAddr = inPck->srcAddr.addr.shortAddr;
		if ( AddrMgrEntryLookupNwk( &entry ) == FALSE ){
			return;
		}
		extAddr = entry.extAddr;
	} else {
		extAddr = extAddr;
	}
	if (head == NULL){
		head = osal_mem_alloc(sizeof(struct LinkQuality));
		head->correlation = inPck->correlation;
		head->LinkQuality = inPck->LinkQuality;
		head->rssi = inPck->rssi;
		osal_memcpy(head->extAddr, inPck->srcAddr.addr.extAddr, Z_EXTADDR_LEN);
		head->next=NULL;
	} else {
		struct LinkQuality * iter = head;
		while (iter != NULL){
			if (osal_memcmp(iter->extAddr, extAddr,Z_EXTADDR_LEN)){
				iter->correlation = inPck->correlation;
				iter->LinkQuality = inPck->LinkQuality;
				iter->rssi = inPck->rssi;
				return;
			}
			iter = iter->next;
		}
		iter = osal_mem_alloc(sizeof(struct LinkQuality));
		iter->correlation = inPck->correlation;
		iter->LinkQuality = inPck->LinkQuality;
		iter->rssi = inPck->rssi;
		osal_memcpy(iter->extAddr, extAddr, Z_EXTADDR_LEN);
		iter->next=head;
		head=iter;
	}
	
}
struct LinkQuality * getLinkQuality(uint8 extAddr[Z_EXTADDR_LEN]){
	struct LinkQuality * iter = head;
	while (iter != NULL){
		if (osal_memcmp(iter->extAddr, extAddr,Z_EXTADDR_LEN)){
			return iter;
		}
		iter = iter->next;
	}
	return NULL;
}
