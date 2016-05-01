/**************************************************************************************************

 DESCRIPTION:
  --

 CREATED: 30/04/2016, by Paolo Achdjian

 FILE: DeviceManager.c

***************************************************************************************************/


#include "AddrMgr.h"
#include "OSAL_Memory.h"
#include "saddr.h"
#include "OSAL_Nv.h"

#include "DeviceManager.h"

#define TABLE_SIZE	DEVICE_MANAGER_TABLE_SIZE
#define NV_ID		0x402
#define INVALID		0xFFFE

static ZDO_DeviceAnnce_t table[TABLE_SIZE];


void deviceManagerInit(void){
	ZDO_DeviceAnnce_t * iter, *end;
	end = table +TABLE_SIZE;
	for(iter = table; iter != end; iter++){
		iter->nwkAddr=INVALID;
	}
	osal_nv_item_init(NV_ID, sizeof(ZDO_DeviceAnnce_t)*TABLE_SIZE, table);
	uint16 size = osal_nv_item_len(NV_ID);
	if (size > TABLE_SIZE*sizeof(ZDO_DeviceAnnce_t)){
		size  = TABLE_SIZE*sizeof(ZDO_DeviceAnnce_t);
	}
	osal_nv_read(NV_ID, 0, size, table);
}

void addDevice( ZDO_DeviceAnnce_t * add){
	uint16 nwkAddr;
	uint16 offset;
	ZDO_DeviceAnnce_t * iter, *end;
	end = table +TABLE_SIZE;
	
	if (AddrMgrNwkAddrLookup(add->extAddr, &nwkAddr)==TRUE){
		return;
	}
	
	for(iter=table; iter != end; iter++){
		if (iter->nwkAddr != INVALID && sAddrExtCmp(iter->extAddr, add->extAddr)==TRUE){
			return;
		}
	}
	for(iter=table,offset=0; iter != end; iter++,offset+=sizeof(ZDO_DeviceAnnce_t)){
		if (iter->nwkAddr==INVALID){
			iter->capabilities = add->capabilities;
			iter->nwkAddr = add->nwkAddr;
			sAddrExtCpy(iter->extAddr, add->extAddr);
			osal_nv_write(NV_ID,offset, sizeof(ZDO_DeviceAnnce_t), iter);
			return;
		}
	}
	
}

ZDO_DeviceAnnce_t * deviceEntryGet(uint8 index) {
	if (index > TABLE_SIZE){
		return NULL;
	}
	if (table[index].nwkAddr == INVALID){
		return NULL;
	}
	return &table[index];
}

uint8 deviceNwkAddrLookup( uint8* extAddr, uint16* nwkAddr) {
	ZDO_DeviceAnnce_t * iter, *end;
	end = table +TABLE_SIZE;
	if (AddrMgrNwkAddrLookup(extAddr, nwkAddr)==TRUE){
		return TRUE;
	}
	for(iter=table; iter != end; iter++){
		if (sAddrExtCmp(iter->extAddr, extAddr)==TRUE){
			*nwkAddr = iter->nwkAddr;
			return TRUE;
		}
	}
	return FALSE;
}

