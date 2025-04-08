# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.8)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
  cmake_policy(SET CMP0079 NEW)
endif()

project ("hp2p")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_definitions(-DNOMINMAX)
add_definitions(-_WINSOCK_DEPRECATED_NO_WARNINGS)

# Optional: Multi-core build flags for MSVC
if (MSVC)
    add_compile_options(/MP)
endif()

# FlatBuffers include dir (can also be system-installed)
include_directories(${CMAKE_SOURCE_DIR}/flatbuffers)
include_directories(${CMAKE_SOURCE_DIR}/include)

# Include sub-projects.
add_subdirectory ("src")

if(WIN32)
  target_link_libraries(hp2p wsock32 ws2_32)
endif()