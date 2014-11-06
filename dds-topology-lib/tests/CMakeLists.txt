# Copyright 2014 GSI, Inc. All rights reserved.
#
#
#
project( dds-topology-lib-tests )

include_directories(${dds-topology-lib_SOURCE_DIR}/src ${Boost_INCLUDE_DIRS})
#=============================================================================


set( SOURCE_FILES
	Test.cpp
	${dds-topology-lib_SOURCE_DIR}/src/Index.cpp
	${dds-topology-lib_SOURCE_DIR}/src/TopoProperty.cpp
	${dds-topology-lib_SOURCE_DIR}/src/TopoBase.cpp
	${dds-topology-lib_SOURCE_DIR}/src/Topology.cpp
	${dds-topology-lib_SOURCE_DIR}/src/Task.cpp
	${dds-topology-lib_SOURCE_DIR}/src/TaskCollection.cpp
	${dds-topology-lib_SOURCE_DIR}/src/TaskGroup.cpp
	${dds-topology-lib_SOURCE_DIR}/src/TopoElement.cpp
	${dds-topology-lib_SOURCE_DIR}/src/TaskContainer.cpp
	${dds-topology-lib_SOURCE_DIR}/src/TopologyParserXML.cpp
	${dds-topology-lib_SOURCE_DIR}/src/TopoUtils.cpp
	${dds-topology-lib_SOURCE_DIR}/src/TopoFactory.cpp
)
add_executable(dds-topology-lib-tests ${SOURCE_FILES} )

target_link_libraries (
	dds-topology-lib-tests
	dds-user-defaults-lib
	${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
	${Boost_FILESYSTEM_LIBRARY}
)

install(TARGETS dds-topology-lib-tests DESTINATION tests)


target_include_directories(dds-topology-lib-tests
	PRIVATE
		$<TARGET_PROPERTY:dds-user-defaults-lib,INTERFACE_INCLUDE_DIRECTORIES>
)

# Copy schema file and test XML files to the build directory.
#add_custom_target(copy_to_build ALL
#            COMMAND ${CMAKE_COMMAND} -E copy ${TOPOLOGY_SCHEMA_PATH} "${dds-topology-lib-tests_BINARY_DIR}"
#            COMMAND ${CMAKE_COMMAND} -E copy ${dds-topology-lib-tests_SOURCE_DIR}/topology_test_1.xml "${dds-topology-lib-tests_BINARY_DIR}"
#            COMMAND ${CMAKE_COMMAND} -E copy ${dds-topology-lib-tests_SOURCE_DIR}/topology_test_2.xml "${dds-topology-lib-tests_BINARY_DIR}"
#            COMMAND ${CMAKE_COMMAND} -E copy ${dds-topology-lib-tests_SOURCE_DIR}/topology_test_3.xml "${dds-topology-lib-tests_BINARY_DIR}"
#            COMMAND ${CMAKE_COMMAND} -E copy ${dds-topology-lib-tests_SOURCE_DIR}/topology_test_4.xml "${dds-topology-lib-tests_BINARY_DIR}"
#            COMMAND ${CMAKE_COMMAND} -E copy ${dds-topology-lib-tests_SOURCE_DIR}/topology_test_5.xml "${dds-topology-lib-tests_BINARY_DIR}"
#			COMMAND ${CMAKE_COMMAND} -E copy ${dds-topology-lib-tests_SOURCE_DIR}/topology_test_6.xml "${dds-topology-lib-tests_BINARY_DIR}"
#            COMMENT "Copying XML Schema file and test topology XML to build directory."
#            VERBATIM)
            
            
if( BUILD_TESTS )
  install(PROGRAMS ${dds-topology-lib-tests_SOURCE_DIR}/topology_test_1.xml DESTINATION tests)
  install(PROGRAMS ${dds-topology-lib-tests_SOURCE_DIR}/topology_test_2.xml DESTINATION tests)
  install(PROGRAMS ${dds-topology-lib-tests_SOURCE_DIR}/topology_test_3.xml DESTINATION tests)
  install(PROGRAMS ${dds-topology-lib-tests_SOURCE_DIR}/topology_test_4.xml DESTINATION tests)
  install(PROGRAMS ${dds-topology-lib-tests_SOURCE_DIR}/topology_test_5.xml DESTINATION tests)
  install(PROGRAMS ${dds-topology-lib-tests_SOURCE_DIR}/topology_test_6.xml DESTINATION tests)
endif( BUILD_TESTS )
