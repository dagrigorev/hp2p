# CMakeList.txt : CMake project for hp2p, include source and define
# project specific logic here.
#

# Add source to this project's executable.
add_executable (hp2p "main.cpp"  "peer.cpp")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET hp2p PROPERTY CXX_STANDARD 20)
endif()

# TODO: Add tests and install targets if needed.
