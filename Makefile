AMSLAH_PATH := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))
PROJECT_NAME := $(shell basename $(shell pwd))

CC = arm-none-eabi-gcc -B/usr/bin/arm-none-eabi-
CXX = arm-none-eabi-g++ -B/usr/bin/arm-none-eabi-
LD = arm-none-eabi-g++ -B/usr/bin/arm-none-eabi
AR = arm-none-eabi-ar
OBJCOPY = arm-none-eabi-objcopy
SIZE = arm-none-eabi-size

BUILD_PATH = build
APP = app


ifndef MCU
MCU = $(shell (grep MCU amslah.cfg > /dev/null && sed -n 's/^.*MCU: //p' amslah.cfg 2>/dev/null) || echo "SAMD21J18A")
endif
CFLAGS = -mthumb -DDEBUG -Os -ffunction-sections -mlong-calls -Wall -g3 -fstack-usage
CFLAGS += -c -D__$(MCU)__
CFLAGS += -mfp16-format=ieee

ifneq (,$(findstring SAMD21, $(MCU)))
CFLAGS += -mcpu=cortex-m0plus
CFLAGS += -mfloat-abi=soft -msoft-float -fsingle-precision-constant
LFLAGS += -mfloat-abi=soft -mthumb -msoft-float
LFLAGS += -T"$(AMSLAH_PATH)/core/samd21j18a_flash.ld"
LFLAGS += -Wl,--gc-sections -mcpu=cortex-m0plus
EDBG_FAMILY = samd21
else
CFLAGS += -mcpu=cortex-m4
CFLAGS += -mfloat-abi=hard -mfpu=fpv4-sp-d16 -fsingle-precision-constant  -mfloat-abi=hard
LFLAGS += -mfloat-abi=hard -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16
LFLAGS += -T"$(AMSLAH_PATH)/core/samd51n20a_flash.ld"
LFLAGS += -Wl,--gc-sections -mcpu=cortex-m4 -mtune=cortex-m4
EDBG_FAMILY = samd51
endif

USER_CFLAGS := $(shell sed -n 's/^.*CFLAGS: //p' amslah.cfg 2>/dev/null)
CFLAGS += $(USER_CFLAGS)

CFLAGS += -DCOMPILE_TIME=$(shell date '+%s') -specs=nano.specs -specs=nosys.specs $(ADDITIONAL_CFLAGS) $(CF)

LFLAGS += -lm -specs=nano.specs -specs=nosys.specs
LFLAGS += -Wl,-Map="$(BUILD_PATH)/memory.map",--cref
LFLAGS += -Wl,--undefined=uxTopUsedPriority

RAMSIZE = $(shell (grep RAMSIZE amslah.cfg > /dev/null && sed -n 's/^.*RAMSIZE: //p' amslah.cfg 2>/dev/null) || echo "")
ifneq ($(RAMSIZE),)
LFLAGS += -Xlinker --defsym=SAMD_RAM_SIZE=$(RAMSIZE)
endif


SHELL:=/bin/bash
LINKOBJS := $(shell realpath $(shell sed -n 's/^.*LINKOBJS: //p' amslah.cfg 2>/dev/null) 2>/dev/null)

LIBDIRS := $(shell realpath $(shell sed -n 's/^.*LIBS: //p' amslah.cfg 2>/dev/null) 2>/dev/null)

INCLUDE = -I"$(AMSLAH_PATH)/core" -I"$(AMSLAH_PATH)/config" -I"$(AMSLAH_PATH)/freertos/include" -I"$(AMSLAH_PATH)/freertos/portable_$(EDBG_FAMILY)" -I"$(AMSLAH_PATH)/extra" -I"."
INCLUDE += $(foreach LIBDIR,$(LIBDIRS),-I"$(LIBDIR)")
INCLUDE += $(foreach LIBDIR,$(shell ls -d */),-I"$(LIBDIR)")

EXCLUDE := $(shell sed -n 's/^.*EXCLUDE: //p' amslah.cfg 2>/dev/null)

ifndef IGNORE_HOOK
HOOKS_CMD_RAW := $(shell sed -n 's/^HOOKS: //p' amslah.cfg 2>&1)
HOOK_VARS := $(shell echo '$(HOOKS_CMD_RAW)' | grep -o '__[^_]*__' | tr -d '_' | sort | uniq)

replace_vars = $(if $(1),$(foreach var,$(1),$(subst __$(var)__,"${$(var)}",$(2))),$(2))
HOOKS_CMD := $(call replace_vars,$(HOOK_VARS),$(HOOKS_CMD_RAW))
$(info HOOKS_CMD: `$(HOOKS_CMD_RAW)` --> `$(HOOKS_CMD)`)

HOOK_VAL := $(shell $(HOOKS_CMD) &> hook_output; echo $$?)
ifneq ($(HOOK_VAL), 0)
$(error "The hook failed! See hook_output.")
endif
endif

ifneq ($(NAME_SUFFIX),)
_NAME_SUFFIX = _$(NAME_SUFFIX)
endif

HOOK_POST := $(shell sed -n 's/^.*HOOKS_POST: //p' amslah.cfg)
ifneq ($(HOOK_POST),)
	HOOK_POST := $(HOOK_POST) build/$(PROJECT_NAME)$(_NAME_SUFFIX).bin
endif

TEXT := $(shell cat hook_output)

CONFIGS = $(AMSLAH_PATH)/config/FreeRTOSConfig.h
CONFIGS += $(AMSLAH_PATH)/config/amslah_config.h
CONFIGS += user_amslah_config.h

DIRS = .
DIRS += $(LIBDIRS)
DIRS += $(shell ls -d */ | grep -v ^build| grep -v ^test)
DIRS := $(filter-out $(EXCLUDE)/,$(DIRS))

CPPSRC = $(foreach DIR,$(DIRS),$(wildcard $(DIR)/*.cpp)) 
CSRC = $(foreach DIR,$(DIRS),$(wildcard $(DIR)/*.c)) 
HSRC = $(foreach DIR,$(DIRS),$(wildcard $(DIR)/*.h)) 

CPPSRC += $(AMSLAH_PATH)/extra/mutex.cpp

ifneq (,$(findstring SAMD21, $(MCU)))
CSRC += $(AMSLAH_PATH)/core/startup_samd21.c
CSRC += $(AMSLAH_PATH)/core/mtb.c
else
CSRC += $(AMSLAH_PATH)/core/startup_samd51.c
endif
CSRC += $(AMSLAH_PATH)/core/gpio.c
CSRC += $(AMSLAH_PATH)/core/util.c
CSRC += $(AMSLAH_PATH)/core/uart.c
CSRC += $(AMSLAH_PATH)/core/eeprom.c
CSRC += $(AMSLAH_PATH)/core/spi.c
CSRC += $(AMSLAH_PATH)/core/i2c.c
CSRC += $(AMSLAH_PATH)/core/pwm.c
CSRC += $(AMSLAH_PATH)/core/adc.c
CSRC += $(AMSLAH_PATH)/core/dac.c
CSRC += $(AMSLAH_PATH)/core/dma_base.c
CSRC += $(AMSLAH_PATH)/core/eic.c
CSRC += $(AMSLAH_PATH)/core/watchdog.c
CSRC += $(AMSLAH_PATH)/core/sercoms.c
CSRC += $(AMSLAH_PATH)/core/serial.c
CSRC += $(AMSLAH_PATH)/core/printf.c
CSRC += $(AMSLAH_PATH)/freertos/croutine.c
CSRC += $(AMSLAH_PATH)/freertos/event_groups.c
CSRC += $(AMSLAH_PATH)/freertos/list.c
CSRC += $(AMSLAH_PATH)/freertos/queue.c
CSRC += $(AMSLAH_PATH)/freertos/tasks.c
CSRC += $(AMSLAH_PATH)/freertos/stream_buffer.c
CSRC += $(AMSLAH_PATH)/freertos/timers.c
CSRC += $(AMSLAH_PATH)/freertos/FreeRTOS-openocd.c
HEAP_VAL = $(shell sed -n 's/^.*HEAP: //p' amslah.cfg 2>/dev/null || echo 1)
ifeq ($(HEAP_VAL), 4)
CSRC += $(AMSLAH_PATH)/freertos/heap_4.c
else
CSRC += $(AMSLAH_PATH)/freertos/heap_1.c
endif

CSRC += $(AMSLAH_PATH)/freertos/portable_$(EDBG_FAMILY)/port.c
ifeq ($(MAKECMDGOALS), test)
HSRC += test/testheader.h
$(eval CPPSRC += test/$(TESTFILE).cpp)
$(eval CPPSRC = $(filter-out %/main.cpp,$(CPPSRC)))
$(eval CSRC = $(filter-out %/main.c,$(CSRC)))
endif
#$(info $$var is [${INCLUDE}])

ifneq ($(SERIAL),)
ICE_SERIAL = -s $(SERIAL)
endif

CPPOBJ := $(CPPSRC:%.cpp=%.o)
COBJ := $(CSRC:%.c=%.o)

OBJ := $(COBJ) $(CPPOBJ)
BUILTOBJ := $(addprefix $(BUILD_PATH)/,$(OBJ))

$(APP): $(BUILTOBJ)
	$(LD) $(LFLAGS) -o build/$(PROJECT_NAME)$(_NAME_SUFFIX).elf $(BUILTOBJ) $(LINKOBJS)
	$(OBJCOPY) --strip-unneeded -O binary build/$(PROJECT_NAME)$(_NAME_SUFFIX).elf build/$(PROJECT_NAME)$(_NAME_SUFFIX).bin
	cat hook_output
	printf "INCLUDE: ${INCLUDE} \nCSRC: ${CSRC} \nCPPSRC: ${CPPSRC}" > build/filelist 
	arm-none-eabi-size "build/$(PROJECT_NAME)$(_NAME_SUFFIX).elf"
	$(HOOK_POST)

$(BUILD_PATH)/%.o: %.cpp $(CONFIGS) $(HSRC)
	mkdir -p $(@D)
	$(CXX) $(INCLUDE) $(CFLAGS) -o "$@" -c "$<"

$(BUILD_PATH)/%.o: %.c $(CONFIGS) $(HSRC)
	mkdir -p $(@D)
	$(CC) $(INCLUDE) -x c $(CFLAGS) -o "$@" -c "$<"

clean:
	rm -rf build

u: $(APP)
	edbg -bpv -t $(EDBG_FAMILY) -f build/$(PROJECT_NAME)$(_NAME_SUFFIX).bin $(ICE_SERIAL)

r: $(APP)
	$(RUN)

ohp:
	$(shell basename $(shell pwd))

test: $(APP) 
	edbg -bpv -t $(EDBG_FAMILY) -f build/amslah.bin 
ocd:
	cd $(AMSLAH_PATH); openocd

gdb:
	arm-none-eabi-gdb build/$(PROJECT_NAME)$(_NAME_SUFFIX).elf -ex "target extended-remote :3333"
