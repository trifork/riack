
#ifndef RIAK_MSG_CODES_H_
#define RIAK_MSG_CODES_H_

#define mc_RpbErrorResp 0
#define mc_RpbPingReq 1
#define mc_RpbPingResp 2
#define mc_RpbGetClientIdReq 3
#define mc_RpbGetClientIdResp 4
#define mc_RpbSetClientIdReq 5
#define mc_RpbSetClientIdResp 6
#define mc_RpbGetServerInfoReq 7
#define mc_RpbGetServerInfoResp 8
#define mc_RpbGetReq 9
#define mc_RpbGetResp 10
#define mc_RpbPutReq 11
#define mc_RpbPutResp 12
#define mc_RpbDelReq 13
#define mc_RpbDelResp 14
#define mc_RpbListBucketsReq 15
#define mc_RpbListBucketsResp 16
#define mc_RpbListKeysReq 17
#define mc_RpbListKeysResp 18
#define mc_RpbGetBucketReq 19
#define mc_RpbGetBucketResp 20
#define mc_RpbSetBucketReq 21
#define mc_RpbSetBucketResp 22
#define mc_RpbMapRedReq 23
#define mc_RpbMapRedResp 24
#define mc_RpbIndexReq 25
#define mc_RpbIndexResp 26
#define mc_RpbSearchQueryReq 27
#define mc_RbpSearchQueryResp 28

int riak_get_msg_description(unsigned char msg_code, char* outbuff, int maxlen);

#endif /* RIAK_MSG_CODES_H_ */
