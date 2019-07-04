AMSLAH_PATH := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))

CC = arm-none-eabi-gcc -B/usr/bin/arm-none-eabi-
CXX = arm-none-eabi-g++ -B/usr/bin/arm-none-eabi-
LD = arm-none-eabi-g++ -B/usr/bin/arm-none-eabi-
AR = arm-none-eabi-ar
OBJCOPY = arm-none-eabi-objcopy
SIZE = arm-none-eabi-size


CFLAGS = -mthumb -DDEBUG -O2 -ffunction-sections -mlong-calls -Wall -g3
CFLAGS += -c -D__SAMD21J18A__ -mcpu=cortex-m0plus
CFLAGS += -mfloat-abi=soft -msoft-float -fsingle-precision-constant

LFLAGS = -T"$(AMSLAH_PATH)/core/samd21j18a_flash.ld"
LFLAGS += -Wl,--gc-sections -mcpu=cortex-m0plus  -lm -specs=nano.specs -specs=nosys.specs
LFLAGS += -mfloat-abi=soft -mthumb -msoft-float

SHELL:=/bin/bash
LIBDIRS := $(shell realpath $(shell sed -n 's/^.*LIBS: //p' amslah.cfg 2>/dev/null) 2>/dev/null)

INCLUDE = -I"$(AMSLAH_PATH)/core" -I"$(AMSLAH_PATH)/config" -I"$(AMSLAH_PATH)/freertos/include" -I"$(AMSLAH_PATH)/freertos/portable" -I"$(AMSLAH_PATH)/extra" -I"."
INCLUDE += $(foreach LIBDIR,$(LIBDIRS),-I"$(LIBDIR)")
INCLUDE += $(foreach LIBDIR,$(shell ls -d */),-I"$(LIBDIR)")

HOOK_VAL := $(shell $(shell sed -n 's/^.*HOOKS: //p' amslah.cfg) &> hook_output; echo $$?)
ifneq ($(HOOK_VAL), 0)
$(error "The hook failed! See hook_output.")
endif

BUILD_PATH = build
APP = app

CONFIGS = $(AMSLAH_PATH)/config/FreeRTOSConfig.h
CONFIGS += $(AMSLAH_PATH)/config/amslah_config.h
CONFIGS += user_amslah_config.h

DIRS = .
DIRS += $(LIBDIRS)
DIRS += $(shell ls -d */ | grep -v ^build| grep -v ^test)

CPPSRC = $(foreach DIR,$(DIRS),$(wildcard $(DIR)/*.cpp)) 
CSRC = $(foreach DIR,$(DIRS),$(wildcard $(DIR)/*.c)) 
HSRC = $(foreach DIR,$(DIRS),$(wildcard $(DIR)/*.h)) 
HSRC += test/testheader.h

CSRC += $(AMSLAH_PATH)/core/startup_samd21.c
CSRC += $(AMSLAH_PATH)/core/gpio.c
CSRC += $(AMSLAH_PATH)/core/util.c
CSRC += $(AMSLAH_PATH)/core/uart.c
CSRC += $(AMSLAH_PATH)/core/spi.c
CSRC += $(AMSLAH_PATH)/core/pwm.c
CSRC += $(AMSLAH_PATH)/core/adc.c
CSRC += $(AMSLAH_PATH)/core/dac.c
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
CSRC += $(AMSLAH_PATH)/freertos/heap_1.c
CSRC += $(AMSLAH_PATH)/freertos/portable/port.c
test:
	$(eval CPPSRC += test/test.cpp)
	$(eval CPPSRC = $(filter-out ./main.cpp,$(CPPSRC)))


CPPOBJ := $(CPPSRC:%.cpp=%.o)
COBJ := $(CSRC:%.c=%.o)

OBJ := $(COBJ) $(CPPOBJ)
BUILTOBJ := $(addprefix $(BUILD_PATH)/,$(OBJ))

$(APP): $(BUILTOBJ) 
	$(LD) -o build/amslah.elf $(BUILTOBJ) $(LFLAGS)
	$(OBJCOPY) --strip-unneeded -O binary build/amslah.elf build/amslah.bin
	arm-none-eabi-size "build/amslah.elf"

$(BUILD_PATH)/%.o: %.cpp $(CONFIGS) $(HSRC)
	mkdir -p $(@D)
	$(CXX) $(INCLUDE) $(CFLAGS) -o "$@" -c "$<"

$(BUILD_PATH)/%.o: %.c $(CONFIGS)
	mkdir -p $(@D)
	$(CC) $(INCLUDE) -x c $(CFLAGS) -o "$@" -c "$<"

TARGET = lasagnaSim

clean:
	rm -rf build

u: $(APP)
	edbg -bpv -t samd21 -f build/amslah.bin



test: $(APP) 
	edbg -bpv -t samd21 -f build/amslah.bin
ocd:
	cd $(AMSLAH_PATH); openocd

gdb:
	arm-none-eabi-gdb build/amslah.elf -ex "target extended-remote :3333"
