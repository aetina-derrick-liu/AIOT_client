#----------------------------------------------------------------
# Generated CMake target import file for configuration "Debug".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "eclipse-paho-mqtt-c::paho-mqtt3c-static" for configuration "Debug"
set_property(TARGET eclipse-paho-mqtt-c::paho-mqtt3c-static APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(eclipse-paho-mqtt-c::paho-mqtt3c-static PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_DEBUG "C"
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/lib/libpaho-mqtt3c.a"
  )

list(APPEND _IMPORT_CHECK_TARGETS eclipse-paho-mqtt-c::paho-mqtt3c-static )
list(APPEND _IMPORT_CHECK_FILES_FOR_eclipse-paho-mqtt-c::paho-mqtt3c-static "${_IMPORT_PREFIX}/lib/libpaho-mqtt3c.a" )

# Import target "eclipse-paho-mqtt-c::paho-mqtt3a-static" for configuration "Debug"
set_property(TARGET eclipse-paho-mqtt-c::paho-mqtt3a-static APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(eclipse-paho-mqtt-c::paho-mqtt3a-static PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_DEBUG "C"
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/lib/libpaho-mqtt3a.a"
  )

list(APPEND _IMPORT_CHECK_TARGETS eclipse-paho-mqtt-c::paho-mqtt3a-static )
list(APPEND _IMPORT_CHECK_FILES_FOR_eclipse-paho-mqtt-c::paho-mqtt3a-static "${_IMPORT_PREFIX}/lib/libpaho-mqtt3a.a" )

# Import target "eclipse-paho-mqtt-c::paho-mqtt3cs-static" for configuration "Debug"
set_property(TARGET eclipse-paho-mqtt-c::paho-mqtt3cs-static APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(eclipse-paho-mqtt-c::paho-mqtt3cs-static PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_DEBUG "C"
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/lib/libpaho-mqtt3cs.a"
  )

list(APPEND _IMPORT_CHECK_TARGETS eclipse-paho-mqtt-c::paho-mqtt3cs-static )
list(APPEND _IMPORT_CHECK_FILES_FOR_eclipse-paho-mqtt-c::paho-mqtt3cs-static "${_IMPORT_PREFIX}/lib/libpaho-mqtt3cs.a" )

# Import target "eclipse-paho-mqtt-c::paho-mqtt3as-static" for configuration "Debug"
set_property(TARGET eclipse-paho-mqtt-c::paho-mqtt3as-static APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(eclipse-paho-mqtt-c::paho-mqtt3as-static PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_DEBUG "C"
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/lib/libpaho-mqtt3as.a"
  )

list(APPEND _IMPORT_CHECK_TARGETS eclipse-paho-mqtt-c::paho-mqtt3as-static )
list(APPEND _IMPORT_CHECK_FILES_FOR_eclipse-paho-mqtt-c::paho-mqtt3as-static "${_IMPORT_PREFIX}/lib/libpaho-mqtt3as.a" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
