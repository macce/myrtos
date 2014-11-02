###############################################################################
# RTOS Build Configuration File
#
# This file is included from all Makefiles in the kernel. It defines a
# number of make variables that are used to find object files,
# libraries, C-headers etc that are necessary during the build.
#
###############################################################################

# The RTOS_ROOT environment variable must be set when building the RTOS
# components. This file must be in the $RTOS_ROOT directory.
ifeq ($(RTOS_ROOT),)
$(error "RTOS_ROOT undefined!")
endif

# Define the architecture and build type:
RTOS_BUILD_VARIANT := CORTEX_M3_DEBUG
ARCH := cortex-m3
DEBUG := yes # yes/no
OPTIMIZE := no # yes/no

# Define the toolchain to use:
RTOS_TOOLCHAIN := codesourcery/arm-2010q1

# Toolchain setup:
include $(RTOS_ROOT)/build/build.mk
# Miscellaneous build support:
include $(RTOS_ROOT)/build/misc.mk
# Make rules common to many components:
include $(RTOS_ROOT)/build/rules.mk

# Static path setup to kernel components:
KERNEL_ARCH_DIR := $(RTOS_ROOT)/$(ARCH)
KERNEL_INCLUDE_DIRS := $(RTOS_ROOT)/include
KERNEL_ARCH_INCLUDE_DIRS := $(KERNEL_ARCH_DIR)/include
KERNEL_OBJ_ROOT := $(RTOS_ROOT)/obj
KERNEL_OBJ_DIR := $(KERNEL_OBJ_ROOT)/$(RTOS_BUILD_VARIANT)
KERNEL_LIB_DIR := $(RTOS_ROOT)/lib
