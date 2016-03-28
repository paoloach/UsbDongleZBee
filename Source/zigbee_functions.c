
#include "AF.h"
#include "ZDProfile.h"
#include "zigbee_functions.h"
#include "ZigBeeUsbBridge.h"

// This list should be filled with Application specific Cluster IDs.
static const cId_t GenericApp_ClusterList[GENERICAPP_MAX_CLUSTERS] =
{
  GENERICAPP_CLUSTERID
};

static  zAddrType_t dstAddr;
/**
* send a request to all the devices in order to force a send of annunce message
*/
void requestDevices(void) {
	  dstAddr.addrMode = AddrBroadcast;
      dstAddr.addr.shortAddr = NWK_BROADCAST_SHORTADDR;
      ZDP_MatchDescReq( &dstAddr, NWK_BROADCAST_SHORTADDR,
                        GENERICAPP_PROFID,
                        GENERICAPP_MAX_CLUSTERS, (cId_t *)GenericApp_ClusterList,
                        GENERICAPP_MAX_CLUSTERS, (cId_t *)GenericApp_ClusterList,
                        FALSE );
}