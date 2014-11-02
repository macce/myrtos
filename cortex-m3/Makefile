include $(RTOS_ROOT)/config.mk

###############################################################################
# Defines
###############################################################################

INCLUDE_FLAGS := $(foreach dir, \
	$(KERNEL_INCLUDE_DIRS) $(KERNEL_ARCH_INCLUDE_DIRS), -I$(dir))

CFLAGS += $(RTOS_CONFIG_CFLAGS) $(INCLUDE_FLAGS)
ASFLAGS += $(RTOS_CONFIG_ASFLAGS) $(INCLUDE_FLAGS)

###############################################################################
# Objects and Libraries
###############################################################################

OBJECTS := $(foreach object, \
	kernel_arch.o kernel_arch_asm.o exceptions.o syscalls.o, \
	$(KERNEL_OBJ_DIR)/$(object))

###############################################################################
# Rules
###############################################################################

.PHONY:	all
all:	$(KERNEL_OBJ_DIR) $(OBJECTS)

$(KERNEL_OBJ_DIR):
	mkdir -p $@

$(KERNEL_OBJ_DIR)/%.o:	src/%.c Makefile
	$(CC) $(CFLAGS) -o $@ $<

$(KERNEL_OBJ_DIR)/%.o:	src/%.S Makefile
	$(AS) $(ASFLAGS) -o $@ $<

.PHONY: clean
clean:
	rm -f $(OBJECTS)
