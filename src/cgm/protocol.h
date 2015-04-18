/*
 * protocol.h
 *
 *  Created on: Apr 6, 2015
 *      Author: stardica
 */


#ifndef PROTOCOL_H_
#define PROTOCOL_H_


#define MESI

enum cgm_access_kind_t {
	cgm_access_invalid = 0,
	cgm_access_fetch,
	cgm_access_load,
	cgm_access_store,
	cgm_access_nc_store,
	cgm_access_nc_load,
	cgm_access_prefetch,
	cgm_access_gets, //get shared
	cgm_access_gets_i, //get shared specific to i caches
	cgm_access_getx, //get exclusive (or get with intent to write)
	cgm_access_inv,  //invalidation request
	cgm_access_putx, //request for writeback of cache block exclusive data.
	cgm_access_puts, //request for writeback of cache block in shared state.
	cgm_access_puto, //request for writeback of cache block in owned state.
	cgm_access_puto_shared, //equest for writeback of cache block in owned state but other sharers of the block exist.
	cgm_access_unblock, //message to unblock next cache level/directory for blocking protocols.
	cgm_access_retry,
	cgm_access_retry_i
};




#endif /*PROTOCOL_H_*/
