﻿# CMAKE generated file: DO NOT EDIT!
# Generated by "NMake Makefiles" Generator, CMake Version 3.23

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

.SUFFIXES: .hpux_make_needs_suffix_list

# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

#Suppress display of executed commands.
$(VERBOSE).SILENT:

# A target that is always out of date.
cmake_force:
.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE
NULL=nul
!ENDIF
SHELL = cmd.exe

# The CMake executable.
CMAKE_COMMAND = "C:\Program Files\CMake\bin\cmake.exe"

# The command to remove a file.
RM = "C:\Program Files\CMake\bin\cmake.exe" -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = "C:\Program Files (x86)\Pico\RPI - pico"

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = "C:\Program Files (x86)\Pico\RPI - pico\build"

# Utility rule file for PioasmBuild.

# Include any custom commands dependencies for this target.
include "Function Generator\CMakeFiles\PioasmBuild.dir\compiler_depend.make"

# Include the progress variables for this target.
include "Function Generator\CMakeFiles\PioasmBuild.dir\progress.make"

"Function Generator\CMakeFiles\PioasmBuild": "Function Generator\CMakeFiles\PioasmBuild-complete"
	cd C:\PROGRA~2\Pico\RPI-PI~1\build\FUNCTI~1
	cd C:\PROGRA~2\Pico\RPI-PI~1\build

"Function Generator\CMakeFiles\PioasmBuild-complete": "Function Generator\pioasm\src\PioasmBuild-stamp\PioasmBuild-install"
"Function Generator\CMakeFiles\PioasmBuild-complete": "Function Generator\pioasm\src\PioasmBuild-stamp\PioasmBuild-mkdir"
"Function Generator\CMakeFiles\PioasmBuild-complete": "Function Generator\pioasm\src\PioasmBuild-stamp\PioasmBuild-download"
"Function Generator\CMakeFiles\PioasmBuild-complete": "Function Generator\pioasm\src\PioasmBuild-stamp\PioasmBuild-update"
"Function Generator\CMakeFiles\PioasmBuild-complete": "Function Generator\pioasm\src\PioasmBuild-stamp\PioasmBuild-patch"
"Function Generator\CMakeFiles\PioasmBuild-complete": "Function Generator\pioasm\src\PioasmBuild-stamp\PioasmBuild-configure"
"Function Generator\CMakeFiles\PioasmBuild-complete": "Function Generator\pioasm\src\PioasmBuild-stamp\PioasmBuild-build"
"Function Generator\CMakeFiles\PioasmBuild-complete": "Function Generator\pioasm\src\PioasmBuild-stamp\PioasmBuild-install"
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --blue --bold --progress-dir="C:\Program Files (x86)\Pico\RPI - pico\build\CMakeFiles" --progress-num=$(CMAKE_PROGRESS_1) "Completed 'PioasmBuild'"
	cd C:\PROGRA~2\Pico\RPI-PI~1\build\FUNCTI~1
	echo >nul && "C:\Program Files\CMake\bin\cmake.exe" -E make_directory "C:/Program Files (x86)/Pico/RPI - pico/build/Function Generator/CMakeFiles"
	echo >nul && "C:\Program Files\CMake\bin\cmake.exe" -E touch "C:/Program Files (x86)/Pico/RPI - pico/build/Function Generator/CMakeFiles/PioasmBuild-complete"
	echo >nul && "C:\Program Files\CMake\bin\cmake.exe" -E touch "C:/Program Files (x86)/Pico/RPI - pico/build/Function Generator/pioasm/src/PioasmBuild-stamp/PioasmBuild-done"
	cd C:\PROGRA~2\Pico\RPI-PI~1\build

"Function Generator\pioasm\src\PioasmBuild-stamp\PioasmBuild-build": "Function Generator\pioasm\src\PioasmBuild-stamp\PioasmBuild-configure"
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --blue --bold --progress-dir="C:\Program Files (x86)\Pico\RPI - pico\build\CMakeFiles" --progress-num=$(CMAKE_PROGRESS_2) "Performing build step for 'PioasmBuild'"
	cd C:\PROGRA~2\Pico\RPI-PI~1\build\pioasm
	$(MAKE)
	cd C:\PROGRA~2\Pico\RPI-PI~1\build

"Function Generator\pioasm\src\PioasmBuild-stamp\PioasmBuild-configure": "Function Generator\pioasm\tmp\PioasmBuild-cfgcmd.txt"
"Function Generator\pioasm\src\PioasmBuild-stamp\PioasmBuild-configure": "Function Generator\pioasm\src\PioasmBuild-stamp\PioasmBuild-patch"
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --blue --bold --progress-dir="C:\Program Files (x86)\Pico\RPI - pico\build\CMakeFiles" --progress-num=$(CMAKE_PROGRESS_3) "Performing configure step for 'PioasmBuild'"
	cd C:\PROGRA~2\Pico\RPI-PI~1\build\pioasm
	echo >nul && "C:\Program Files\CMake\bin\cmake.exe" "-GNMake Makefiles" "C:/Program Files (x86)/Pico/pico-sdk/tools/pioasm"
	echo >nul && "C:\Program Files\CMake\bin\cmake.exe" -E touch "C:/Program Files (x86)/Pico/RPI - pico/build/Function Generator/pioasm/src/PioasmBuild-stamp/PioasmBuild-configure"
	cd C:\PROGRA~2\Pico\RPI-PI~1\build

"Function Generator\pioasm\src\PioasmBuild-stamp\PioasmBuild-download": "Function Generator\pioasm\src\PioasmBuild-stamp\PioasmBuild-source_dirinfo.txt"
"Function Generator\pioasm\src\PioasmBuild-stamp\PioasmBuild-download": "Function Generator\pioasm\src\PioasmBuild-stamp\PioasmBuild-mkdir"
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --blue --bold --progress-dir="C:\Program Files (x86)\Pico\RPI - pico\build\CMakeFiles" --progress-num=$(CMAKE_PROGRESS_4) "No download step for 'PioasmBuild'"
	cd C:\PROGRA~2\Pico\RPI-PI~1\build\FUNCTI~1
	echo >nul && "C:\Program Files\CMake\bin\cmake.exe" -E echo_append
	echo >nul && "C:\Program Files\CMake\bin\cmake.exe" -E touch "C:/Program Files (x86)/Pico/RPI - pico/build/Function Generator/pioasm/src/PioasmBuild-stamp/PioasmBuild-download"
	cd C:\PROGRA~2\Pico\RPI-PI~1\build

"Function Generator\pioasm\src\PioasmBuild-stamp\PioasmBuild-install": "Function Generator\pioasm\src\PioasmBuild-stamp\PioasmBuild-build"
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --blue --bold --progress-dir="C:\Program Files (x86)\Pico\RPI - pico\build\CMakeFiles" --progress-num=$(CMAKE_PROGRESS_5) "No install step for 'PioasmBuild'"
	cd C:\PROGRA~2\Pico\RPI-PI~1\build\pioasm
	echo >nul && "C:\Program Files\CMake\bin\cmake.exe" -E echo_append
	echo >nul && "C:\Program Files\CMake\bin\cmake.exe" -E touch "C:/Program Files (x86)/Pico/RPI - pico/build/Function Generator/pioasm/src/PioasmBuild-stamp/PioasmBuild-install"
	cd C:\PROGRA~2\Pico\RPI-PI~1\build

"Function Generator\pioasm\src\PioasmBuild-stamp\PioasmBuild-mkdir":
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --blue --bold --progress-dir="C:\Program Files (x86)\Pico\RPI - pico\build\CMakeFiles" --progress-num=$(CMAKE_PROGRESS_6) "Creating directories for 'PioasmBuild'"
	cd C:\PROGRA~2\Pico\RPI-PI~1\build\FUNCTI~1
	echo >nul && "C:\Program Files\CMake\bin\cmake.exe" -P "C:/Program Files (x86)/Pico/RPI - pico/build/Function Generator/pioasm/tmp/PioasmBuild-mkdirs.cmake"
	echo >nul && "C:\Program Files\CMake\bin\cmake.exe" -E touch "C:/Program Files (x86)/Pico/RPI - pico/build/Function Generator/pioasm/src/PioasmBuild-stamp/PioasmBuild-mkdir"
	cd C:\PROGRA~2\Pico\RPI-PI~1\build

"Function Generator\pioasm\src\PioasmBuild-stamp\PioasmBuild-patch": "Function Generator\pioasm\src\PioasmBuild-stamp\PioasmBuild-update"
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --blue --bold --progress-dir="C:\Program Files (x86)\Pico\RPI - pico\build\CMakeFiles" --progress-num=$(CMAKE_PROGRESS_7) "No patch step for 'PioasmBuild'"
	cd C:\PROGRA~2\Pico\RPI-PI~1\build\FUNCTI~1
	echo >nul && "C:\Program Files\CMake\bin\cmake.exe" -E echo_append
	echo >nul && "C:\Program Files\CMake\bin\cmake.exe" -E touch "C:/Program Files (x86)/Pico/RPI - pico/build/Function Generator/pioasm/src/PioasmBuild-stamp/PioasmBuild-patch"
	cd C:\PROGRA~2\Pico\RPI-PI~1\build

"Function Generator\pioasm\src\PioasmBuild-stamp\PioasmBuild-update": "Function Generator\pioasm\src\PioasmBuild-stamp\PioasmBuild-download"
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --blue --bold --progress-dir="C:\Program Files (x86)\Pico\RPI - pico\build\CMakeFiles" --progress-num=$(CMAKE_PROGRESS_8) "No update step for 'PioasmBuild'"
	cd C:\PROGRA~2\Pico\RPI-PI~1\build\FUNCTI~1
	echo >nul && "C:\Program Files\CMake\bin\cmake.exe" -E echo_append
	echo >nul && "C:\Program Files\CMake\bin\cmake.exe" -E touch "C:/Program Files (x86)/Pico/RPI - pico/build/Function Generator/pioasm/src/PioasmBuild-stamp/PioasmBuild-update"
	cd C:\PROGRA~2\Pico\RPI-PI~1\build

PioasmBuild: "Function Generator\CMakeFiles\PioasmBuild"
PioasmBuild: "Function Generator\CMakeFiles\PioasmBuild-complete"
PioasmBuild: "Function Generator\pioasm\src\PioasmBuild-stamp\PioasmBuild-build"
PioasmBuild: "Function Generator\pioasm\src\PioasmBuild-stamp\PioasmBuild-configure"
PioasmBuild: "Function Generator\pioasm\src\PioasmBuild-stamp\PioasmBuild-download"
PioasmBuild: "Function Generator\pioasm\src\PioasmBuild-stamp\PioasmBuild-install"
PioasmBuild: "Function Generator\pioasm\src\PioasmBuild-stamp\PioasmBuild-mkdir"
PioasmBuild: "Function Generator\pioasm\src\PioasmBuild-stamp\PioasmBuild-patch"
PioasmBuild: "Function Generator\pioasm\src\PioasmBuild-stamp\PioasmBuild-update"
PioasmBuild: "Function Generator\CMakeFiles\PioasmBuild.dir\build.make"
.PHONY : PioasmBuild

# Rule to build all files generated by this target.
"Function Generator\CMakeFiles\PioasmBuild.dir\build": PioasmBuild
.PHONY : "Function Generator\CMakeFiles\PioasmBuild.dir\build"

"Function Generator\CMakeFiles\PioasmBuild.dir\clean":
	cd C:\PROGRA~2\Pico\RPI-PI~1\build\FUNCTI~1
	$(CMAKE_COMMAND) -P CMakeFiles\PioasmBuild.dir\cmake_clean.cmake
	cd C:\PROGRA~2\Pico\RPI-PI~1\build
.PHONY : "Function Generator\CMakeFiles\PioasmBuild.dir\clean"

"Function Generator\CMakeFiles\PioasmBuild.dir\depend":
	$(CMAKE_COMMAND) -E cmake_depends "NMake Makefiles" "C:\Program Files (x86)\Pico\RPI - pico" "C:\Program Files (x86)\Pico\RPI - pico\Function Generator" "C:\Program Files (x86)\Pico\RPI - pico\build" "C:\Program Files (x86)\Pico\RPI - pico\build\Function Generator" "C:\Program Files (x86)\Pico\RPI - pico\build\Function Generator\CMakeFiles\PioasmBuild.dir\DependInfo.cmake" --color=$(COLOR)
.PHONY : "Function Generator\CMakeFiles\PioasmBuild.dir\depend"

