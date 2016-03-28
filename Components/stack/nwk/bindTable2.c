/**************************************************************************************************

 DESCRIPTION:
  --

 CREATED: 27/02/2015, by Paolo Achdjian

 FILE: bindTable2.c

***************************************************************************************************/
#include "bindTable2.h"
#include "BindingTable.h"
#include "OSAL.h"
#include "OSAL_Memory.h"


struct DstBindTableEntry {
	uint8 ep;
	uint16 shortAddr;
	struct DstBindTableEntry * next;
};
	
struct SrcBindTableEntry {
	uint8 ep;
	uint16 shortAddr;
	uint16 clusterId;
	struct DstBindTableEntry * dst;
	struct SrcBindTableEntry * next;
};

BindingEntry_t *(*pbindAddEntry)( byte srcEpInt,
                                  zAddrType_t *dstAddr, byte dstEpInt,
                                  byte numClusterIds, uint16 *clusterIds ) = (void*)NULL;
uint16 (*pbindNumOfEntries)( void ) = (void*)NULL;
void (*pbindRemoveDev)( zAddrType_t *Addr ) = (void*)NULL;

static struct SrcBindTableEntry * head = NULL;

static BindingEntry_t result;

static struct SrcBindTableEntry * findSource(uint16 shortAddr, uint8 ep, uint16 cluster);
static struct SrcBindTableEntry * findSourceEndpoint(uint16 shortAddr, uint8 ep);
static byte createSrc( struct BindingEntry * bindEntry);
static struct DstBindTableEntry * createDst( struct BindingEntry * bindEntry);
static byte createDestIfNotFind(struct SrcBindTableEntry * src, uint16 shortAddress, uint8 ep);


void InitBindingTable( void ){
	head = NULL;
}

byte bind( struct BindingEntry * bindEntry ){
	if (head == NULL){
		return createSrc(bindEntry);
	} else {
		struct SrcBindTableEntry * src = findSource( bindEntry->srcShortAddress,bindEntry->srcEP, bindEntry->cluster);
		if (src == NULL){
			return createSrc(bindEntry);
		} 
		return createDestIfNotFind(src, bindEntry->dstShortAddress, bindEntry->dstEP);
	}
}

byte createDestIfNotFind(struct SrcBindTableEntry * src, uint16 shortAddress, uint8 ep){
	struct DstBindTableEntry * tempDest = src->dst;
	while (tempDest != NULL){
		if (tempDest->shortAddr == shortAddress && tempDest->ep == ep){
			return 1;
		}
		tempDest = tempDest->next;
	}
	struct DstBindTableEntry * dest = osal_mem_alloc(sizeof (struct DstBindTableEntry));
	if (dest == NULL){
		return 0;
	}
	dest->ep =ep;
	dest->shortAddr = shortAddress;
	dest->next = src->dst;
	src->dst = dest;
	return 1;
}

static struct DstBindTableEntry * createDst( struct BindingEntry * bindEntry){
	struct DstBindTableEntry * dest = osal_mem_alloc(sizeof (struct DstBindTableEntry));
	if (dest == NULL){
		return NULL;
	}
	dest->ep = bindEntry->dstEP;
	dest->shortAddr = bindEntry->dstShortAddress;
	dest->next = NULL;
	return dest;
}

byte createSrc( struct BindingEntry * bindEntry) {
	struct SrcBindTableEntry * src;
	src = osal_mem_alloc(sizeof (struct SrcBindTableEntry));
	if (src != NULL){
		src->ep = bindEntry->srcEP;
		src->shortAddr = bindEntry->srcShortAddress;
		src->clusterId = bindEntry->cluster;
		src->next = head;
		
		struct DstBindTableEntry * dest = createDst(bindEntry);
		if (dest == NULL){
			osal_mem_free(src);
			return 0;
		}
		src->dst = dest;
		head = src;
	}
	return 1;
}

static struct SrcBindTableEntry * findSource(uint16 shortAddr, uint8 ep, uint16 cluster){
	struct SrcBindTableEntry * iter = head;
	while (iter != NULL){
		if (iter->shortAddr == shortAddr && iter->ep==ep && iter->clusterId == cluster){
			return iter;
		}
		iter = iter->next;
	}
	return NULL;
}

static struct SrcBindTableEntry * findSourceEndpoint(uint16 shortAddr, uint8 ep){
	struct SrcBindTableEntry * iter = head;
	while (iter != NULL){
		if (iter->shortAddr == shortAddr && iter->ep==ep){
			return iter;
		}
		iter = iter->next;
	}
	return NULL;
}

BindingEntry_t *bindFindExisting( byte srcEpInt, zAddrType_t *dstShortAddr, byte dstEpInt ){
	return NULL;
}

byte bindIsClusterIDinList( BindingEntry_t *entry, uint16 clusterId ){
	return 0;
}

 byte bindRemoveClusterIdFromList( BindingEntry_t *entry, uint16 clusterId ){
	 return 1;
}

byte bindRemoveEntry( BindingEntry_t *pBind ){
	return 0;
}

void bindAddressClear( uint16 dstIdx ){
}

BindingEntry_t *GetBindingTableEntry( uint16 Nth ){
	return NULL;
}

uint8 bindingAddrMgsHelperConvert( uint16 idx, zAddrType_t *addr ) {
	return 0;
}

uint16 bindingAddrMgsHelperConvertShort( uint16 idx ){
	return idx;
}

uint16 bindNumReflections( uint8 ep, uint16 clusterID ) {
}

BindingEntry_t *bindFind( uint8 ep, uint16 clusterID, uint8 skipping ) {
	return NULL;
}

uint16 bindAddrIndexGet( zAddrType_t* addr ){
}