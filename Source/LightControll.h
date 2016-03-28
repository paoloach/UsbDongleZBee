#ifndef __LIGHT_CONTROLL_H_
#define __LIGHT_CONTROLL_H_

#include "ZDObject.h"

struct LightDevice {
	uint16 nwkAddr;
	byte   endPoint;
};

void addLight(ZDO_SimpleDescRsp_t * simpleDescr);

const struct LightDevice * getFirstLight(void);

void setLightOff(const struct LightDevice *lightDevice);
void setLightOn(const struct LightDevice *lightDevice);
void setLightToggle(const struct LightDevice *lightDevice);
void readLightValue(const struct LightDevice *lightDevice);

#endif