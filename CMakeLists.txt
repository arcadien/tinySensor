
cmake_minimum_required(VERSION 3.10)
project(TinySensor VERSION 0.1.0)
set (CMAKE_CXX_STANDARD 11)


set(SOURCES )

if(WIN32 OR UNIX)
  message("-- Compiling for Windows target")
  list(APPEND SOURCES src/base.cpp)
  add_definitions("-DF_CPU=${MCU_SPEED}")

else()
  message("-- Compiling for AVR target")
  
  set(AVR_MCU attiny84a)
  set(MCU_SPEED 1000000UL)
  add_definitions("-DF_CPU=${MCU_SPEED}")
  add_definitions("-DARDUINO=101")

  # AVR_L_FUSE (NO DEFAULT)
  # AVR_H_FUSE (NO DEFAULT)
  # AVR_UPLOADTOOL (default: avrdude)
  # AVR_UPLOADTOOL_PORT (default: usb)
  # AVR_PROGRAMMER (default: avrispmkII)

  include_directories("${CMAKE_SOURCE_DIR}/include")
  include_directories("${CMAKE_SOURCE_DIR}/third_party/ds18b20/include")
  include_directories("${CMAKE_SOURCE_DIR}/third_party/SparkFun_BME280/src/")

  list(APPEND SOURCES

	src/main.cpp

    third_party/tiny-i2c/TinyI2CMaster.cpp
    third_party/tiny-i2c/TinyI2CMaster.h
	src/protocol/Oregon.cpp
	include/protocol/Oregon.h

	third_party/SparkFun_BME280/src/SparkFunBME280.cpp
	third_party/SparkFun_BME280/src/SparkFunBME280.h

	third_party/ds18b20/src/ds18b20.c
	third_party/ds18b20/src/onewire.c
	third_party/ds18b20/src/romsearch.c

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

include_directories("${CMAKE_SOURCE_DIR}/")
include_directories("${CMAKE_SOURCE_DIR}/include/")
include_directories("${CMAKE_SOURCE_DIR}/third_party/unity/src")
include_directories("${CMAKE_SOURCE_DIR}/third_party/tiny-i2c")

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
	endif()
	add_subdirectory("third_party/unity")
	add_library(tinySensorLib OBJECT ${SOURCES})
endif()

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
