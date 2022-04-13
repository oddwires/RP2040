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

# Utility rule file for pio_rotary_encoder_pio_blink_pio_h.

# Include any custom commands dependencies for this target.
include "Function Generator\CMakeFiles\pio_rotary_encoder_pio_blink_pio_h.dir\compiler_depend.make"

# Include the progress variables for this target.
include "Function Generator\CMakeFiles\pio_rotary_encoder_pio_blink_pio_h.dir\progress.make"

"Function Generator\CMakeFiles\pio_rotary_encoder_pio_blink_pio_h": "Function Generator\pio_blink.pio.h"
	cd C:\PROGRA~2\Pico\RPI-PI~1\build\FUNCTI~1
	cd C:\PROGRA~2\Pico\RPI-PI~1\build

"Function Generator\pio_blink.pio.h": "..\Function Generator\pio_blink.pio"
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --blue --bold --progress-dir="C:\Program Files (x86)\Pico\RPI - pico\build\CMakeFiles" --progress-num=$(CMAKE_PROGRESS_1) "Generating pio_blink.pio.h"
	cd C:\PROGRA~2\Pico\RPI-PI~1\build\FUNCTI~1
	..\pioasm\pioasm.exe -o c-sdk "C:/Program Files (x86)/Pico/RPI - pico/Function Generator/pio_blink.pio" "C:/Program Files (x86)/Pico/RPI - pico/build/Function Generator/pio_blink.pio.h"
	cd C:\PROGRA~2\Pico\RPI-PI~1\build

pio_rotary_encoder_pio_blink_pio_h: "Function Generator\CMakeFiles\pio_rotary_encoder_pio_blink_pio_h"
pio_rotary_encoder_pio_blink_pio_h: "Function Generator\pio_blink.pio.h"
pio_rotary_encoder_pio_blink_pio_h: "Function Generator\CMakeFiles\pio_rotary_encoder_pio_blink_pio_h.dir\build.make"
.PHONY : pio_rotary_encoder_pio_blink_pio_h

# Rule to build all files generated by this target.
"Function Generator\CMakeFiles\pio_rotary_encoder_pio_blink_pio_h.dir\build": pio_rotary_encoder_pio_blink_pio_h
.PHONY : "Function Generator\CMakeFiles\pio_rotary_encoder_pio_blink_pio_h.dir\build"

"Function Generator\CMakeFiles\pio_rotary_encoder_pio_blink_pio_h.dir\clean":
	cd C:\PROGRA~2\Pico\RPI-PI~1\build\FUNCTI~1
	$(CMAKE_COMMAND) -P CMakeFiles\pio_rotary_encoder_pio_blink_pio_h.dir\cmake_clean.cmake
	cd C:\PROGRA~2\Pico\RPI-PI~1\build
.PHONY : "Function Generator\CMakeFiles\pio_rotary_encoder_pio_blink_pio_h.dir\clean"

"Function Generator\CMakeFiles\pio_rotary_encoder_pio_blink_pio_h.dir\depend":
	$(CMAKE_COMMAND) -E cmake_depends "NMake Makefiles" "C:\Program Files (x86)\Pico\RPI - pico" "C:\Program Files (x86)\Pico\RPI - pico\Function Generator" "C:\Program Files (x86)\Pico\RPI - pico\build" "C:\Program Files (x86)\Pico\RPI - pico\build\Function Generator" "C:\Program Files (x86)\Pico\RPI - pico\build\Function Generator\CMakeFiles\pio_rotary_encoder_pio_blink_pio_h.dir\DependInfo.cmake" --color=$(COLOR)
.PHONY : "Function Generator\CMakeFiles\pio_rotary_encoder_pio_blink_pio_h.dir\depend"
