# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.16

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list


# Suppress display of executed commands.
$(VERBOSE).SILENT:


# A target that is always out of date.
cmake_force:

.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/xbj/FuxianCode/fuxian_ClusterChat/ClusterChatServer

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/xbj/FuxianCode/fuxian_ClusterChat/ClusterChatServer/build

# Include any dependencies generated for this target.
include src/server/CMakeFiles/chatServer.dir/depend.make

# Include the progress variables for this target.
include src/server/CMakeFiles/chatServer.dir/progress.make

# Include the compile flags for this target's objects.
include src/server/CMakeFiles/chatServer.dir/flags.make

src/server/CMakeFiles/chatServer.dir/main.cpp.o: src/server/CMakeFiles/chatServer.dir/flags.make
src/server/CMakeFiles/chatServer.dir/main.cpp.o: ../src/server/main.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/xbj/FuxianCode/fuxian_ClusterChat/ClusterChatServer/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object src/server/CMakeFiles/chatServer.dir/main.cpp.o"
	cd /home/xbj/FuxianCode/fuxian_ClusterChat/ClusterChatServer/build/src/server && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/chatServer.dir/main.cpp.o -c /home/xbj/FuxianCode/fuxian_ClusterChat/ClusterChatServer/src/server/main.cpp

src/server/CMakeFiles/chatServer.dir/main.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/chatServer.dir/main.cpp.i"
	cd /home/xbj/FuxianCode/fuxian_ClusterChat/ClusterChatServer/build/src/server && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/xbj/FuxianCode/fuxian_ClusterChat/ClusterChatServer/src/server/main.cpp > CMakeFiles/chatServer.dir/main.cpp.i

src/server/CMakeFiles/chatServer.dir/main.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/chatServer.dir/main.cpp.s"
	cd /home/xbj/FuxianCode/fuxian_ClusterChat/ClusterChatServer/build/src/server && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/xbj/FuxianCode/fuxian_ClusterChat/ClusterChatServer/src/server/main.cpp -o CMakeFiles/chatServer.dir/main.cpp.s

# Object files for target chatServer
chatServer_OBJECTS = \
"CMakeFiles/chatServer.dir/main.cpp.o"

# External object files for target chatServer
chatServer_EXTERNAL_OBJECTS =

../bin/chatServer: src/server/CMakeFiles/chatServer.dir/main.cpp.o
../bin/chatServer: src/server/CMakeFiles/chatServer.dir/build.make
../bin/chatServer: src/server/CMakeFiles/chatServer.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/xbj/FuxianCode/fuxian_ClusterChat/ClusterChatServer/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable ../../../bin/chatServer"
	cd /home/xbj/FuxianCode/fuxian_ClusterChat/ClusterChatServer/build/src/server && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/chatServer.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
src/server/CMakeFiles/chatServer.dir/build: ../bin/chatServer

.PHONY : src/server/CMakeFiles/chatServer.dir/build

src/server/CMakeFiles/chatServer.dir/clean:
	cd /home/xbj/FuxianCode/fuxian_ClusterChat/ClusterChatServer/build/src/server && $(CMAKE_COMMAND) -P CMakeFiles/chatServer.dir/cmake_clean.cmake
.PHONY : src/server/CMakeFiles/chatServer.dir/clean

src/server/CMakeFiles/chatServer.dir/depend:
	cd /home/xbj/FuxianCode/fuxian_ClusterChat/ClusterChatServer/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/xbj/FuxianCode/fuxian_ClusterChat/ClusterChatServer /home/xbj/FuxianCode/fuxian_ClusterChat/ClusterChatServer/src/server /home/xbj/FuxianCode/fuxian_ClusterChat/ClusterChatServer/build /home/xbj/FuxianCode/fuxian_ClusterChat/ClusterChatServer/build/src/server /home/xbj/FuxianCode/fuxian_ClusterChat/ClusterChatServer/build/src/server/CMakeFiles/chatServer.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : src/server/CMakeFiles/chatServer.dir/depend
