cmake_minimum_required(VERSION 2.6)
project(PHYLAB)

include_directories(
  ${PROJECT_SOURCE_DIR}/src
)

if(WIN32)
  add_definitions(-DSDL_MAIN_HANDLED -DGLEW_STATIC)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -static")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static -static-libgcc -static-libstdc++")
endif()

add_library(re
  src/re/Context.c
  src/re/Renderer.c
  src/re/mathutil.c
  src/re/Buffer.c
  src/re/internal.c
  src/re/Shader.c
  src/re/Mesh.c
  src/re/util.c
)
if(WIN32)
  target_link_libraries(re glew32 opengl32 winmm imm32 version setupapi)
else()
  target_link_libraries(re GLEW GL m)
endif()

add_executable(phylab
  src/phylab/main.c
  src/phylab/TcpSocket.c
)
if(WIN32)
  target_link_libraries(phylab re SDL2 ws2_32)
else()
  target_link_libraries(phylab re SDL2)
endif()

add_executable(sample
  src/sample/main.cpp
)
target_link_libraries(sample ws2_32)