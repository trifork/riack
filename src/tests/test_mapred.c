
#include "test.h"

char* testdata_1 = "Alice was beginning to get very tired of sitting by her sister on the \
bank, and of having nothing to do: once or twice she had peeped into the \
book her sister was reading, but it had no pictures or conversations in \
it, 'and what is the use of a book,' thought Alice 'without pictures or \
conversation?";

char* testdata_2 = "So she was considering in her own mind (as well as she could, for the \
hot day made her feel very sleepy and stupid), whether the pleasure \
of making a daisy-chain would be worth the trouble of getting up and \
picking the daisies, when suddenly a White Rabbit with pink eyes ran \
close by her.";

char* testdata_3 = "The rabbit-hole went straight on like a tunnel for some way, and then \
dipped suddenly down, so suddenly that Alice had not a moment to think \
about stopping herself before she found herself falling down a very deep\
well.";
// http://wiki.basho.com/MapReduce.html

const char* mapred_test_query = "{\"inputs\":[[\"%s\",\"%s\"],[\"%s\",\"%s\"],[\"%s\",\"%s\"]]\
,\"query\":[{\"map\":{\"language\":\"javascript\",\"source\":\"\
function(v) {\
  var m = v.values[0].data.toLowerCase().match(/\\w*/g);\
  var r = [];\
  for(var i in m) {\
    if(m[i] != '') {\
      var o = {};\
      o[m[i]] = 1;\
      r.push(o);\
    }\
  }\
  return r;\
}\
\"}},{\"reduce\":{\"language\":\"javascript\",\"source\":\"\
function(v) {\
  var r = {};\
  for(var i in v) {\
    for(var w in v[i]) {\
      if(w in r) r[w] += v[i][w];\
      else r[w] = v[i][w];\
    }\
  }\
  return [r];\
}\
\"}}]}";

int test_mapred(char* testcase)
{
	if (strcmp(testcase, "basic") == 0) {
		return test_mapred_basic();
	}
	return -1;
}

int test_mapred_basic() {
	int result;
	char buffer[2000];
	struct RIACK_MAPRED_RESULT *mapresult;
	result = 1;
	if (put("test_mapred1", testdata_1) == RIACK_SUCCESS &&
		put("test_mapred2", testdata_2) == RIACK_SUCCESS &&
		put("test_mapred3", testdata_3) == RIACK_SUCCESS) {
		sprintf(buffer, mapred_test_query, RIAK_TEST_BUCKET, "test_mapred1",
				RIAK_TEST_BUCKET, "test_mapred2",
				RIAK_TEST_BUCKET, "test_mapred3");
		if (riack_map_reduce(test_client, strlen(buffer), buffer, APPLICATION_JSON, &mapresult) == RIACK_SUCCESS) {
			// TODO verufy the actual results instead of printing them

			riack_free_mapred_result(test_client, mapresult);
			result = 0;
		}
		delete("test_mapred1");
		delete("test_mapred2");
		delete("test_mapred3");
	}
	return result;
}

