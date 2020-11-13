cmake_minimum_required(VERSION 3.0)
cmake_policy(VERSION 3.0)

# for intellisense
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

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
find_package(assimp REQUIRED)

# check dependencies
message(STATUS "OPENGL INCLUDE: ${OPENGL_INCLUDE_DIRS}")
message(STATUS "OPENGL LIBRARIES: ${OPENGL_LIBRARIES}")
cmake_print_properties(TARGETS glfw PROPERTIES LOCATION INTERFACE_INCLUDE_DIRECTORIES)
cmake_print_variables(ASSIMP_INCLUDE_DIRS ASSIMP_LIBRARY_DIRS ASSIMP_LIBRARIES)
# cmake_print_properties(TARGETS assimp::assimp PROPERTIES LOCATION INTERFACE_INCLUDE_DIRECTORIES IMPORTED_LOCATION_RELEASE)

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
  3d_viewer
  )

foreach(OTHER_EXE ${OTHER_EXES})
  file(GLOB SOURCES
    "src/${OTHER_EXE}/*.cpp"
    "src/${OTHER_EXE}/*.h"
    "src/${OTHER_EXE}/*.vs"
    "src/${OTHER_EXE}/*.fs"
    "src/${OTHER_EXE}/*.gs"
    "src/${OTHER_EXE}/shaders/*"
    )
  add_executable(${OTHER_EXE} ${SOURCES})
  target_link_libraries(${OTHER_EXE} PRIVATE GLAD glfw stb_image)
  target_include_directories(${OTHER_EXE} PRIVATE ${GLM_INCLUDE_DIRS})
  # assimp
  target_include_directories(${OTHER_EXE} PRIVATE ${ASSIMP_INCLUDE_DIRS})
  target_link_directories(${OTHER_EXE} PRIVATE ${ASSIMP_LIBRARY_DIRS})
  target_link_libraries(${OTHER_EXE} PRIVATE ${ASSIMP_LIBRARIES})
  
  set_target_properties(${OTHER_EXE} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${OTHER_EXE}
    RUNTIME_OUTPUT_DIRECTORY_DEBUG Debug/${OTHER_EXE}
    RUNTIME_OUTPUT_DIRECTORY_RELEASE Release/${OTHER_EXE}
    CXX_STANDARD 17
    )
  # link shader files
  file(GLOB SHADERS
    "src/${OTHER_EXE}/*.vs"
    "src/${OTHER_EXE}/*.fs"
    "src/${OTHER_EXE}/*.gs"
    )
  foreach(SHADER ${SHADERS})
    get_filename_component(SHADER_NAME ${SHADER} NAME)
    add_custom_command(TARGET ${OTHER_EXE} POST_BUILD COMMAND
      ${CMAKE_COMMAND} -E create_symlink ${SHADER} $<TARGET_FILE_DIR:${OTHER_EXE}>/${SHADER_NAME}
      )
  endforeach(SHADER)
  # link shaders directory
  if(EXISTS ${CMAKE_SOURCE_DIR}/src/${OTHER_EXE}/shaders)
    add_custom_command(TARGET ${OTHER_EXE} POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E create_symlink ${CMAKE_SOURCE_DIR}/src/${OTHER_EXE}/shaders $<TARGET_FILE_DIR:${OTHER_EXE}>/shaders
      )
  endif()
  # link resources directory
  if(EXISTS ${CMAKE_SOURCE_DIR}/src/${OTHER_EXE}/resources)
    add_custom_command(TARGET ${OTHER_EXE} POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E create_symlink ${CMAKE_SOURCE_DIR}/src/${OTHER_EXE}/resources $<TARGET_FILE_DIR:${OTHER_EXE}>/resources
      )
  endif()
endforeach(OTHER_EXE)
