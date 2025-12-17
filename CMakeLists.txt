cmake_minimum_required(VERSION 4.0)

project(Client CXX)

set(CMAKE_CXX_STANDARD 23)

#Find the required SDL2 libraries
find_package(SDL2 REQUIRED)
find_package(SDL2_image REQUIRED)
find_package(SDL2_mixer REQUIRED)

#Include the SDL2 header files
include_directories(
        ${SDL2_INCLUDE_DIRS}
        ${SDL2_IMAGE_INCLUDE_DIRS}
        ${SDL2_MIXER_INCLUDE_DIRS}
)

file(GLOB_RECURSE SOURCE_FILES "src/*.h" "src/*.cpp")

add_executable(${PROJECT_NAME} ${SOURCE_FILES})

target_link_libraries(${PROJECT_NAME}
        ${SDL2_LIBRARIES}
        SDL2_image::SDL2_image
        SDL2_mixer::SDL2_mixer
)

target_include_directories(${PROJECT_NAME}
        PRIVATE ${SDL2_INCLUDE_DIRS}
        ${SDL2_IMAGE_INCLUDE_DIRS}
        ${SDL2_MIXER_INCLUDE_DIRS}
)

add_custom_target(copy_assets
COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_LIST_DIR}/copy_assets.cmake
)
add_dependencies(${PROJECT_NAME} copy_assets)


