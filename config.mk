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

# Toolchain setup:
include $(RTOS_ROOT)/build/build.mk

# Miscellaneous build support:
include $(RTOS_ROOT)/build/misc.mk

# Make rules common to many components:
include $(RTOS_ROOT)/build/rules.mk

# RTOS build tools common to all configurations:
BUILD_TOOLS_DIR := $(RTOS_ROOT)/build/tools

# Static path setup to kernel components:
KERNEL_ARCH_DIR := $(RTOS_ROOT)/kernel/$(ARCH)
KERNEL_INCLUDE_DIRS := $(RTOS_ROOT)/kernel/include
KERNEL_ARCH_INCLUDE_DIRS := $(KERNEL_ARCH_DIR)/include
KERNEL_OBJ_ROOT := $(RTOS_ROOT)/kernel/obj
KERNEL_OBJ_DIR := $(KERNEL_OBJ_ROOT)/$(RTOS_BUILD_VARIANT)
KERNEL_LIB_DIR := $(RTOS_ROOT)/kernel/lib
