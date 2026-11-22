cmake_minimum_required(VERSION 3.16)
project(CGProject CXX)
set(CMAKE_CXX_STANDARD 17)

# ── Find OpenGL and GLUT ────────────────────────────────────────────────────
find_package(OpenGL REQUIRED)
find_package(GLUT   REQUIRED)

include_directories(
    ${OPENGL_INCLUDE_DIRS}
    ${GLUT_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/include
)

# ── Source files ─────────────────────────────────────────────────────────────
set(SOURCES
    main.cpp
    src/algorithms/LineAlgorithms.cpp
    src/algorithms/CircleAlgorithms.cpp
    src/transformations/Transformations.cpp
    src/clipping/Clipping.cpp
    src/curves/Curves.cpp
)

add_executable(CGProject ${SOURCES})

target_link_libraries(CGProject
    ${OPENGL_LIBRARIES}
    ${GLUT_LIBRARIES}
)

# Platform-specific
if(WIN32)
    target_link_libraries(CGProject opengl32 glu32 freeglut)
elseif(APPLE)
    target_link_libraries(CGProject "-framework OpenGL" "-framework GLUT")
    target_compile_options(CGProject PRIVATE -Wno-deprecated)
endif()

# Copy assets
add_custom_command(TARGET CGProject POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/assets $<TARGET_FILE_DIR:CGProject>/assets
)
