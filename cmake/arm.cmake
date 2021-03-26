# This one is important
SET(CMAKE_SYSTEM_NAME Linux)
SET(CMAKE_SYSTEM_PROCESSOR "aarch64")

if("${CROSS}" STREQUAL "")
  # Default the cross compiler prefix to something known to work.
  set(CROSS aarch64-linux-gnu-)
endif()

# Specify the cross compiler
SET(CMAKE_C_COMPILER   /usr/bin/aarch64-linux-gnu-gcc-9)
SET(CMAKE_CXX_COMPILER /usr/bin/aarch64-linux-gnu-g++-9)

SET(CMAKE_CROSS_COMPILING ON)
SET(CMAKE_CROSSCOMPILING_EMULATOR /usr/bin/qemu-system-aarch64)

# Where is the target environment
SET(CMAKE_FIND_ROOT_PATH  /usr/aarch64-linux-gnu)

# Search for programs in the build host directories
SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)

# For libraries and headers in the target directories
SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

