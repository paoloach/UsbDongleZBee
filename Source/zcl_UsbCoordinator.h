/**************************************************************************************************

 DESCRIPTION:
  --

 CREATED: 18/02/2015, by Paolo Achdjian

 FILE: zcl_UsbCoordinator.h

***************************************************************************************************/

#ifndef ZCL_SAMPLELIGHT_H
#define ZCL_SAMPLELIGHT_H

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDES
 */
#include "zcl.h"
#include "ZDProfile.h"


/*********************************************************************
 * TYPEDEFS
 */
	
enum Coordinator_TYPE_MSG {
	GetSimpleDescritorMsgType
	};	
	
typedef void (*ResponseParser)(zdoIncomingMsg_t * );

typedef struct
{
  osal_event_hdr_t  hdr;
  enum  Coordinator_TYPE_MSG typeMsg;
} CoodinatorData_t;

typedef struct {
	osal_event_hdr_t  hdr;
 	enum  Coordinator_TYPE_MSG typeMsg;
	uint16 			nwkAddr;
	uint8			endPoint;
} GetSimpleDescriptor_t;

/*********************************************************************
 * VARIABLES
 */



/*********************************************************************
 * FUNCTIONS
 */



/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* ZCL_SAMPLELIGHT_H */
