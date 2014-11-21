cmake_minimum_required (VERSION 2.6)
project (Riack)

SET(Riack_VERSION_MAJOR 1)
SET(Riack_VERSION_MINOR 1)

SET(RIAK_TEST_SERVER_IP 127.0.0.1)
SET(RIAK_TEST_SERVER_PB_PORT 8087)

OPTION (BUILD_SHARED_LIBS "Build Shared Libraries" OFF)
SET(CMAKE_BUILD_TYPE DEBUG)

INCLUDE (CheckIncludeFiles)
CHECK_INCLUDE_FILES ("stdint.h" HAVE_STDINT_H)

SET (LIB_TYPE STATIC)

IF (BUILD_SHARED_LIBS)
  SET (LIB_TYPE SHARED)
  SET (RIACK_SHARED 1)
ENDIF (BUILD_SHARED_LIBS)

CONFIGURE_FILE (
  "${PROJECT_SOURCE_DIR}/riack-config.h.in"
  "${PROJECT_SOURCE_DIR}/riack-config.h"
  )

ENABLE_TESTING()

SET(RIACK_SRC	riack_commands.c
                riack_sock.c
				riack_sock.h
				riack.c
				riack_kv.c
				riack_2i.c
				riack_crdt.c
				riack_mapreduce.c
				riack_search.c
				riack.h
				riack_defines.h
				riack_compat.h
				riack_msg.c
				riack_msg.h
				riack_mem.c
				riack_helpers.h
				riack_helpers.c
				riack_internal.h
				ints.h
				protocol/riak_msg_codes.h
				protocol/riak_msg_codes.c
				protocol/riak_search.pb-c.h
				protocol/riak_search.pb-c.c
				protocol/riak.pb-c.h
				protocol/riak.pb-c.c
				protocol/riak_kv.pb-c.h
				protocol/riak_kv.pb-c.c
				protocol/riak_dt.pb-c.h
				protocol/riak_dt.pb-c.c
				protocol/riak_yokozuna.pb-c.h
				protocol/riak_yokozuna.pb-c.c
				protobuf-c/protobuf-c.c
				protobuf-c/protobuf-c.h)

SET(TESTS_MAIN_SRC	testsrc/test.h
				 	testsrc/test.c 
				 	testsrc/test_2i.c 
					testsrc/test_mem.c 
					testsrc/test_get_put.c
					testsrc/test_ping.c
					testsrc/test_bucket.c
					testsrc/test_mapred.c
					testsrc/test_delete.c
					testsrc/test_meta_links.c
					testsrc/test_misc.c
					testsrc/test_load.c
					testsrc/test_search.c
					testsrc/test_ext_props.c
					testsrc/test_crdt.c
					testsrc/test_clientid.c)

add_library (riack ${LIB_TYPE} ${RIACK_SRC})
target_link_libraries(riack ${LIBS})

INSTALL(TARGETS riack
        DESTINATION lib
        PERMISSIONS OWNER_READ GROUP_READ WORLD_READ)
        
INSTALL(FILES riack.h riack_defines.h
        DESTINATION include
        PERMISSIONS OWNER_READ GROUP_READ WORLD_READ)

include_directories("${PROJECT_SOURCE_DIR}")
include_directories("${PROJECT_BINARY_DIR}")

add_executable(tests ${TESTS_MAIN_SRC})
target_link_libraries(tests ${LIBS} riack)

IF (WIN32)
ELSE()
   SET_TARGET_PROPERTIES(riack PROPERTIES COMPILE_FLAGS "-std=c99 -fPIC")
ENDIF (WIN32)


ADD_TEST(test-load        ${CMAKE_CURRENT_BINARY_DIR}/tests load init ${RIAK_TEST_SERVER_IP} ${RIAK_TEST_SERVER_PB_PORT})
ADD_TEST(test-ping        ${CMAKE_CURRENT_BINARY_DIR}/tests ping basic ${RIAK_TEST_SERVER_IP} ${RIAK_TEST_SERVER_PB_PORT})
ADD_TEST(test-connect_with_opts  ${CMAKE_CURRENT_BINARY_DIR}/tests misc options ${RIAK_TEST_SERVER_IP} ${RIAK_TEST_SERVER_PB_PORT})
ADD_TEST(test-reconnect   ${CMAKE_CURRENT_BINARY_DIR}/tests misc reconnect ${RIAK_TEST_SERVER_IP} ${RIAK_TEST_SERVER_PB_PORT})
ADD_TEST(test-serverinfo  ${CMAKE_CURRENT_BINARY_DIR}/tests misc serverinfo ${RIAK_TEST_SERVER_IP} ${RIAK_TEST_SERVER_PB_PORT})
ADD_TEST(test-last-error  ${CMAKE_CURRENT_BINARY_DIR}/tests misc error ${RIAK_TEST_SERVER_IP} ${RIAK_TEST_SERVER_PB_PORT})
ADD_TEST(test-largeobject ${CMAKE_CURRENT_BINARY_DIR}/tests misc large ${RIAK_TEST_SERVER_IP} ${RIAK_TEST_SERVER_PB_PORT})
ADD_TEST(test-clientid 	  ${CMAKE_CURRENT_BINARY_DIR}/tests clientid basic ${RIAK_TEST_SERVER_IP} ${RIAK_TEST_SERVER_PB_PORT})
ADD_TEST(test-props-bucket ${CMAKE_CURRENT_BINARY_DIR}/tests ext_props bucket ${RIAK_TEST_SERVER_IP} ${RIAK_TEST_SERVER_PB_PORT})
ADD_TEST(test-props-type  ${CMAKE_CURRENT_BINARY_DIR}/tests ext_props type ${RIAK_TEST_SERVER_IP} ${RIAK_TEST_SERVER_PB_PORT})
ADD_TEST(test-bucket-list ${CMAKE_CURRENT_BINARY_DIR}/tests bucket list ${RIAK_TEST_SERVER_IP} ${RIAK_TEST_SERVER_PB_PORT})
ADD_TEST(test-delete      ${CMAKE_CURRENT_BINARY_DIR}/tests delete basic ${RIAK_TEST_SERVER_IP} ${RIAK_TEST_SERVER_PB_PORT})
ADD_TEST(test-put1		  ${CMAKE_CURRENT_BINARY_DIR}/tests get_put put1 ${RIAK_TEST_SERVER_IP} ${RIAK_TEST_SERVER_PB_PORT})
ADD_TEST(test-put2 		  ${CMAKE_CURRENT_BINARY_DIR}/tests get_put put2 ${RIAK_TEST_SERVER_IP} ${RIAK_TEST_SERVER_PB_PORT})
ADD_TEST(test-put3 		  ${CMAKE_CURRENT_BINARY_DIR}/tests get_put put3 ${RIAK_TEST_SERVER_IP} ${RIAK_TEST_SERVER_PB_PORT})
ADD_TEST(test-get1 		  ${CMAKE_CURRENT_BINARY_DIR}/tests get_put get1 ${RIAK_TEST_SERVER_IP} ${RIAK_TEST_SERVER_PB_PORT})
ADD_TEST(test-mapred1 	  ${CMAKE_CURRENT_BINARY_DIR}/tests mapred basic ${RIAK_TEST_SERVER_IP} ${RIAK_TEST_SERVER_PB_PORT})
ADD_TEST(test-search1 	  ${CMAKE_CURRENT_BINARY_DIR}/tests search search1 ${RIAK_TEST_SERVER_IP} ${RIAK_TEST_SERVER_PB_PORT})
ADD_TEST(test-2i-load	  ${CMAKE_CURRENT_BINARY_DIR}/tests 2i load ${RIAK_TEST_SERVER_IP} ${RIAK_TEST_SERVER_PB_PORT})
ADD_TEST(test-2i-exact    ${CMAKE_CURRENT_BINARY_DIR}/tests 2i exact ${RIAK_TEST_SERVER_IP} ${RIAK_TEST_SERVER_PB_PORT})
ADD_TEST(test-2i-range    ${CMAKE_CURRENT_BINARY_DIR}/tests 2i range ${RIAK_TEST_SERVER_IP} ${RIAK_TEST_SERVER_PB_PORT})
ADD_TEST(test-2i-pagination ${CMAKE_CURRENT_BINARY_DIR}/tests 2i pagination ${RIAK_TEST_SERVER_IP} ${RIAK_TEST_SERVER_PB_PORT})
ADD_TEST(test-2i-pagination-stream ${CMAKE_CURRENT_BINARY_DIR}/tests 2i stream_pag ${RIAK_TEST_SERVER_IP} ${RIAK_TEST_SERVER_PB_PORT})
ADD_TEST(test-2i-cleanup  ${CMAKE_CURRENT_BINARY_DIR}/tests 2i cleanup ${RIAK_TEST_SERVER_IP} ${RIAK_TEST_SERVER_PB_PORT})
ADD_TEST(test-crdt-counter ${CMAKE_CURRENT_BINARY_DIR}/tests crdt counter ${RIAK_TEST_SERVER_IP} ${RIAK_TEST_SERVER_PB_PORT})
ADD_TEST(test-ml-load	  ${CMAKE_CURRENT_BINARY_DIR}/tests meta-links load ${RIAK_TEST_SERVER_IP} ${RIAK_TEST_SERVER_PB_PORT})
ADD_TEST(test-ml-links    ${CMAKE_CURRENT_BINARY_DIR}/tests meta-links links ${RIAK_TEST_SERVER_IP} ${RIAK_TEST_SERVER_PB_PORT})
ADD_TEST(test-ml-metas    ${CMAKE_CURRENT_BINARY_DIR}/tests meta-links metas ${RIAK_TEST_SERVER_IP} ${RIAK_TEST_SERVER_PB_PORT})
ADD_TEST(test-cleanup     ${CMAKE_CURRENT_BINARY_DIR}/tests load cleanup ${RIAK_TEST_SERVER_IP} ${RIAK_TEST_SERVER_PB_PORT})
