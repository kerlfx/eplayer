cmake_minimum_required(VERSION 3.0.0)
project(vulkanview VERSION 0.1.0)

include(CTest)
enable_testing()


if(MSVC)
    add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
    add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")
endif()

aux_source_directory(. src)

add_library(vulkanview ${src})

set_target_properties( vulkanview
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/output"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/output"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/output"
    EXECUTABLE_OUTPUT_PATH "${CMAKE_CURRENT_SOURCE_DIR}/output"
)

target_include_directories(vulkanview 
    PUBLIC "../../include" "../../3rd/include"
)

target_link_directories(vulkanview 
    PUBLIC "../../lib" "../../3rd/lib"
)
target_link_libraries(vulkanview    
   elog/elog 
   vulkan/vulkan-1 GLFW/glfw3
)


set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
