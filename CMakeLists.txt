cmake_minimum_required(VERSION 3.19)

project(m2dev-client-src)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ASan support
option(ENABLE_ASAN "Enable AddressSanitizer" OFF)

set(CMAKE_MODULE_PATH
	${CMAKE_MODULE_PATH}
	"${CMAKE_CURRENT_SOURCE_DIR}/buildtool"
)

include(Utils)

set(CMAKE_CONFIGURATION_TYPES "Debug;RelWithDebInfo;Release" CACHE STRING "Configurations" FORCE)
set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<$<CONFIG:Debug,RelWithDebInfo,Release>:ProgramDatabase>")

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Zi")
set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /DEBUG")

add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")
add_compile_options(/MP)

if(MSVC)
	add_compile_definitions(UNICODE _UNICODE)
endif()

add_compile_options(
	$<$<CXX_COMPILER_ID:MSVC>:/wd4828>
	$<$<CXX_COMPILER_ID:MSVC>:/wd4996>
	$<$<CXX_COMPILER_ID:MSVC>:/wd5033>
)

# ASan flags
if(ENABLE_ASAN)
	if(MSVC)
		add_compile_options(/fsanitize=address)
		add_link_options(/fsanitize=address)
		add_definitions(-D_DISABLE_VECTOR_ANNOTATION)
		add_definitions(-D_DISABLE_STRING_ANNOTATION)
	else()
		add_compile_options(-fsanitize=address -fno-omit-frame-pointer)
		add_link_options(-fsanitize=address)
	endif()
endif()

add_definitions(-DNOMINMAX)
add_definitions(-DWIN32_LEAN_AND_MEAN)
add_definitions(-D_CRT_SECURE_NO_WARNINGS)
add_definitions(-DWINDOWS_IGNORE_PACKING_MISMATCH)

add_definitions(-DUSE_LOD)
add_definitions(-DDUNGEON_WORK)
add_definitions(-DBUILDING_GRANNY_STATIC)
add_definitions(-DGRANNY_THREADED)
add_definitions(-DPy_NO_ENABLE_SHARED)

add_compile_definitions("$<$<CONFIG:Debug>:DEBUG>")
add_compile_definitions("$<$<CONFIG:RelWithDebInfo>:_DISTRIBUTE>")
add_compile_definitions("$<$<CONFIG:Release>:_DISTRIBUTE>")

include_directories("src")
include_directories("extern/include")
include_directories("vendor/libsodium/src/libsodium/include")
include_directories("vendor/freetype-2.13.3/include")

# Add subdirectories for libraries and executables
add_subdirectory(vendor)
# --- Protobuf: mesajele sistemelor noi (proto/, aceleasi fisiere ca pe server) ---
# system = vcpkg (x64-windows-static); vendor = compilat local din vendor/ (inca neimplementat)
set(M2_PROTOBUF_SOURCE "system" CACHE STRING "De unde vine Protobuf: system sau vendor")
set_property(CACHE M2_PROTOBUF_SOURCE PROPERTY STRINGS system vendor)
if (M2_PROTOBUF_SOURCE STREQUAL "system")
	find_package(Protobuf CONFIG REQUIRED)
	message(STATUS "Protobuf ${Protobuf_VERSION} (system)")
elseif (M2_PROTOBUF_SOURCE STREQUAL "vendor")
	message(FATAL_ERROR "M2_PROTOBUF_SOURCE=vendor nu e inca implementat")
else()
	message(FATAL_ERROR "M2_PROTOBUF_SOURCE trebuie sa fie system sau vendor, nu ${M2_PROTOBUF_SOURCE}")
endif()
if (NOT COMMAND protobuf_generate)
	message(FATAL_ERROR "Protobuf gasit, dar fara functia protobuf_generate (versiune prea veche?)")
endif()
add_subdirectory(proto)

add_subdirectory(src)
add_subdirectory(extern)