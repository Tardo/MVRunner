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
set(ZPG_PACKER_BIN "zpg_packer"
    CACHE PATH "The path to the zpg_packer binary.")


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
if (POLICY CMP0074)
  cmake_policy(SET CMP0074 NEW)
endif()
project("MVRunner")

set(BINARY_NAME "MVRunner")
set(PROJECT_VERSION_MAJOR ${MVRUNNER_VERSION_MAJOR})
set(PROJECT_VERSION_MINOR ${MVRUNNER_VERSION_MINOR})
set(PROJECT_VERSION_PATCH ${MVRUNNER_VERSION_PATCH})

if (CMAKE_BUILD_TYPE STREQUAL "Debug" OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo" OR CMAKE_CFG_INTDIR STREQUAL "Debug" OR CMAKE_CFG_INTDIR STREQUAL "RelWithDebInfo")
	set(IS_DEBUG TRUE)
	set(CMAKE_DEBUG_POSTFIX "_d")
	if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
		set(CMAKE_EXECUTABLE_SUFFIX "_d.exe")
	else()
		set(CMAKE_EXECUTABLE_SUFFIX "_d")
	endif()
else()
	set(IS_DEBUG FALSE)
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
    set(cxx_flags "${CMAKE_CXX_FLAGS} /W4 /EHsc /MP /GS")
    set(c_flags "${CMAKE_C_FLAGS} /W4 /MP /GS")
    add_definitions(-D_CRT_SECURE_NO_WARNINGS -D_SCL_SECURE_NO_WARNINGS)
endif()

set(CMAKE_CXX_FLAGS "${cxx_flags} -std=c++0x"
    CACHE STRING "Flags used by the compiler during all build types." FORCE)
set(CMAKE_C_FLAGS   "${c_flags}"
    CACHE STRING "Flags used by the compiler during all build types." FORCE)


# DEFINITIONS
set(SFML_STATIC_LIBRARIES TRUE)


# FIND LIBRARIES
include (FindOpenGL)
include (FindPkgConfig)
find_package(ZLIB REQUIRED)
find_package(Zpg REQUIRED)
pkg_check_modules (SFML	REQUIRED sfml-graphics>=2.5 sfml-window>=2.5 sfml-audio>=2.5 sfml-system>=2.5)

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
                src/engine/CSystemSound.cpp)
set(GAME_SRC    src/game/components/CMapRender.cpp
                src/game/components/CMenus.cpp
                src/game/components/CUI.cpp
                src/game/components/CPlayerRender.cpp
                src/game/components/CItemRender.cpp
                src/game/components/CSimpleParticleRender.cpp
                src/game/components/CParticleSystemRender.cpp
                src/game/components/CControls.cpp
                src/game/components/CCamera.cpp
                src/game/components/CDebuggerRender.cpp
                src/game/components/CLightRender.cpp
                src/game/components/CFluidRender.cpp
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
                src/game/entities/CLight.cpp
                src/game/entities/CSimpleParticle.cpp
                src/game/entities/CWater.cpp
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
	if (IS_DEBUG)
		add_custom_command(TARGET ${BINARY_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${ZLIB_INCLUDE_DIR}/../bin/zlibd.dll ${PROJECT_BINARY_DIR}/${CMAKE_BUILD_TYPE})
	else()
		add_custom_command(TARGET ${BINARY_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${ZLIB_INCLUDE_DIR}/../bin/zlib.dll ${PROJECT_BINARY_DIR}/${CMAKE_BUILD_TYPE})
	endif()
	add_custom_command(TARGET ${BINARY_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${SFML_INCLUDE_DIR}/../bin/openal32.dll ${PROJECT_BINARY_DIR}/${CMAKE_BUILD_TYPE})
endif()

# CREATE ZPG
if (TARGET_OS STREQUAL "windows")
	if (ZPG_PACKER_BIN STREQUAL "zpg_packer")
		set(ZPG_PACKER_BIN "zpg_packer.exe")
	endif()
	add_custom_target(create_zpg COMMAND "${ZPG_PACKER_BIN}" ${PROJECT_BINARY_DIR}/${CMAKE_BUILD_TYPE}/assets.zpg -C -A ${CMAKE_SOURCE_DIR}/data/)
else()
	add_custom_target(create_zpg COMMAND zpg_packer assets.zpg -C -A ${CMAKE_SOURCE_DIR}/data/)
endif()
                        
# INSTALL
install(TARGETS ${BINARY_NAME} RUNTIME DESTINATION ${BIN_DIR} COMPONENT binaries)
if (TARGET_OS STREQUAL "windows")
	if (IS_DEBUG)
		install(FILES ${PROJECT_BINARY_DIR}/${CMAKE_BUILD_TYPE}/zlibd.dll DESTINATION bin/ COMPONENT binaries)
	else()
		install(FILES ${PROJECT_BINARY_DIR}/${CMAKE_BUILD_TYPE}/zlib.dll DESTINATION bin/ COMPONENT binaries)
	endif()
	install(FILES ${PROJECT_BINARY_DIR}/${CMAKE_BUILD_TYPE}/openal32.dll DESTINATION ${BIN_DIR} COMPONENT binaries)
endif()
if (TARGET_COMPILER STREQUAL "cl")
	install(FILES ${PROJECT_BINARY_DIR}/${CMAKE_BUILD_TYPE}/assets.zpg DESTINATION ${BIN_DIR} COMPONENT binaries)
else()
	install(FILES ${PROJECT_BINARY_DIR}/assets.zpg DESTINATION ${BIN_DIR} COMPONENT binaries)
endif()


# CPACK
include(InstallRequiredSystemLibraries)
set(CPACK_PACKAGE_NAME ${PROJECT_NAME})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "MVRunner A Reto Game")
set(CPACK_PACKAGE_VENDOR "Alexandre Díaz")
if (TARGET_OS STREQUAL "windows")
	SET(CPACK_GENERATOR ZIP)
endif()
set(CPACK_ARCHIVE_COMPONENT_INSTALL ON)
set(CPACK_COMPONENTS_ALL binaries)
set(CPACK_PACKAGE_VERSION_MAJOR "${PROJECT_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${PROJECT_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${PROJECT_VERSION_PATCH}")
if(VERSION_PATCH STREQUAL "0")
  set(CPACK_PACKAGE_VERSION ${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR})
else()
  set(CPACK_PACKAGE_VERSION ${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH})
endif()
SET(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE")
SET(CPACK_RESOURCE_FILE_README "${CMAKE_SOURCE_DIR}/README.md")
set(CPACK_SYSTEM_NAME ${CMAKE_SYSTEM_NAME})

if(TARGET_OS AND TARGET_BITS)
  if(TARGET_OS STREQUAL "windows")
    set(CPACK_SYSTEM_NAME "win${TARGET_BITS}")
    set(CPACK_GENERATOR ZIP)
  elseif(TARGET_OS STREQUAL "linux")
    # Assuming Intel here.
    if(TARGET_BITS EQUAL 32)
      set(CPACK_SYSTEM_NAME "linux_x86")
    elseif(TARGET_BITS EQUAL 64)
      set(CPACK_SYSTEM_NAME "linux_x86_64")
    endif()
  endif()
endif()
if (IS_DEBUG)
	set(CPACK_PACKAGE_FILE_NAME ${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${CPACK_SYSTEM_NAME}-Debug)
else()
	set(CPACK_PACKAGE_FILE_NAME ${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${CPACK_SYSTEM_NAME})
endif()
set(CPACK_ARCHIVE_PORTABLE_FILE_NAME ${CPACK_PACKAGE_FILE_NAME})
set(CPACK_SOURCE_PACKAGE_FILE_NAME ${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-src)
set(CPACK_SOURCE_FILES
	CMakeLists.txt
	README.md
	LICENSE
	COPYING
	cmake/
	src/
	data/
	external/
	scripts/
)
set(CPACK_SOURCE_IGNORE_FILES
	"\\\\.pyc$"
	"/\\\\.git"
	"/__pycache__/"
)
include(CPack)
