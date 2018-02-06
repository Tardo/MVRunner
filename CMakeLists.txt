cmake_minimum_required(VERSION 2.8)

# GET GAME VERSION
file(STRINGS src/game/version.h VERSION_LINE
  LIMIT_COUNT 1
  REGEX GAME_RELEASE_VERSION
)
if(VERSION_LINE MATCHES "\"([0-9]+)\\.([0-9]+)\\.([0-9]+[ab]?)\"")
  set(MVRUNNER_VERSION_MAJOR ${CMAKE_MATCH_1})
  set(MVRUNNER_VERSION_MINOR ${CMAKE_MATCH_2})
  set(MVRUNNER_VERSION_PATCH ${CMAKE_MATCH_3})
elseif(VERSION_LINE MATCHES "\"([0-9]+)\\.([0-9]+[ab]?)\"")
  set(MVRUNNER_VERSION_MAJOR ${CMAKE_MATCH_1})
  set(MVRUNNER_VERSION_MINOR ${CMAKE_MATCH_2})
  set(MVRUNNER_VERSION_PATCH "0")
else()
  message(FATAL_ERROR "Couldn't parse version from src/game/version.h")
endif()

# GLOBAL PARAMS
set(EXTERNAL_DIR "external/"
    CACHE PATH "The path to the external libraries.")
set(SRC_DIR "src/"
    CACHE PATH "The path to the main source.")
set(OTHER_DIR "other/"
    CACHE PATH "The path to the other source.")
set(BIN_DIR "bin/"
    CACHE PATH "The path to the build production.")


# Enable release symbols by default
# must be done before project() statement
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build (Debug or Release)" FORCE)
endif()

set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS
  src/game/version.h
)

# THE PROJECT
if(POLICY CMP0054)
  cmake_policy(SET CMP0054 NEW)
endif()
project("MVRunner")

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
	set(IS_DEBUG TRUE)
	set(BINARY_NAME "MVRunner_d")
else()
	set(IS_DEBUG FALSE)
	set(BINARY_NAME "MVRunner")
endif()

# MODULES
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")


# DETECT SYSTEM PARAMS
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(TARGET_BITS "64")
elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
    set(TARGET_BITS "32")
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set(TARGET_OS "windows")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(TARGET_OS "linux")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    set(TARGET_OS "mac")
endif()

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(TARGET_COMPILER "gcc")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    set(TARGET_COMPILER "clang")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    set(TARGET_COMPILER "cl")
endif()


# CONFIGURE FLAGS
if (TARGET_COMPILER STREQUAL "gcc" OR
    TARGET_COMPILER STREQUAL "clang")
    set(cxx_flags "${CMAKE_CXX_FLAGS} -Wall")
    set(c_flags "${CMAKE_C_FLAGS} -Wall")
elseif (TARGET_COMPILER STREQUAL "cl")
    set(cxx_flags "${CMAKE_CXX_FLAGS} /W4 /EHsc")
    set(c_flags "${CMAKE_C_FLAGS} /W4")
endif()

set(CMAKE_CXX_FLAGS "${cxx_flags} -std=c++0x"
    CACHE STRING "Flags used by the compiler during all build types." FORCE)
set(CMAKE_C_FLAGS   "${c_flags}"
    CACHE STRING "Flags used by the compiler during all build types." FORCE)


# DEFINITIONS
set(SFML_STATIC_LIBRARIES TRUE)


# FIND LIBRARIES
find_package(ZLIB REQUIRED)
find_package(SFML 2.4 COMPONENTS graphics window audio system REQUIRED)
find_package(Zpg REQUIRED)
include (FindOpenGL)


set(CLIENT_LIBS Box2D 
                tmxparser
                tinyxml2
                ${SFML_LIBRARIES}
                ${ZPG_LIBRARIES}
                ${OPENGL_glu_LIBRARY}
                ${SFML_DEPENDENCIES}
                ${ZPG_DEPENDENCIES})


# INCLUDES
set(CLIENT_INCLUDES ${ZLIB_INCLUDE_DIR}
                    ${SFML_INCLUDE_DIR}
                    ${ZPG_INCLUDE_DIR}
                    ${EXTERNAL_DIR}
                    ${EXTERNAL_DIR}/tinyxml2
                    ${SRC_DIR})
include_directories(${CLIENT_INCLUDES})


# EXTERNAL LIBRARIES
add_subdirectory(${EXTERNAL_DIR} ${CMAKE_CURRENT_BINARY_DIR}/libs)


# GAME SOURCE
set(BASE_SRC    src/base/system.cpp)
set(ENGINE_SRC  src/engine/CAssetManager.cpp
                src/engine/CConfig.cpp
                src/engine/CGame.cpp
                src/engine/CLocalization.cpp
                src/engine/CNoise.cpp
                src/engine/CSystemBox2D.cpp
                src/engine/CSystemFx.cpp
                src/engine/CSystemLight.cpp
                src/engine/CSystemSound.cpp)
set(GAME_SRC    src/game/components/CMapRender.cpp
                src/game/components/CMenus.cpp
                src/game/components/CUI.cpp
                src/game/components/CPlayerRender.cpp
                src/game/components/CItemRender.cpp
                src/game/components/CParticleRender.cpp
                src/game/components/CControls.cpp
                src/game/components/CCamera.cpp
                src/game/controllers/CControllerMain.cpp
                src/game/controllers/CControllerMenu.cpp
                src/game/entities/primitives/CB2Circle.cpp
                src/game/entities/primitives/CB2Polygon.cpp
                src/game/entities/primitives/CB2Chain.cpp
                src/game/entities/CAmbientSound.cpp
                src/game/entities/CCharacter.cpp
                src/game/entities/CFire.cpp
                src/game/entities/CHitBox.cpp
                src/game/entities/CProjectile.cpp
                src/game/entities/CSign.cpp
                src/game/CContext.cpp
                src/game/CController.cpp
                src/game/CGameClient.cpp
                src/game/CPlayer.cpp
                src/game/CEntity.cpp
                src/game/CMap.cpp)
set(CLIENT_SRC  ${BASE_SRC} ${ENGINE_SRC} ${GAME_SRC})
 

# EXECUTABLE
add_executable(${BINARY_NAME} ${CLIENT_SRC})
target_link_libraries(${BINARY_NAME} ${CLIENT_LIBS})
set_property(TARGET ${BINARY_NAME} PROPERTY CXX_STANDARD 11)
if (TARGET_OS STREQUAL "windows")
	add_custom_command(TARGET ${BINARY_NAME} PRE_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${ZLIB_INCLUDE_DIR}/../bin/zlib.dll ${PROJECT_BINARY_DIR}/${CMAKE_BUILD_TYPE})
	add_custom_command(TARGET ${BINARY_NAME} PRE_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${SFML_INCLUDE_DIR}/../bin/openal32.dll ${PROJECT_BINARY_DIR}/${CMAKE_BUILD_TYPE})
endif()

# CREATE ZPG
if (TARGET_COMPILER STREQUAL "cl")
	if (TARGET_BITS STREQUAL "32")
		add_custom_target(create_zpg COMMAND "C:/Program Files (x86)/libZpg/zpg_packer.exe" ${PROJECT_BINARY_DIR}/${CMAKE_BUILD_TYPE}/assets.zpg -C -A ${CMAKE_SOURCE_DIR}/data/)
	else()
		add_custom_target(create_zpg COMMAND "C:/Program Files/libZpg/zpg_packer.exe" ${PROJECT_BINARY_DIR}/${CMAKE_BUILD_TYPE}/assets.zpg -C -A ${CMAKE_SOURCE_DIR}/data/)
	endif()
else()
	add_custom_target(create_zpg COMMAND zpg_packer assets.zpg -C -A ${CMAKE_SOURCE_DIR}/data/)
endif()

# CREATE ZIP FILE
if (TARGET_OS STREQUAL "windows")
	add_custom_target(create_zip COMMAND ${CMAKE_COMMAND} -E tar "cfv" "${BINARY_NAME}-${MVRUNNER_VERSION_MAJOR}.${MVRUNNER_VERSION_MINOR}.${MVRUNNER_VERSION_PATCH}-win${TARGET_BITS}.zip" --format=zip
	       								 "${PROJECT_BINARY_DIR}/${CMAKE_BUILD_TYPE}/openal32.dll"
	       								 "${PROJECT_BINARY_DIR}/${CMAKE_BUILD_TYPE}/zlib.dll"
	       								 "${PROJECT_BINARY_DIR}/${CMAKE_BUILD_TYPE}/assets.zpg"
	       								 "${PROJECT_BINARY_DIR}/${CMAKE_BUILD_TYPE}/${BINARY_NAME}.exe")
else()
	add_custom_target(create_zip COMMAND ${CMAKE_COMMAND} -E tar "cfv" "${BINARY_NAME}-${MVRUNNER_VERSION_MAJOR}.${MVRUNNER_VERSION_MINOR}.${MVRUNNER_VERSION_PATCH}-linux${TARGET_BITS}.zip" --format=zip
	       								 "${PROJECT_BINARY_DIR}/assets.zpg"
	       								 "${PROJECT_BINARY_DIR}/${BINARY_NAME}")
endif()
add_dependencies(create_zip ${BINARY_NAME} create_zpg)
                        
# INSTALL
install(TARGETS ${BINARY_NAME} RUNTIME DESTINATION ${BIN_DIR} COMPONENT binaries)
if (TARGET_OS STREQUAL "windows")
	install(FILES ${PROJECT_BINARY_DIR}/${CMAKE_BUILD_TYPE}/zlib.dll DESTINATION ${BIN_DIR} COMPONENT binaries)
	install(FILES ${PROJECT_BINARY_DIR}/${CMAKE_BUILD_TYPE}/openal32.dll DESTINATION ${BIN_DIR} COMPONENT binaries)
endif()
if (TARGET_COMPILER STREQUAL "cl")
	install(FILES ${PROJECT_BINARY_DIR}/${CMAKE_BUILD_TYPE}/assets.zpg DESTINATION ${BIN_DIR} COMPONENT binaries)
else()
	install(FILES ${PROJECT_BINARY_DIR}/assets.zpg DESTINATION ${BIN_DIR} COMPONENT binaries)
endif()


# CPACK
include(InstallRequiredSystemLibraries)
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "MVRunner A Reto Game")
set(CPACK_PACKAGE_VENDOR "Alexandre Díaz")
if (TARGET_OS STREQUAL "windows")
	SET(CPACK_GENERATOR "NSIS ZIP")
	SET(CPACK_NSIS_DISPLAY_NAME "${CPACK_PACKAGE_INSTALL_DIRECTORY} MVRunner")
	SET(CPACK_NSIS_CONTACT "dev@redneboa.es")
	SET(CPACK_NSIS_MODIFY_PATH ON)
endif()
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/COPYING")
set(CPACK_PACKAGE_VERSION_MAJOR "${MVRUNNER_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${MVRUNNER_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${MVRUNNER_VERSION_PATCH}")
include(CPack)
