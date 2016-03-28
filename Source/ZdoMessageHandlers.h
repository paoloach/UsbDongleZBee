/**************************************************************************************************
  Filename:       ZdoMessageHandlers.h


  Copyright 2014 Paolo Achdjian

**************************************************************************************************/

#ifndef __ZDO_MESSAGE_HANDLERS
#define __ZDO_MESSAGE_HANDLERS


#include "ZDProfile.h"
#include "af.h"

/*********************************************************************
 * TYPES
 */
	
typedef void (*ZDOMessageHandler)(zdoIncomingMsg_t * );

/*********************************************************************
 * FUNCTIONS
 */
ZDOMessageHandler ZDOMessageHandlerFactory(cId_t clusterId);
#endif