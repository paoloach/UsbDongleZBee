/**************************************************************************************************

 DESCRIPTION:
  --

 CREATED: 27/02/2015, by Paolo Achdjian

 FILE: bindTable2.h

***************************************************************************************************/
#ifndef BINDINGTABLE_ACHDJIAN_H
#define BINDINGTABLE_ACHDJIAN_H

#include "ZComDef.h"

#ifdef __cplusplus
extern "C" {
#endif
	


struct BindingEntry {
	uint8 srcEP;
	uint16 srcShortAddress;
	uint16 cluster;
	uint8 dstEP;
	uint16 dstShortAddress;
};

	
extern void InitBindingTable( void );

extern byte bind(struct BindingEntry * bindEntry );


#ifdef __cplusplus
}
#endif

#endif