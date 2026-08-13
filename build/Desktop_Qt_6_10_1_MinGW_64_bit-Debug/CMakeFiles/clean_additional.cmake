# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "ArcaneCampusDatabaseSmokeTest_autogen"
  "ArcaneCampusDatabase_autogen"
  "CMakeFiles\\ArcaneCampusDatabaseSmokeTest_autogen.dir\\AutogenUsed.txt"
  "CMakeFiles\\ArcaneCampusDatabaseSmokeTest_autogen.dir\\ParseCache.txt"
  "CMakeFiles\\ArcaneCampusDatabase_autogen.dir\\AutogenUsed.txt"
  "CMakeFiles\\ArcaneCampusDatabase_autogen.dir\\ParseCache.txt"
  "CMakeFiles\\HogwartsOnline_autogen.dir\\AutogenUsed.txt"
  "CMakeFiles\\HogwartsOnline_autogen.dir\\ParseCache.txt"
  "HogwartsOnline_autogen"
  )
endif()
