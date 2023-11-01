cmake_minimum_required(VERSION 3.24)

project(roboflex_audio_sdl VERSION 0.1.5 DESCRIPTION "roboflex audio sensor using SDL2")

# specify the C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)


# -------------------- 
# Resolve dependencies

include(FetchContent)

find_package(SDL2 REQUIRED)               

# download and build roboflex_core
FetchContent_Declare(roboflex_core
    GIT_REPOSITORY https://github.com/flexrobotics/roboflex.git
    GIT_TAG        main
)
set(BUILD_ROBOFLEX_PYTHON_EXT OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(roboflex_core)

# # download and build roboflex_transport_zmq
# ONLY NEEDED IF YOU COMPILE THE EXAMPLES
# TODO: separate out into a separate CMakeLists.txt in examples
# FetchContent_Declare(roboflex_transport_zmq
#     GIT_REPOSITORY https://github.com/flexrobotics/roboflex_transport_zmq.git
#     GIT_TAG        main
# )
# FetchContent_MakeAvailable(roboflex_transport_zmq)

# -------------------- 
# Define the library

add_library(roboflex_audio_sdl STATIC
    src/audio_sdl.cpp
    include/roboflex_audio_sdl/audio_sdl.h
)

# Set some properties on our library
set_property(TARGET roboflex_audio_sdl PROPERTY 
    POSITION_INDEPENDENT_CODE ON
)

# Include directories when we compile our library
target_include_directories(roboflex_audio_sdl PUBLIC 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include> 
    $<INSTALL_INTERFACE:include>
    ${SDL2_INCLUDE_DIRS}
)

# Link against the necessary libraries
target_link_libraries(roboflex_audio_sdl PUBLIC 
    roboflex_core
    #SDL2
    ${SDL2_LIBRARIES}

    # Unfortunately, we need to link against all these libraries
    # because they are used by roboflex_core, and cmake doesn't
    # seem to be transitivily doing it's thing. Urgh, someone help.
    flatbuffers_util 
    xtensor 
    xsimd 
    xtl
    Eigen3::Eigen
)


# -------------------- 
# Examples

# basic_0 example
# add_executable(stream_from_mic_cpp examples/stream_from_mic_cpp.cpp)
# target_link_libraries(stream_from_mic_cpp PRIVATE 
#     roboflex_core 
#     roboflex_audio_sdl
#     roboflex_transport_zmq
# )


# -------------------- 
# install

# If you need to install the roboflex_audio_sdl library
install(TARGETS roboflex_audio_sdl 
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include
)

install(DIRECTORY include/roboflex_audio_sdl
    DESTINATION include
)


# --------------------
# build python bindings

add_subdirectory(python)
