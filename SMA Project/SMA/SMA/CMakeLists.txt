add_executable (SMA "SMA.cpp" )
if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET SMA PROPERTY CXX_STANDARD 20)
endif()