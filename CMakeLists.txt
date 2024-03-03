cmake_minimum_required(VERSION 3.21)

project(syncped)

find_package(WEX REQUIRED)

if (wexENABLE_GETTEXT)
  include(FindGettext)
endif ()

include_directories(${wex_INCLUDE_DIR})
link_directories(${wex_LIB_DIR})

set(CPACK_GENERATOR "ZIP")
set(CPACK_PACKAGE_NAME "${PROJECT_NAME}")
set(CPACK_PACKAGE_VERSION "${APPL_VERSION}")
set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-v${CPACK_PACKAGE_VERSION}")

set(CMAKE_OSX_ARCHITECTURES "arm64")

add_subdirectory(src)
add_subdirectory(locale)

target_link_libraries(${PROJECT_NAME} ${wex_LIBRARIES})

install(TARGETS ${PROJECT_NAME} DESTINATION bin)

if (wexINSTALL)
  # Install some wex files so cpack can create a complete package.
  set(CONFIG_INSTALL_DIR bin)
  set(WEX_SOURCE_DIR ${CMAKE_SOURCE_DIR}/../../wex)
    
  install(DIRECTORY ${WEX_SOURCE_DIR}/data/ 
    DESTINATION ${CONFIG_INSTALL_DIR} 
    FILES_MATCHING PATTERN "*.xml" )
  
  install(DIRECTORY ${WEX_SOURCE_DIR}/data/ 
    DESTINATION ${CONFIG_INSTALL_DIR} 
    FILES_MATCHING PATTERN "*.xsl" )
  
  install(DIRECTORY ${WEX_SOURCE_DIR}/data/ 
    DESTINATION ${CONFIG_INSTALL_DIR} 
    FILES_MATCHING PATTERN "*.txt" )
endif ()

if (NOT WIN32)
  if (EXISTS "/usr/share/applications")
    install(FILES ${PROJECT_NAME}.desktop DESTINATION /usr/share/applications)
  endif()
  if (EXISTS "/usr/share/icons")
    install(FILES ${PROJECT_NAME}.png DESTINATION /usr/share/icons/hicolor/scalable/apps/)
    install(FILES ${PROJECT_NAME}.svg DESTINATION /usr/share/icons/hicolor/scalable)
  endif()
endif()

include(CPack)
