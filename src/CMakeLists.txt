file(READ version.cpp APPL_VERSION_CONTENTS)
string(REGEX MATCH "[ \t]+([0-9]+),[ \t\n]*([0-9]+),[ \t\n]*([0-9]+)"
  APPL_VERSION ${APPL_VERSION_CONTENTS})
set(APPL_VERSION ${CMAKE_MATCH_1}.${CMAKE_MATCH_2}.${CMAKE_MATCH_3})
string(REGEX MATCH "\".c.[ 0-9a-zA-Z,.\-]+\""
  APPL_COPYRIGHT ${APPL_VERSION_CONTENTS})
set(APPL_COPYRIGHT ${APPL_COPYRIGHT})

file(GLOB SRCS "*.cpp")

if (WIN32)
  set(SRCS ${SRCS} app.rc)
endif()

if (NOT APPLE)
  add_executable(${PROJECT_NAME} WIN32 ${SRCS})
else()
  # Define some settings for the Bundle
  # add to mac osx Info.plist: <key>NSPrincipleClass</key> (new cmake version)
  set( MACOSX_BUNDLE_BUNDLE_NAME ${PROJECT_NAME} )
  set( MACOSX_BUNDLE_GUI_IDENTIFIER ${PROJECT_NAME} )
  set( MACOSX_BUNDLE_ICON_FILE app-large.icns )
  set( MACOSX_BUNDLE_INFO_STRING ${PROJECT_NAME} )
  set( MACOSX_BUNDLE_SHORT_VERSION_STRING ${APPL_VERSION} )
  set( MACOSX_BUNDLE_BUNDLE_VERSION ${APPL_VERSION} )
  set( MACOSX_BUNDLE_COPYRIGHT ${APPL_COPYRIGHT} )

  # create a bundle with an icon too!
  add_executable( ${PROJECT_NAME} MACOSX_BUNDLE ${SRCS} )
  
  # Allows for bundle re-creation just by running "make". Also installs bundle icon
  add_custom_target( osx_bundle_dirs
    COMMAND mkdir -p 
      ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.app/Contents/Resources
    COMMAND mkdir -p 
      ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.app/Contents/MacOS
    COMMAND cp 
      ${CMAKE_SOURCE_DIR}/${MACOSX_BUNDLE_ICON_FILE}
      ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.app/Contents/Resources/${MACOSX_BUNDLE_ICON_FILE}
   )
  add_dependencies( ${PROJECT_NAME} osx_bundle_dirs )
  # This tells cmake where to place files inside the bundle
  set_source_files_properties( 
    ${ProjectName_RESOURCES} ${ProjectName_TRANSLATIONS}
    PROPERTIES MACOSX_PACKAGE_LOCATION Resources )
endif()
