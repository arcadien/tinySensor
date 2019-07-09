
cmake_minimum_required(VERSION 3.12)
project(TinySensor VERSION 0.1.0)


add_definitions("-DF_CPU=${MCU_SPEED}")

set(SOURCES )

if(WIN32 OR UNIX)
  message("-- Compiling for Windows target")
  list(APPEND SOURCES src/base.cpp)

else()
  message("-- Compiling for AVR target")
  list(APPEND SOURCES
	src/main.cpp
	protocol/Oregon.cpp
	sensors/bme280/SparkFunBME280.cpp
	sensors/bme280/SparkFunBME280.h
	busses/wire/USIWire.cpp
	protocol/Oregon.h
	busses/wire/USI_TWI_Master/USI_TWI_Master.c
	busses/wire/USI_TWI_Slave/USI_TWI_Slave.c
  ) 
endif()

##########################################################################
# set compiler options for build types
##########################################################################
if(CMAKE_BUILD_TYPE MATCHES Release)
   set(CMAKE_C_FLAGS_RELEASE "-Os")
   set(CMAKE_CXX_FLAGS_RELEASE "-Os")
endif(CMAKE_BUILD_TYPE MATCHES Release)

if(CMAKE_BUILD_TYPE MATCHES RelWithDebInfo)
   set(CMAKE_C_FLAGS_RELWITHDEBINFO "-Os -save-temps -g -gdwarf-3 -gstrict-dwarf")
   set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-Os -save-temps -g -gdwarf-3 -gstrict-dwarf")
endif(CMAKE_BUILD_TYPE MATCHES RelWithDebInfo)

if(CMAKE_BUILD_TYPE MATCHES Debug)
   set(CMAKE_C_FLAGS_DEBUG "-O0 -save-temps -g -gdwarf-3 -gstrict-dwarf")
   set(CMAKE_CXX_FLAGS_DEBUG "-O0 -save-temps -g -gdwarf-3 -gstrict-dwarf")
endif(CMAKE_BUILD_TYPE MATCHES Debug)

if(AVR)
	##########################################################################
	# compiler options for all build types
	##########################################################################
	add_definitions("-DF_CPU=${MCU_SPEED}")
	add_definitions("-fpack-struct")
	add_definitions("-fshort-enums")
	add_definitions("-Wall")
	#add_definitions("-Werror")
	#add_definitions("-pedantic")
	#add_definitions("-pedantic-errors")
	add_definitions("-funsigned-char")
	add_definitions("-funsigned-bitfields")
	add_definitions("-ffunction-sections")

	##########################################################################
	# status messages
	##########################################################################
	message(STATUS "Current uploadtool is: ${AVR_UPLOADTOOL}")
	message(STATUS "Current programmer is: ${AVR_PROGRAMMER}")
	message(STATUS "Current upload port is: ${AVR_UPLOADTOOL_PORT}")
	message(STATUS "Current uploadtool options are: ${AVR_UPLOADTOOL_OPTIONS}")
	message(STATUS "Current MCU is set to: ${AVR_MCU}")
	message(STATUS "Current MCU speed (Hz) is set to: ${MCU_SPEED}")
	message(STATUS "Current H_FUSE is set to: ${AVR_H_FUSE}")
	message(STATUS "Current L_FUSE is set to: ${AVR_L_FUSE}")

	add_avr_executable(tinySensor ${SOURCES} src/mainWrapper.cpp)
	add_avr_library(unity third_party/unity/src/unity.c)
	set_target_properties(tinySensor PROPERTIES LINK_FLAGS "--wrap main" )
else()
	if(WITH_COVERAGE)
		set(CMAKE_CXX_FLAGS "-g -O0 -Wall -fprofile-arcs -ftest-coverage")
		set(CMAKE_C_FLAGS "-g -O0 -Wall -fprofile-arcs -ftest-coverage")
	endif
	add_subdirectory("third_party/unity")
	add_library(tinySensorLib OBJECT ${SOURCES})
endif()


include_directories("${CMAKE_SOURCE_DIR}/")
include_directories("${CMAKE_SOURCE_DIR}/include/")
include_directories("third_party/unity/src")

include(CTest)
enable_testing()

# Scenario Runner
SET(TESTS
 firstTest.cpp
)
foreach(test_ ${TESTS})

	if(AVR)
		add_avr_executable(${test_}_driver tests/${test_} ${SOURCES})
		avr_target_link_libraries(${test_}_driver unity)
	else()
		add_executable(${test_}_driver tests/${test_})
		target_link_libraries(${test_}_driver tinySensorLib)
		target_link_libraries(${test_}_driver unity)
	endif()
	add_test(${test_}_driver ${test_}_driver)
endforeach()
