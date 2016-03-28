#include "LightControll.h"
#include "OSAL_Memory.h"
#include "AF.h"
#include "zcl.h"
#include "zcl_general.h"
#include "zcl_UsbCoordinator.h"

struct LightDeviceList {
	struct LightDevice lightDevice;
	struct LightDeviceList  * next;
};

static struct LightDeviceList * listHead = NULL;
static struct LightDeviceList * listBottom = NULL;


void addLight(ZDO_SimpleDescRsp_t * simpleDescr){
	if (listHead == NULL){
		listBottom = listHead = osal_mem_alloc(sizeof(struct LightDevice));
	} else {
		listBottom->next =  osal_mem_alloc(sizeof(struct LightDevice));	
		listBottom = listBottom->next;
	}
	listBottom->lightDevice.nwkAddr = simpleDescr->nwkAddr;
	listBottom->lightDevice.endPoint = simpleDescr->simpleDesc.EndPoint;
	listBottom->next=NULL;
}

const struct LightDevice * getFirstLight(void) {
	if (listHead != NULL){
		return &listHead->lightDevice;
	} else {
		return NULL;
	}
}

void setLightOff(const struct LightDevice *lightDevice) {
	if (lightDevice != NULL){
		afAddrType_t addrType;
		addrType.addr.shortAddr = lightDevice->nwkAddr;
		addrType.addrMode = afAddr16Bit;
		addrType.endPoint = lightDevice->endPoint;
	
		
	}
}


void setLightOn(const struct LightDevice *lightDevice) {
	if (lightDevice != NULL){
		afAddrType_t addrType;
		addrType.addr.shortAddr = lightDevice->nwkAddr;
		addrType.addrMode = afAddr16Bit;
		addrType.endPoint = lightDevice->endPoint;
	
		
	}
}

void readLightValue(const struct LightDevice *lightDevice) {
	if (lightDevice != NULL){
		afAddrType_t addrType;
		addrType.addr.shortAddr = lightDevice->nwkAddr;
		addrType.addrMode = afAddr16Bit;
		addrType.endPoint = lightDevice->endPoint;
	
		zclReadCmd_t *readCmd;
		readCmd = (zclReadCmd_t *)osal_mem_alloc( sizeof ( zclReadCmd_t ) + sizeof(uint16) );
		readCmd->numAttr = 1;
		readCmd->attrID[0] = 0;
	
		zcl_SendRead(SAMPLELIGHT_ENDPOINT, &addrType, ZCL_CLUSTER_ID_GEN_ON_OFF, readCmd,ZCL_FRAME_CLIENT_SERVER_DIR, FALSE, 0 );
	}
}


void setLightToggle(const struct LightDevice *lightDevice) {
	if (lightDevice != NULL){
		afAddrType_t addrType;
		addrType.addr.shortAddr = lightDevice->nwkAddr;
		addrType.addrMode = afAddr16Bit;
		addrType.endPoint = lightDevice->endPoint;
	
		
	}
}

