file(READ version.cpp APPL_VERSION_CONTENTS)
string(REGEX MATCH "[ \t]+([0-9]+),[ \t\n]*([0-9]+),[ \t\n]*([0-9]+)"
  APPL_VERSION ${APPL_VERSION_CONTENTS})
set(APPL_VERSION ${CMAKE_MATCH_1}.${CMAKE_MATCH_2}.${CMAKE_MATCH_3})
string(REGEX MATCH "\".c.[ 0-9a-zA-Z,.\-]+\""
  APPL_COPYRIGHT ${APPL_VERSION_CONTENTS})
set(APPL_COPYRIGHT ${APPL_COPYRIGHT})

file(GLOB SRCS "*.cpp")

if (NOT WIN32)
  add_custom_target(codespell
    COMMAND codespell ${INCLUDES} ${SRCS}
      -L nd,te,INOUT,inout,defint,iif,hel -S "*.po,*.png,*.ico,*.icns,*.bin"
    COMMENT "codespell" VERBATIM)

  add_custom_target(cppcheck
    COMMAND cppcheck 
      --std=c++${WEX_CXX_STANDARD}
      --quiet --enable=all
      --check-level=exhaustive
      --suppress=cppcheckError 
      --suppress=missingIncludeSystem
      --suppress=internalAstError 
      --suppress=invalidFunctionArg
      --suppress=noExplicitConstructor 
      --suppress=noOperatorEq 
      --suppress=unknownMacro 
      --suppress=unmatchedSuppression 
      --suppress=unusedScopedObject 
      --suppress=unusedFunction 
      --suppress=shadowArgument 
      --suppress=shadowVariable 
      ${SRCS}
    COMMENT "cppcheck" VERBATIM)

  add_custom_target(robocop
    COMMAND robocop
      -e 0201 -e 0202 -e 0308 -e 0309 -e 0413 -e 0505 -e 0704
      -e 1003 -e 1004 -e 1005 -e 1006
      WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/test/
    COMMENT "robocop" VERBATIM)      
endif()

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
