cmake_minimum_required(VERSION 3.10)

project(vktuto)

set(CMAKE_CXX_STANDARD 17)

find_package(Vulkan REQUIRED)
find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)

file(GLOB SHADER_FILES "shader/*.vert" "shader/*.frag")
set(COMPILED_SHADER_FILES)

foreach(SHADER ${SHADER_FILES})
    get_filename_component(SHADER_NAME ${SHADER} NAME)
    get_filename_component(SHADER_EXT ${SHADER} EXT)
    string(REPLACE "." "" SHADER_EXT ${SHADER_EXT})
    set(SHADER_OUTPUT "${SHADER_EXT}.spv")
    set(SHADER_OUTPUT_PATH "${CMAKE_CURRENT_SOURCE_DIR}/shader/${SHADER_OUTPUT}")
    message(STATUS "shader name : ${SHADER_OUTPUT_PATH}")
    add_custom_command(
        OUTPUT ${SHADER_OUTPUT_PATH}
        COMMAND glslc ${SHADER} -o ${SHADER_OUTPUT_PATH} 
        DEPENDS ${SHADER}
    )
    list(APPEND COMPILED_SHADER_FILES ${SHADER_OUTPUT_PATH})
endforeach()

add_custom_target(shaders DEPENDS ${COMPILED_SHADER_FILES})

set(LIBS Vulkan::Vulkan ${GLM_LIBRARIES} glfw)

aux_source_directory(${CMAKE_SOURCE_DIR}/src SRC)

add_executable(vkdemo ${SRC})

add_dependencies(vkdemo shaders)

target_link_libraries(vkdemo ${LIBS})