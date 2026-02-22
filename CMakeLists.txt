cmake_minimum_required(VERSION 3.15)

project(WebView2-Desktop-Vue)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

include(CMake/CMakeRC.cmake)
include(CMake/WebView2.cmake)

include(FetchContent)

FetchContent_Populate(
    json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG 8167d2f64122c3ef7e5a0d9cab239fdba37aa54c
)

file(GLOB_RECURSE UI_RESOURCES_FILES "Resources/UI/dist/*")

cmrc_add_resource_library(UI_RESOURCES)
cmrc_add_resources(UI_RESOURCES WHENCE "Resources/UI/dist" ${UI_RESOURCES_FILES})

file(GLOB_RECURSE PROJECT_SOURCES Source/*.cpp)

add_executable(${PROJECT_NAME} WIN32
    ${PROJECT_SOURCES}
    
    Application.rc
)

target_include_directories(${PROJECT_NAME} PRIVATE
    Source

    ${WEBVIEW2_INCLUDE_DIRS}

    ${json_SOURCE_DIR}/single_include
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    UI_RESOURCES

    ${WEBVIEW2_LIB_DIR}/WebView2LoaderStatic.lib
    
    shlwapi
)