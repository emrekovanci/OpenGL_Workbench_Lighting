cmake_minimum_required(VERSION 3.8)

project(OpenGL_Lighting)

set(CMAKE_CXX_STANDARD 17)

add_executable(OpenGL_Lighting
	"headers/Vertex.hpp"
	"headers/PointLight.hpp"
	"headers/FreeLookCamera.hpp"
	"Model.cpp"
	"Mesh.cpp"
	"Shader.cpp"
	"TextureManager.cpp"
	"Main.cpp"
)

find_package(glad CONFIG REQUIRED)
target_link_libraries(OpenGL_Lighting PRIVATE glad::glad)

find_package(glfw3 CONFIG REQUIRED)
target_link_libraries(OpenGL_Lighting PRIVATE glfw)

find_package(Stb REQUIRED)
target_include_directories(OpenGL_Lighting PRIVATE ${Stb_INCLUDE_DIR})

find_package(glm CONFIG REQUIRED)
target_link_libraries(OpenGL_Lighting PRIVATE glm::glm)

find_package(imgui CONFIG REQUIRED)
target_link_libraries(OpenGL_Lighting PRIVATE imgui::imgui)

find_package(assimp CONFIG REQUIRED)
target_link_libraries(OpenGL_Lighting PRIVATE assimp::assimp)

set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${PROJECT_NAME})

add_custom_command(
    TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/resources $<TARGET_FILE_DIR:${PROJECT_NAME}>/resources
)