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

file(COPY ${CMAKE_SOURCE_DIR}/shaders DESTINATION ${CMAKE_BINARY_DIR})
file(COPY ${CMAKE_SOURCE_DIR}/textures DESTINATION ${CMAKE_BINARY_DIR})
file(COPY ${CMAKE_SOURCE_DIR}/models DESTINATION ${CMAKE_BINARY_DIR})