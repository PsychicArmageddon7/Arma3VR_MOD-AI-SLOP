cmake_minimum_required(VERSION 3.20)
project(Arma3VR LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
    message(FATAL_ERROR "Arma 3 VR must be compiled as a 64-bit extension (x64).")
endif()

set(SOURCES
    src/dllmain.cpp
    src/Extension/RVExtension.cpp
    src/Graphics/ArmaGraphics.cpp
    src/Graphics/DepthCaptureHook.cpp
    src/Graphics/StereoPipeline.cpp
    src/Tracking/FreeTrackProvider.cpp
    src/VR/OpenXRContext.cpp
)

set(HEADERS
    include/Common.h
    include/Extension/RVExtension.h
    include/Graphics/ArmaGraphics.h
    include/Graphics/DepthCaptureHook.h
    include/Graphics/StereoPipeline.h
    include/Tracking/FreeTrackProvider.h
    include/VR/OpenXRContext.h
)

add_library(Arma3VR_x64 SHARED ${SOURCES} ${HEADERS})

target_include_directories(Arma3VR_x64 PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_definitions(Arma3VR_x64 PRIVATE 
    ARMA3VR_EXTENSION_EXPORTS
    UNICODE
    _UNICODE
)

# Tell the linker where to find openxr_loader.lib inside the repo
target_link_directories(Arma3VR_x64 PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/lib
)

# Link Windows graphics and OpenXR
target_link_libraries(Arma3VR_x64 PRIVATE
    d3d11
    dxgi
    d3dcompiler
    openxr_loader
)

# Output compiled DLL directly into @Arma3VR
set_target_properties(Arma3VR_x64 PROPERTIES
    PREFIX ""
    OUTPUT_NAME "Arma3VR_x64"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/@Arma3VR"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_CURRENT_SOURCE_DIR}/@Arma3VR"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_CURRENT_SOURCE_DIR}/@Arma3VR"
)
