###############################################################################
# Main Kernel Makefile
#
# This makefile calls all relevant sub-component makefiles to build the
# object files necessary for the specified architecture and build type.
#
# It requires that all sub-components put the generated object files in a
# directory named <sub-component-dir>/obj/$(BUILD_TYPE). An
# example of this is the cortex-m3 sub-component:
#
# .
# |-- Makefile
# |-- include
# |-- obj
# |   `-- DEBUG
# |       |-- exceptions.o
# |       `-- kernel_arch_asm.o
# `-- src
#     |-- exceptions.S
#     `-- kernel_arch_asm.S
#
###############################################################################

include config.mk

###############################################################################
# Defines
###############################################################################

# SUBCOMP_DIRS is a list of all sub-components to build. Currently, we only
# build one architecture at a time.
SUBCOMP_DIRS := portable $(KERNEL_ARCH_DIR)

# LIBRARIES points out the built kernel library.
LIBRARIES := $(KERNEL_LIB_DIR)/libkernel.a

# Include files to install.
INCLUDES := $(foreach inc, kernel.h rtos_types.h, include/$(inc))

# Place to install kernel stuff.
LIB_INSTALL_PATH := $(KERNEL_INSTALL_ROOT)/lib/$(RTOS_BUILD_VARIANT)
INCLUDE_INSTALL_PATH := $(KERNEL_INSTALL_ROOT)/include

###############################################################################
# Rules
###############################################################################

.PHONY:	all
all:	$(LIBRARIES)

$(KERNEL_LIB_DIR):
	mkdir -p $@

.PHONY:	$(LIBRARIES)
$(LIBRARIES): $(KERNEL_LIB_DIR)
	@for subcomp_dir in $(SUBCOMP_DIRS); do \
		make -C $$subcomp_dir all; \
	done
	$(AR) $(ARFLAGS) $@ $(KERNEL_OBJ_DIR)/*

.PHONY:	install
install:	$(LIBRARIES)
	install -d $(LIB_INSTALL_PATH) $(INCLUDE_INSTALL_PATH)
	for lib in $(LIBRARIES); do \
		install -m 644 $$lib $(LIB_INSTALL_PATH); \
	done
	for inc in $(INCLUDES); do \
		install -m 644 $$inc $(INCLUDE_INSTALL_PATH); \
	done

.PHONY:	clean
clean:
	rm -rf $(KERNEL_OBJ_DIR) $(KERNEL_LIB_DIR)

.PHONY:	distclean
distclean:
	rm -rf $(KERNEL_OBJ_ROOT) $(KERNEL_LIB_ROOT)
