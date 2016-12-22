
#include "riak_msg_codes.h"
#include <string.h>

const char* riak_start_tls = "Start tls";
const char* riak_req_auth = "Auth request";
const char* riak_rsp_auth = "Auth response";
const char* riak_rsp_err = "Error response";
const char* riak_req_ping = "Ping request";
const char* riak_rsp_ping = "Ping response";
const char* riak_req_get_client_id = "Get client id request";
const char* riak_rsp_get_client_id = "Get client id response";
const char* riak_req_set_client_id = "Set client id request";
const char* riak_rsp_set_client_id = "Set client id response";
const char* riak_req_get_server_info = "Get server info request";
const char* riak_rsp_get_server_info = "Get server info response";
const char* riak_req_get = "Get request";
const char* riak_rsp_get = "Get response";
const char* riak_req_put = "Put request";
const char* riak_rsp_put = "Put response";
const char* riak_req_del = "Delete request";
const char* riak_rsp_del = "Delete response";
const char* riak_req_list_buckets = "List buckets request";
const char* riak_rsp_list_buckets = "List buckets response";
const char* riak_req_list_keys = "List keys request";
const char* riak_rsp_list_keys = "List keys response";
const char* riak_req_get_bucket = "Get bucket request";
const char* riak_rsp_get_bucket = "Get bucket response";
const char* riak_req_set_bucket = "Set bucket request";
const char* riak_rsp_set_bucket = "Set bucket response";
const char* riak_req_map_reduce = "Map reduce request";
const char* riak_rsp_map_reduce = "Map reduce response";
const char* riak_req_index = "Index request";
const char* riak_rsp_index = "Index response";
const char* riak_req_search = "Search query request";
const char* riak_rsp_search = "Search query response";
const char* riak_unknown_cmk = "Unknown message code";

int riak_get_msg_description(unsigned char msg_code, char* outbuff, int maxlen)
{
	int desclen = 0;
	const char *msgstr = 0;
	switch (msg_code)
	{
	case mc_RpbErrorResp:
		msgstr = riak_rsp_err;
		break;
	case mc_RpbStartTls:
		msgstr = riak_start_tls;
		break;
	case mc_RpbPingReq:
		msgstr = riak_req_ping;
		break;
	case mc_RpbPingResp:
		msgstr = riak_rsp_ping;
		break;
	case mc_RpbAuthReq:
		msgstr = riak_req_auth;
		break;
	case mc_RpbAuthResp:
		msgstr = riak_rsp_auth;
		break;
	case mc_RpbGetClientIdReq:
		msgstr = riak_req_get_client_id;
		break;
	case mc_RpbGetClientIdResp:
		msgstr = riak_rsp_get_client_id;
		break;
	case mc_RpbSetClientIdReq:
		msgstr = riak_req_set_client_id;
		break;
	case mc_RpbSetClientIdResp:
		msgstr = riak_rsp_set_client_id;
		break;
	case mc_RpbGetServerInfoReq:
		msgstr = riak_req_get_server_info;
		break;
	case mc_RpbGetServerInfoResp:
		msgstr = riak_rsp_get_server_info;
		break;
	case mc_RpbGetReq:
		msgstr = riak_req_get;
		break;
	case mc_RpbGetResp:
		msgstr = riak_rsp_get;
		break;
	case mc_RpbPutReq:
		msgstr = riak_req_put;
		break;
	case mc_RpbPutResp:
		msgstr = riak_rsp_put;
		break;
	case mc_RpbDelReq:
		msgstr = riak_req_del;
		break;
	case mc_RpbDelResp:
		msgstr = riak_rsp_del;
		break;
	case mc_RpbListBucketsReq:
		msgstr = riak_req_list_buckets;
		break;
	case mc_RpbListBucketsResp:
		msgstr = riak_rsp_list_buckets;
		break;
	case mc_RpbListKeysReq:
		msgstr = riak_req_list_keys;
		break;
	case mc_RpbListKeysResp:
		msgstr = riak_rsp_list_keys;
		break;
	case mc_RpbGetBucketReq:
		msgstr = riak_req_get_bucket;
		break;
	case mc_RpbGetBucketResp:
		msgstr = riak_rsp_get_bucket;
		break;
	case mc_RpbSetBucketReq:
		msgstr = riak_req_set_bucket;
		break;
	case mc_RpbSetBucketResp:
		msgstr = riak_rsp_set_bucket;
		break;
	case mc_RpbMapRedReq:
		msgstr = riak_req_map_reduce;
		break;
	case mc_RpbMapRedResp:
		msgstr = riak_rsp_map_reduce;
		break;
	case mc_RpbIndexReq:
		msgstr = riak_req_index;
		break;
	case mc_RpbIndexResp:
		msgstr = riak_rsp_index;
		break;
	case mc_RpbSearchQueryReq:
		msgstr = riak_req_search;
		break;
	case mc_RbpSearchQueryResp:
		msgstr = riak_rsp_search;
		break;
	default:
		msgstr = riak_unknown_cmk;
		break;
	}
	// Min
	desclen = (int)strlen(msgstr) > maxlen-1 ? maxlen-1 : (int)strlen(msgstr);
	if (desclen > 0)
	{
		memcpy(outbuff, msgstr, desclen);
	}
	outbuff[desclen] = 0;
	return desclen;
}
