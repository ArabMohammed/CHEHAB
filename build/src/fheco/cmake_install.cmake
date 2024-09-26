# Install script for directory: /home/ma9232/CHEHAB/src/fheco

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/usr/local")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Install shared libraries without execute permission?
if(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  set(CMAKE_INSTALL_SO_NO_EXE "0")
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

# Set path to fallback-tool for dependency-resolution.
if(NOT DEFINED CMAKE_OBJDUMP)
  set(CMAKE_OBJDUMP "/usr/bin/objdump")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/FHECO/fheco" TYPE FILE FILES "/home/ma9232/CHEHAB/src/fheco/fheco.hpp")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  include("/home/ma9232/CHEHAB/build/src/fheco/code_gen/cmake_install.cmake")
  include("/home/ma9232/CHEHAB/build/src/fheco/dsl/cmake_install.cmake")
  include("/home/ma9232/CHEHAB/build/src/fheco/ir/cmake_install.cmake")
  include("/home/ma9232/CHEHAB/build/src/fheco/param_select/cmake_install.cmake")
  include("/home/ma9232/CHEHAB/build/src/fheco/passes/cmake_install.cmake")
  include("/home/ma9232/CHEHAB/build/src/fheco/trs/cmake_install.cmake")
  include("/home/ma9232/CHEHAB/build/src/fheco/trs/reduct_order/cmake_install.cmake")
  include("/home/ma9232/CHEHAB/build/src/fheco/util/cmake_install.cmake")

endif()

