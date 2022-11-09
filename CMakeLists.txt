cmake_minimum_required(VERSION 3.10)

project(vktuto)

set(CMAKE_CXX_STANDARD 17)

find_package(Vulkan REQUIRED)
find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)

set(LIBS Vulkan::Vulkan ${GLM_LIBRARIES} glfw)

aux_source_directory(${CMAKE_SOURCE_DIR}/src SRC)

add_executable(vkdemo ${SRC})

target_link_libraries(vkdemo ${LIBS})