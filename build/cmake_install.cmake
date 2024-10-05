# Install script for directory: /home/moh/PFE_Homorphic_encryption/EqualitySaturationHallideSyntax/CHEHAB

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
  set(CMAKE_INSTALL_SO_NO_EXE "1")
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

# Set default install directory permissions.
if(NOT DEFINED CMAKE_OBJDUMP)
  set(CMAKE_OBJDUMP "/usr/bin/objdump")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/usr/local/lib/libfheco.a")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  file(INSTALL DESTINATION "/usr/local/lib" TYPE STATIC_LIBRARY FILES "/home/moh/PFE_Homorphic_encryption/EqualitySaturationHallideSyntax/CHEHAB/build/libfheco.a")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  include("/home/moh/PFE_Homorphic_encryption/EqualitySaturationHallideSyntax/CHEHAB/build/src/fheco/cmake_install.cmake")
  include("/home/moh/PFE_Homorphic_encryption/EqualitySaturationHallideSyntax/CHEHAB/build/benchmarks/box_blur/cmake_install.cmake")
  include("/home/moh/PFE_Homorphic_encryption/EqualitySaturationHallideSyntax/CHEHAB/build/benchmarks/dot_product/cmake_install.cmake")
  include("/home/moh/PFE_Homorphic_encryption/EqualitySaturationHallideSyntax/CHEHAB/build/benchmarks/gx_kernel/cmake_install.cmake")
  include("/home/moh/PFE_Homorphic_encryption/EqualitySaturationHallideSyntax/CHEHAB/build/benchmarks/gy_kernel/cmake_install.cmake")
  include("/home/moh/PFE_Homorphic_encryption/EqualitySaturationHallideSyntax/CHEHAB/build/benchmarks/hamming_dist/cmake_install.cmake")
  include("/home/moh/PFE_Homorphic_encryption/EqualitySaturationHallideSyntax/CHEHAB/build/benchmarks/l2_distance/cmake_install.cmake")
  include("/home/moh/PFE_Homorphic_encryption/EqualitySaturationHallideSyntax/CHEHAB/build/benchmarks/linear_reg/cmake_install.cmake")
  include("/home/moh/PFE_Homorphic_encryption/EqualitySaturationHallideSyntax/CHEHAB/build/benchmarks/matrix_mul/cmake_install.cmake")
  include("/home/moh/PFE_Homorphic_encryption/EqualitySaturationHallideSyntax/CHEHAB/build/benchmarks/poly_reg/cmake_install.cmake")
  include("/home/moh/PFE_Homorphic_encryption/EqualitySaturationHallideSyntax/CHEHAB/build/benchmarks/roberts_cross/cmake_install.cmake")
  include("/home/moh/PFE_Homorphic_encryption/EqualitySaturationHallideSyntax/CHEHAB/build/benchmarks/polynomials_coyote/cmake_install.cmake")
  include("/home/moh/PFE_Homorphic_encryption/EqualitySaturationHallideSyntax/CHEHAB/build/benchmarks/sobel/cmake_install.cmake")

endif()

if(CMAKE_INSTALL_COMPONENT)
  set(CMAKE_INSTALL_MANIFEST "install_manifest_${CMAKE_INSTALL_COMPONENT}.txt")
else()
  set(CMAKE_INSTALL_MANIFEST "install_manifest.txt")
endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
file(WRITE "/home/moh/PFE_Homorphic_encryption/EqualitySaturationHallideSyntax/CHEHAB/build/${CMAKE_INSTALL_MANIFEST}"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
