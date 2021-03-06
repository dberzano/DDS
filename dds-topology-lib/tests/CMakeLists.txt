# Copyright 2014 GSI, Inc. All rights reserved.
#
#
#
project( dds_topology_lib-tests )

include_directories(${dds_topology_lib_SOURCE_DIR}/src ${Boost_INCLUDE_DIRS})
#=============================================================================


add_executable(dds_topology_lib-tests Test.cpp )
add_executable(dds_topology_lib-performance-tests TestPerformance.cpp )

target_link_libraries (
	dds_topology_lib-tests
    dds_topology_lib
	dds_user_defaults_lib
	${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
	${Boost_FILESYSTEM_LIBRARY}
)

target_link_libraries (
    dds_topology_lib-performance-tests
    dds_topology_lib
    dds_user_defaults_lib
    ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
    ${Boost_FILESYSTEM_LIBRARY}
)

install(TARGETS dds_topology_lib-tests DESTINATION tests)
install(TARGETS dds_topology_lib-performance-tests DESTINATION tests)

target_include_directories(dds_topology_lib-tests
	PRIVATE
		$<TARGET_PROPERTY:dds_user_defaults_lib,INTERFACE_INCLUDE_DIRECTORIES>
)

##################################################################
# task-test_index.cpp
##################################################################

set( TEST_INDEX_SOURCE_FILES
	./task-test_index.cpp
)

add_executable(task-test_index ${TEST_INDEX_SOURCE_FILES})

target_link_libraries (
    task-test_index
    dds_intercom_lib
)

target_include_directories(task-test_index
PRIVATE
     $<TARGET_PROPERTY:dds_intercom_lib,INTERFACE_INCLUDE_DIRECTORIES>
)

install(TARGETS task-test_index DESTINATION tests)


if( BUILD_TESTS )
  install(FILES ${dds_topology_lib-tests_SOURCE_DIR}/topology_test_1.xml DESTINATION tests)
  install(FILES ${dds_topology_lib-tests_SOURCE_DIR}/topology_test_2.xml DESTINATION tests)
  install(FILES ${dds_topology_lib-tests_SOURCE_DIR}/topology_test_3.xml DESTINATION tests)
  install(FILES ${dds_topology_lib-tests_SOURCE_DIR}/topology_test_4.xml DESTINATION tests)
  install(FILES ${dds_topology_lib-tests_SOURCE_DIR}/topology_test_5.xml DESTINATION tests)
  install(FILES ${dds_topology_lib-tests_SOURCE_DIR}/topology_test_6.xml DESTINATION tests)
  install(FILES ${dds_topology_lib-tests_SOURCE_DIR}/topology_test_7.xml DESTINATION tests)
  install(FILES ${dds_topology_lib-tests_SOURCE_DIR}/topology_test_8.xml DESTINATION tests)
	install(FILES ${dds_topology_lib-tests_SOURCE_DIR}/topology_test_diff_1.xml DESTINATION tests)
	install(FILES ${dds_topology_lib-tests_SOURCE_DIR}/topology_test_diff_2.xml DESTINATION tests)
  install(FILES ${dds_topology_lib-tests_SOURCE_DIR}/topology_test_1_iterators_1.txt DESTINATION tests)
  install(FILES ${dds_topology_lib-tests_SOURCE_DIR}/topology_test_1_iterators_2.txt DESTINATION tests)
  install(FILES ${dds_topology_lib-tests_SOURCE_DIR}/topology_test_1_iterators_3.txt DESTINATION tests)
  install(FILES ${dds_topology_lib-tests_SOURCE_DIR}/topology_test_1_iterators_4.txt DESTINATION tests)
  install(FILES ${dds_topology_lib-tests_SOURCE_DIR}/topology_test_1_iterators_5.txt DESTINATION tests)
  install(FILES ${dds_topology_lib-tests_SOURCE_DIR}/topology_test_1_maps_1.txt DESTINATION tests)
  install(FILES ${dds_topology_lib-tests_SOURCE_DIR}/topology_test_1_maps_2.txt DESTINATION tests)
  install(FILES ${dds_topology_lib-tests_SOURCE_DIR}/topology_test_1_maps_3.txt DESTINATION tests)
  install(FILES ${dds_topology_lib-tests_SOURCE_DIR}/topology_test_1_maps_4.txt DESTINATION tests)
  install(FILES ${dds_topology_lib-tests_SOURCE_DIR}/topology_test_1_maps_5.txt DESTINATION tests)
  install(FILES ${dds_topology_lib-tests_SOURCE_DIR}/topology_test_7_maps_1.txt DESTINATION tests)
  install(FILES ${dds_topology_lib-tests_SOURCE_DIR}/topology_test_7_maps_2.txt DESTINATION tests)
  install(FILES ${dds_topology_lib-tests_SOURCE_DIR}/topology_test_7_maps_3.txt DESTINATION tests)
  install(FILES ${dds_topology_lib-tests_SOURCE_DIR}/topology_test_7_maps_4.txt DESTINATION tests)
  install(FILES ${dds_topology_lib-tests_SOURCE_DIR}/topology_test_7_maps_5.txt DESTINATION tests)
	install(FILES ${dds_topology_lib-tests_SOURCE_DIR}/topology_test_diff.txt DESTINATION tests)
  install(FILES ${dds_topology_lib-tests_SOURCE_DIR}/index_test_hosts.cfg DESTINATION tests)
  install(FILES ${dds_topology_lib-tests_SOURCE_DIR}/index_test.xml DESTINATION tests)
  install(FILES ${dds_topology_lib-tests_SOURCE_DIR}/index_test_result.txt DESTINATION tests)
  install(PROGRAMS ${dds_topology_lib-tests_SOURCE_DIR}/run_test_index.sh DESTINATION tests)
endif( BUILD_TESTS )
