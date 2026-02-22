find_program(NUGET_EXE nuget.exe)
if(NOT NUGET_EXE)
    message(FATAL_ERROR "NuGet.exe not found! Please install NuGet and make sure it's in your PATH")
endif()

set(WEBVIEW2_VERSION "1.0.3351.48" CACHE STRING "WebView2 package version")
set(WIL_VERSION "1.0.250325.1" CACHE STRING "WIL package version")

file(WRITE ${CMAKE_BINARY_DIR}/packages.config
    "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
    "<packages>\n"
    "  <package id=\"Microsoft.Web.WebView2\" version=\"${WEBVIEW2_VERSION}\" targetFramework=\"native\" />\n"
    "  <package id=\"Microsoft.Windows.ImplementationLibrary\" version=\"${WIL_VERSION}\" targetFramework=\"native\" />\n"
    "</packages>\n"
)

message(STATUS "Restoring NuGet packages...")

execute_process(
    COMMAND ${NUGET_EXE} restore ${CMAKE_BINARY_DIR}/packages.config -PackagesDirectory ${CMAKE_BINARY_DIR}/packages
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    RESULT_VARIABLE nuget_result
    OUTPUT_VARIABLE nuget_output
    ERROR_VARIABLE nuget_error
)

if(NOT nuget_result EQUAL 0)
    message(FATAL_ERROR "NuGet restore failed:\n${nuget_error}")
endif()

set(WEBVIEW2_DIR "${CMAKE_BINARY_DIR}/packages/Microsoft.Web.WebView2.${WEBVIEW2_VERSION}")
set(WIL_DIR "${CMAKE_BINARY_DIR}/packages/Microsoft.Windows.ImplementationLibrary.${WIL_VERSION}")

if(NOT EXISTS "${WEBVIEW2_DIR}" OR NOT EXISTS "${WIL_DIR}")
    message(FATAL_ERROR "Failed to restore NuGet packages")
endif()

set(WEBVIEW2_INCLUDE_DIRS
    "${WEBVIEW2_DIR}/build/native/include"
    "${WIL_DIR}/include"
)

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(WEBVIEW2_LIB_DIR "${WEBVIEW2_DIR}/build/native/x64")
else()
    set(WEBVIEW2_LIB_DIR "${WEBVIEW2_DIR}/build/native/x86")
endif()

set(WEBVIEW2_FOUND TRUE)

message(STATUS "WebView2 setup complete")