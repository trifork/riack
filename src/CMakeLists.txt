cmake_minimum_required (VERSION 2.6)
project (Riack)

SET(RIAK_TEST_SERVER_IP fmk-riak02.trifork.com)
SET(RIAK_TEST_SERVER_PB_PORT 8087)
OPTION (BUILD_SHARED_LIBS "Build Shared Libraries" ON)

ENABLE_TESTING()

SET(CMAKE_BUILD_TYPE DEBUG)
SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake/Modules/")

find_package(ProtoBufC REQUIRED)
include_directories(${PROTOBUFC_INCLUDE_DIRS})
set(LIBS ${LIBS} ${PROTOBUFC_LIBRARIES})

SET(RIACK_SRC	riack_sock.c
				riack_sock.h
				riack.c
				riack.h
				riack_defines.h
				riack_msg.c
				riack_msg.h
				riack_mem.c
				riack_helpers.h
				riack_helpers.c
				protocol/riak_msg_codes.h
				protocol/riak_msg_codes.c
				protocol/riak_search.pb-c.h
				protocol/riak_search.pb-c.c
				protocol/riak.pb-c.h
				protocol/riak.pb-c.c
				protocol/riak_kv.pb-c.h
				protocol/riak_kv.pb-c.c)

SET(TESTS_MAIN_SRC	tests/test.h
				 	tests/test.c 
					tests/test_mem.c 
					tests/test_get_put.c
					tests/test_ping.c
					tests/test_bucket.c
					tests/test_mapred.c
					tests/test_delete.c
					tests/test_misc.c
					tests/test_clientid.c)

SET (LIB_TYPE STATIC)
IF (BUILD_SHARED_LIBS)
  SET (LIB_TYPE SHARED)
ENDIF (BUILD_SHARED_LIBS)

add_library (riack ${LIB_TYPE} ${RIACK_SRC})
target_link_libraries(riack ${LIBS})

INSTALL(TARGETS riack
        DESTINATION lib
        PERMISSIONS OWNER_READ GROUP_READ WORLD_READ)
        
INSTALL(FILES riack.h riack_defines.h
        DESTINATION include
        PERMISSIONS OWNER_READ GROUP_READ WORLD_READ)

add_executable(tests ${TESTS_MAIN_SRC})
target_link_libraries(tests ${LIBS} riack)

IF (WIN32)
ELSE()
   SET_TARGET_PROPERTIES(riack PROPERTIES COMPILE_FLAGS "-std=c99 -fPIC")
ENDIF (WIN32)


ADD_TEST(test-ping        ${CMAKE_CURRENT_BINARY_DIR}/tests ping basic ${RIAK_TEST_SERVER_IP} ${RIAK_TEST_SERVER_PB_PORT})
ADD_TEST(test-serverinfo  ${CMAKE_CURRENT_BINARY_DIR}/tests misc serverinfo ${RIAK_TEST_SERVER_IP} ${RIAK_TEST_SERVER_PB_PORT})
ADD_TEST(test-clientid 	  ${CMAKE_CURRENT_BINARY_DIR}/tests clientid basic ${RIAK_TEST_SERVER_IP} ${RIAK_TEST_SERVER_PB_PORT})
ADD_TEST(test-bucket-list ${CMAKE_CURRENT_BINARY_DIR}/tests bucket list ${RIAK_TEST_SERVER_IP} ${RIAK_TEST_SERVER_PB_PORT})
ADD_TEST(test-delete      ${CMAKE_CURRENT_BINARY_DIR}/tests delete basic ${RIAK_TEST_SERVER_IP} ${RIAK_TEST_SERVER_PB_PORT})
ADD_TEST(test-put1		  ${CMAKE_CURRENT_BINARY_DIR}/tests get_put put1 ${RIAK_TEST_SERVER_IP} ${RIAK_TEST_SERVER_PB_PORT})
ADD_TEST(test-put2 		  ${CMAKE_CURRENT_BINARY_DIR}/tests get_put put2 ${RIAK_TEST_SERVER_IP} ${RIAK_TEST_SERVER_PB_PORT})
ADD_TEST(test-get1 		  ${CMAKE_CURRENT_BINARY_DIR}/tests get_put get1 ${RIAK_TEST_SERVER_IP} ${RIAK_TEST_SERVER_PB_PORT})
ADD_TEST(test-mapred1 	  ${CMAKE_CURRENT_BINARY_DIR}/tests mapred basic ${RIAK_TEST_SERVER_IP} ${RIAK_TEST_SERVER_PB_PORT})

