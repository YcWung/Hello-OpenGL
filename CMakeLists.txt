cmake_minimum_required(VERSION 3.0)
cmake_policy(VERSION 3.0)

include(CMakePrintHelpers)

project(hello-OpenGL)

# dependencies
# include_directories("${CMAKE_SOURCE_DIR}/include")
find_package(GLM REQUIRED)
message(STATUS "GLM INCLUDE: ${GLM_INCLUDE_DIRS}")
find_package(GLFW3 REQUIRED)
find_package(OpenGL REQUIRED)
add_library(GLAD "src/glad.c")
target_include_directories(GLAD PUBLIC "${CMAKE_SOURCE_DIR}/include")
add_library(stb_image "src/stb_image.cpp")
target_include_directories(stb_image PUBLIC "${CMAKE_SOURCE_DIR}/include")

# check dependencies
message(STATUS "OPENGL INCLUDE: ${OPENGL_INCLUDE_DIRS}")
message(STATUS "OPENGL LIBRARIES: ${OPENGL_LIBRARIES}")
cmake_print_properties(TARGETS glfw PROPERTIES LOCATION INTERFACE_INCLUDE_DIRECTORIES)

# add targets

add_executable(hello-window "src/hello-window.cpp")
target_link_libraries(hello-window GLAD glfw)

add_executable(hello-triangle "src/hello-triangle.cpp")
target_link_libraries(hello-triangle GLAD glfw)

add_executable(hello-triangle-elements "src/hello-triangle-elements.cpp")
target_link_libraries(hello-triangle-elements GLAD glfw)

# targets in sub-directories

set(OTHER_EXES
  camera_system
  )

foreach(OTHER_EXE ${OTHER_EXES})
  file(GLOB SOURCES
    "src/${OTHER_EXE}/*.cpp"
    "src/${OTHER_EXE}/*.h"
    "src/${OTHER_EXE}/*.vs"
    "src/${OTHER_EXE}/*.fs"
    "src/${OTHER_EXE}/*.gs"
    )
  add_executable(${OTHER_EXE} ${SOURCES})
  target_link_libraries(${OTHER_EXE} GLAD glfw stb_image)
  target_include_directories(${OTHER_EXE} PRIVATE ${GLM_INCLUDE_DIRS})
  set_target_properties(${OTHER_EXE} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${OTHER_EXE}
    RUNTIME_OUTPUT_DIRECTORY_DEBUG Debug/${OTHER_EXE}
    RUNTIME_OUTPUT_DIRECTORY_RELEASE Release/${OTHER_EXE}
    CXX_STANDARD 17
    )
  # get_target_property(OTHER_EXE_OUTPUT_DIR ${OTHER_EXE} RUNTIME_OUTPUT_DIRECTORY)
  # message(STATUS "${OTHER_EXE} OUTPUT IN: ${OTHER_EXE_OUTPUT_DIR}")
  # copy shader files into the build tree
  file(GLOB SHADERS
    "src/${OTHER_EXE}/*.vs"
    "src/${OTHER_EXE}/*.fs"
    "src/${OTHER_EXE}/*.gs"
    )
  foreach(SHADER ${SHADERS})
    add_custom_command(TARGET ${OTHER_EXE} POST_BUILD COMMAND
      ${CMAKE_COMMAND} -E copy_if_different ${SHADER} $<TARGET_FILE_DIR:${OTHER_EXE}>
      )
  endforeach(SHADER)
  # copy resources in to build tree
  file(GLOB RESOURCES "src/${OTHER_EXE}/resources/*")
  add_custom_command(TARGET ${OTHER_EXE} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:${OTHER_EXE}>/resources
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${RESOURCES} $<TARGET_FILE_DIR:${OTHER_EXE}>/resources
    )
endforeach(OTHER_EXE)
    
cmake_print_variables(CMAKE_RUNTIME_OUTPUT_DIRECTORY)
