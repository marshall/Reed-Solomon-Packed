TARGET:=rs_test.elf

OPTLVL:=0
DBG:=-g -Wl,-Map,rs_test.map

MCU_FLAGS:=-mcpu=cortex-m4 -mthumb -mlittle-endian -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb-interwork

DEFINES:= -D'STM32F40_41xxx'
DEFINES+= -D'HSE_VALUE=((uint32_t)24000000)'
DEFINES+= -D'USE_STDPERIPH_DRIVER'

ifeq (,$(TOOLCHAIN_ROOT))
TOOLCHAIN_ROOT := /usr/lib
endif
TOOLCHAIN_ROOT:=$(abspath $(TOOLCHAIN_ROOT))
TOOLCHAIN_BIN:=$(TOOLCHAIN_ROOT)/bin
TOOLCHAIN_PREFIX:=arm-none-eabi

HDWARE:=$(abspath $(CURDIR)/hardware)
LIBS:=$(abspath $(CURDIR)/Libraries)
STM:=$(LIBS)/STM32F4xx_StdPeriph_Driver
RS_SRC:=$(abspath $(CURDIR)/src)

BUILD_DIR:=$(abspath $(CURDIR)/build)

INCLUDE:=-I$(CURDIR)/include
INCLUDE+=-I$(HDWARE)/include
INCLUDE+=-I$(LIBS)/CMSIS/Device/ST/STM32F4xx/Include
INCLUDE+=-I$(LIBS)/CMSIS/Include
INCLUDE+=-I$(LIBS)/syscall
INCLUDE+=-I$(STM)/inc

OBJS:=$(BUILD_DIR)/rs_test.o
OBJS+=$(BUILD_DIR)/alpha_to.o
OBJS+=$(BUILD_DIR)/genpoly.o
OBJS+=$(BUILD_DIR)/index_of.o

OBJS+=$(BUILD_DIR)/startup_stm32f40_41xxx.o
OBJS+=$(BUILD_DIR)/stm32f4xx_it.o
OBJS+=$(BUILD_DIR)/system_stm32f4xx.o

OBJS+=$(BUILD_DIR)/syscall.o

OBJS+=$(BUILD_DIR)/stm32f4xx_gpio.o
OBJS+=$(BUILD_DIR)/stm32f4xx_rcc.o
OBJS+=$(BUILD_DIR)/stm32f4xx_usart.o
OBJS+=$(BUILD_DIR)/stm32f4xx_rng.o

# don't count on having the tools in the PATH...
CC := $(TOOLCHAIN_BIN)/$(TOOLCHAIN_PREFIX)-gcc
LD := $(TOOLCHAIN_BIN)/$(TOOLCHAIN_PREFIX)-gcc
OBJCOPY := $(TOOLCHAIN_BIN)/$(TOOLCHAIN_PREFIX)-objcopy
AS := $(TOOLCHAIN_BIN)/$(TOOLCHAIN_PREFIX)-as
AR := $(TOOLCHAIN_BIN)/$(TOOLCHAIN_PREFIX)-ar
GDB := $(TOOLCHAIN_BIN)/$(TOOLCHAIN_PREFIX)-gdb

COMMONFLAGS:=-O$(OPTLVL) $(DBG) -Wall $(MCU_FLAGS)
CFLAGS:=$(COMMONFLAGS) $(DEFINES)
LDFLAGS:=$(COMMONFLAGS) -fno-exceptions -T$(CURDIR)/STM32F4xx_FLASH.ld

.PHONY: all debug clean

all: build | $(TARGET)

build:
	mkdir build

$(BUILD_DIR)/startup_stm32f40_41xxx.o: $(HDWARE)/startup_stm32f40_41xxx.s
	@$(AS) -c $< -o $@

$(BUILD_DIR)/rs_test.o: $(CURDIR)/rs_test.c
	@$(CC) $(CFLAGS) $(INCLUDE) -o $@ -c $<

$(BUILD_DIR)/alpha_to.o: $(RS_SRC)/alpha_to.c
	@$(CC) $(CFLAGS) $(INCLUDE) -o $@ -c $<

$(BUILD_DIR)/genpoly.o: $(RS_SRC)/genpoly.c
	@$(CC) $(CFLAGS) $(INCLUDE) -o $@ -c $<

$(BUILD_DIR)/index_of.o: $(RS_SRC)/index_of.c
	@$(CC) $(CFLAGS) $(INCLUDE) -o $@ -c $<

$(BUILD_DIR)/stm32f4xx_it.o: $(HDWARE)/stm32f4xx_it.c
	@$(CC) $(CFLAGS) $(INCLUDE) -o $@ -c $<

$(BUILD_DIR)/system_stm32f4xx.o: $(HDWARE)/system_stm32f4xx.c
	@$(CC) $(CFLAGS) $(INCLUDE) -o $@ -c $<

$(BUILD_DIR)/syscall.o: $(LIBS)/syscall/syscall.c
	@$(CC) $(CFLAGS) $(INCLUDE) -o $@ -c $<

$(BUILD_DIR)/stm32f4xx_gpio.o: $(STM)/src/stm32f4xx_gpio.c
	@$(CC) $(CFLAGS) $(INCLUDE) -o $@ -c $<

$(BUILD_DIR)/stm32f4xx_rcc.o: $(STM)/src/stm32f4xx_rcc.c
	@$(CC) $(CFLAGS) $(INCLUDE) -o $@ -c $<

$(BUILD_DIR)/stm32f4xx_rng.o: $(STM)/src/stm32f4xx_rng.c
	@$(CC) $(CFLAGS) $(INCLUDE) -o $@ -c $<

$(BUILD_DIR)/stm32f4xx_usart.o: $(STM)/src/stm32f4xx_usart.c
	@$(CC) $(CFLAGS) $(INCLUDE) -o $@ -c $<

$(TARGET): $(OBJS)
	@$(LD) $(LDFLAGS) -o $@ $^

clean:
	@echo [RM] BUILD DIR
	@rm -f $(BUILD_DIR)/*
	@echo [RM] .ELF file
	@rm -f $(TARGET)

debug:
	@$(GDB) -tui $(TARGET)