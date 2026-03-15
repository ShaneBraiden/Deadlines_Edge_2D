cmake_minimum_required(VERSION 3.20)
project(DeadlinesEdge VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- Dependencies (installed via vcpkg) ---
find_package(SFML COMPONENTS system window graphics REQUIRED)
find_package(box2d CONFIG REQUIRED)

# --- Gather all source files ---
file(GLOB_RECURSE SOURCES "src/*.cpp")

# --- Create executable ---
add_executable(${PROJECT_NAME} ${SOURCES})

# --- Include directories ---
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/include)

# --- Link libraries ---
target_link_libraries(${PROJECT_NAME} PRIVATE
    sfml-system sfml-window sfml-graphics
    box2d::box2d
)
