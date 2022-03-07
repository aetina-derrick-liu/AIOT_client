#----------------------------------------------------------------
# Generated CMake target import file.
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "eclipse-paho-mqtt-c::paho-mqtt3c-static" for configuration ""
set_property(TARGET eclipse-paho-mqtt-c::paho-mqtt3c-static APPEND PROPERTY IMPORTED_CONFIGURATIONS NOCONFIG)
set_target_properties(eclipse-paho-mqtt-c::paho-mqtt3c-static PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_NOCONFIG "C"
  IMPORTED_LOCATION_NOCONFIG "${_IMPORT_PREFIX}/lib/libpaho-mqtt3c.a"
  )

list(APPEND _IMPORT_CHECK_TARGETS eclipse-paho-mqtt-c::paho-mqtt3c-static )
list(APPEND _IMPORT_CHECK_FILES_FOR_eclipse-paho-mqtt-c::paho-mqtt3c-static "${_IMPORT_PREFIX}/lib/libpaho-mqtt3c.a" )

# Import target "eclipse-paho-mqtt-c::paho-mqtt3a-static" for configuration ""
set_property(TARGET eclipse-paho-mqtt-c::paho-mqtt3a-static APPEND PROPERTY IMPORTED_CONFIGURATIONS NOCONFIG)
set_target_properties(eclipse-paho-mqtt-c::paho-mqtt3a-static PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_NOCONFIG "C"
  IMPORTED_LOCATION_NOCONFIG "${_IMPORT_PREFIX}/lib/libpaho-mqtt3a.a"
  )

list(APPEND _IMPORT_CHECK_TARGETS eclipse-paho-mqtt-c::paho-mqtt3a-static )
list(APPEND _IMPORT_CHECK_FILES_FOR_eclipse-paho-mqtt-c::paho-mqtt3a-static "${_IMPORT_PREFIX}/lib/libpaho-mqtt3a.a" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
