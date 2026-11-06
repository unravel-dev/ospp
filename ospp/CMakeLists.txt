file(GLOB libsrc *.h *.cpp *.hpp *.c *.cc)

set(target_name ospp)

file(GLOB libsrc "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
                 "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
                 "${CMAKE_CURRENT_SOURCE_DIR}/*.hpp"
                 "${CMAKE_CURRENT_SOURCE_DIR}/*.c"
                 "${CMAKE_CURRENT_SOURCE_DIR}/*.cc")

# The browser entry for host runtimes that never call main; other platforms enter through main.h.
if(NOT EMSCRIPTEN)
	list(REMOVE_ITEM libsrc "${CMAKE_CURRENT_SOURCE_DIR}/main_web.cpp")
endif()

message( STATUS "--------------------------------" )
if("${OSPP_BACKEND_TYPE}" STREQUAL "SDL")
    message( STATUS "SDL WINDOW backend selected." )
	file(GLOB impl_src "${CMAKE_CURRENT_SOURCE_DIR}/impl/sdl/*.h"
					"${CMAKE_CURRENT_SOURCE_DIR}/impl/sdl/*.cpp"
					"${CMAKE_CURRENT_SOURCE_DIR}/impl/sdl/*.hpp"
					"${CMAKE_CURRENT_SOURCE_DIR}/impl/sdl/*.c"
					"${CMAKE_CURRENT_SOURCE_DIR}/impl/sdl/*.cc")

elseif("${OSPP_BACKEND_TYPE}" STREQUAL "GLFW")
	message( STATUS "GLFW WINDOW backend selected." )
	file(GLOB impl_src "${CMAKE_CURRENT_SOURCE_DIR}/impl/glfw/*.h"
					"${CMAKE_CURRENT_SOURCE_DIR}/impl/glfw/*.cpp"
					"${CMAKE_CURRENT_SOURCE_DIR}/impl/glfw/*.hpp"
					"${CMAKE_CURRENT_SOURCE_DIR}/impl/glfw/*.c"
					"${CMAKE_CURRENT_SOURCE_DIR}/impl/glfw/*.cc")

elseif("${OSPP_BACKEND_TYPE}" STREQUAL "MML")

	message( STATUS "MML WINDOW backend selected." )
	file(GLOB impl_src "${CMAKE_CURRENT_SOURCE_DIR}/impl/mml/*.h"
					"${CMAKE_CURRENT_SOURCE_DIR}/impl/mml/*.cpp"
					"${CMAKE_CURRENT_SOURCE_DIR}/impl/mml/*.hpp"
					"${CMAKE_CURRENT_SOURCE_DIR}/impl/mml/*.c"
					"${CMAKE_CURRENT_SOURCE_DIR}/impl/mml/*.cc")

else()
    message( FATAL_ERROR "No backend type was selected." )
endif()
message( STATUS "--------------------------------" )

list(APPEND libsrc ${impl_src})

add_library(${target_name} ${libsrc})

if("${OSPP_BACKEND_TYPE}" STREQUAL "SDL")
	target_compile_definitions(${target_name} PRIVATE SDL_BACKEND)
	if(BUILD_SHARED_LIBS)
		target_link_libraries(${target_name} PRIVATE SDL3-shared)
	else()
		target_link_libraries(${target_name} PRIVATE SDL3-static)
	endif()

elseif("${OSPP_BACKEND_TYPE}" STREQUAL "GLFW")
	target_compile_definitions(${target_name} PRIVATE GLFW_BACKEND)
	if(WIN32)
		target_compile_definitions(${target_name} PRIVATE GLFW_EXPOSE_NATIVE_WIN32)
	elseif(APPLE)
		target_compile_definitions(${target_name} PRIVATE GLFW_EXPOSE_NATIVE_COCOA)
	else()
		#target_compile_definitions(${target_name} PRIVATE GLFW_EXPOSE_NATIVE_WAYLAND)
		#target_compile_definitions(${target_name} PRIVATE GLFW_EXPOSE_NATIVE_MIR)
		target_compile_definitions(${target_name} PRIVATE GLFW_EXPOSE_NATIVE_X11)
	endif()
	target_link_libraries(${target_name} PRIVATE glfw)


elseif("${OSPP_BACKEND_TYPE}" STREQUAL "MML")
	target_compile_definitions(${target_name} PRIVATE MML_BACKEND)

	target_link_libraries(${target_name} PRIVATE mml-window)

endif()

target_link_libraries(${target_name} PUBLIC ${CMAKE_DL_LIBS})
target_compile_definitions(${target_name} PRIVATE OS_NO_EXCEPTIONS)

target_include_directories(${target_name}
    PUBLIC
		${CMAKE_CURRENT_SOURCE_DIR}/..
)
set_target_properties(${target_name} PROPERTIES
    CXX_STANDARD 14
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
    POSITION_INDEPENDENT_CODE ON
    WINDOWS_EXPORT_ALL_SYMBOLS ON
)
