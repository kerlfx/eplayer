cmake_minimum_required(VERSION 3.0.0)
project(EPlayer VERSION 0.1.0)

include(CTest)
enable_testing()

add_subdirectory(src/vulkan)


if(MSVC)
    # add_compile_options("$<$<C_COMPILER_ID:MSVC>:/std:c++17>")
    add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
    add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")
endif()

aux_source_directory(src/ src)

add_executable(EPlayer ${src})

set_target_properties( EPlayer
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/output"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/output"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/output"
    EXECUTABLE_OUTPUT_PATH "${CMAKE_CURRENT_SOURCE_DIR}/output"
)

target_include_directories(EPlayer 
    PUBLIC "3rd/include" "3rd/include/ffmpeg"
)

target_link_directories(EPlayer 
    PUBLIC "3rd/lib" "3rd/lib/ffmpeg"
)
target_link_libraries(EPlayer    
   avformat.dll avutil.dll avcodec.dll swscale.dll
   vulkan/vulkan-1 GLFW/glfw3 
   elog/elog 
   vulkanview
)



set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
