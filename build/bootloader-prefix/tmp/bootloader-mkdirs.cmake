# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/home/lukas/esp/esp-idf/components/bootloader/subproject"
  "/home/lukas/esp/PIR/02/Template_modified/build/bootloader"
  "/home/lukas/esp/PIR/02/Template_modified/build/bootloader-prefix"
  "/home/lukas/esp/PIR/02/Template_modified/build/bootloader-prefix/tmp"
  "/home/lukas/esp/PIR/02/Template_modified/build/bootloader-prefix/src/bootloader-stamp"
  "/home/lukas/esp/PIR/02/Template_modified/build/bootloader-prefix/src"
  "/home/lukas/esp/PIR/02/Template_modified/build/bootloader-prefix/src/bootloader-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/home/lukas/esp/PIR/02/Template_modified/build/bootloader-prefix/src/bootloader-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/home/lukas/esp/PIR/02/Template_modified/build/bootloader-prefix/src/bootloader-stamp${cfgdir}") # cfgdir has leading slash
endif()
