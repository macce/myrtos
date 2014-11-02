###############################################################################
# RTOS Toolchain Makefile Fragment
###############################################################################

# The RTOS_TOOLCHAIN environment variable must be set by config.mk in
# $(RTOS_ROOT)/config/$(RTOS_BUILD_VARIANT).
ifeq ($(RTOS_TOOLCHAIN),)
	$(error "RTOS_TOOLCHAIN undefined!")
endif

# Define default flags for toolchain commands:
CFLAGS :=
ASFLAGS :=
LDFLAGS :=
CPPFLAGS :=
ARFLAGS :=

# Now include specific toolchain support. This may add to or replace the
# default toolchain flags defined above.
include $(RTOS_ROOT)/build/$(RTOS_TOOLCHAIN)/toolchain.mk
# Variables defined by toolchain.mk:
#   CC (compiler executable name)
#   AS (assembler executable name)
#   LD (linker executable name)
#   CPP (C preprocessor executable name)
#   AR (archiver executable name)
