mitk_create_module(IASegmentadorAbdominal
  DEPENDS PUBLIC MitkCore MitkPython MitkDataTypesExt
  PACKAGE_DEPENDS Qt5|Core Qt5|Widgets
  WARNINGS_NO_ERRORS
)

add_subdirectory(cmdapps)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/NiftyNet DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
