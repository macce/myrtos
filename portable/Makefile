include $(RTOS_ROOT)/config.mk

###############################################################################
# Defines
###############################################################################

INCLUDE_FLAGS := $(foreach dir, \
	$(KERNEL_INCLUDE_DIRS) $(KERNEL_ARCH_INCLUDE_DIRS), -I$(dir))

CFLAGS += $(RTOS_CONFIG_CFLAGS) $(INCLUDE_FLAGS)

###############################################################################
# Objects and Libraries
###############################################################################

OBJECTS := $(foreach object, kernel.o, $(KERNEL_OBJ_DIR)/$(object))

###############################################################################
# Rules
###############################################################################

.PHONY:	all
all:	$(KERNEL_OBJ_DIR) $(OBJECTS)

$(KERNEL_OBJ_DIR):
	mkdir -p $@

$(KERNEL_OBJ_DIR)/%.o:	src/%.c Makefile
	$(CC) $(CFLAGS) -o $@ $<

.PHONY: clean
clean:
	rm -f $(OBJECTS)
