# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "CMakeFiles\\HogwartsOnline_autogen.dir\\AutogenUsed.txt"
  "CMakeFiles\\HogwartsOnline_autogen.dir\\ParseCache.txt"
  "HogwartsOnline_autogen"
  )
endif()
