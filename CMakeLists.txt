cmake_minimum_required(VERSION 3.25.0)
project(vstgui-d2d1-test)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Path to VSTGUI source (our MinGW-patched fork)
set(VSTGUI_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../vstgui" CACHE PATH "Path to VSTGUI source root")

# Include VSTGUI headers
include_directories(${VSTGUI_ROOT})

# Our test app source
add_executable(vstgui_d2d1_test WIN32
    src/main.cpp
)

# Link against pre-built VSTGUI libraries
target_link_directories(vstgui_d2d1_test PRIVATE
    ${VSTGUI_ROOT}/build-mingw/Release/libs
)

target_link_libraries(vstgui_d2d1_test PRIVATE
    vstgui
    vstgui_uidescription
    d2d1
    dwrite
    ole32
    oleaut32
    uuid
    windowscodecs
    d3d11
    dxgi
    dcomp
    dwmapi
    shlwapi
    imm32
    comctl32
    msimg32
    opengl32
)

# Static linking for MinGW
if(MINGW)
    target_link_options(vstgui_d2d1_test PRIVATE -static-libgcc -static-libstdc++)
    target_compile_definitions(vstgui_d2d1_test PRIVATE UNICODE _UNICODE)
endif()
