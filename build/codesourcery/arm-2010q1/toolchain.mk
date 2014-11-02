###############################################################################
# RTOS Specific Toolchain Makefile Fragment
###############################################################################

CC := arm-none-eabi-gcc
AS := arm-none-eabi-gcc
LD := arm-none-eabi-ld
CPP := arm-none-eabi-cpp
AR := arm-none-eabi-ar

ifeq ($(strip $(OPTIMIZE)), yes)
OPTIMIZATION_FLAGS := -O3
else ifeq ($(strip $(OPTIMIZE)), no)
OPTIMIZATION_FLAGS := -O0
else
$(error No optimization level selected!)
endif

ifeq ($(strip $(DEBUG)), yes)
DEBUG_FLAGS := -g
else ifeq ($(strip $(DEBUG)), no)
DEBUG_FLAGS :=
else
$(error No debug level selected!)
endif

CFLAGS += -c $(OPTIMIZATION_FLAGS) $(DEBUG_FLAGS) -fno-common \
	-mcpu=cortex-m3 -mthumb
ASFLAGS += -c -Wa,-mfpu=softfpa,-mcpu=cortex-m3,-mthumb
CPPFLAGS += -P
ARFLAGS += r
