cmake_minimum_required(VERSION 3.16)
project(ChromeBackground LANGUAGES CXX RC)

if(NOT WIN32)
    message(FATAL_ERROR "This application targets Windows. Use a Windows compiler or a Windows cross-compiler.")
endif()

add_executable(Chrome WIN32 src/main.cpp src/app.rc)
target_compile_features(Chrome PRIVATE cxx_std_17)
target_include_directories(Chrome PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/assets")
target_compile_definitions(Chrome PRIVATE UNICODE _UNICODE WIN32_LEAN_AND_MEAN NOMINMAX)
target_link_libraries(Chrome PRIVATE shell32 user32 gdi32)
if(MSVC)
    target_compile_options(Chrome PRIVATE $<$<COMPILE_LANGUAGE:CXX>:/W4> $<$<COMPILE_LANGUAGE:CXX>:/utf-8>)
else()
    target_compile_options(Chrome PRIVATE $<$<COMPILE_LANGUAGE:CXX>:-Wall;-Wextra;-Wpedantic>)
    # MinGW's GUI startup expects WinMain unless Unicode startup is enabled.
    target_link_options(Chrome PRIVATE -municode)
endif()
