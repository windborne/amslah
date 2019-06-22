AMSLAH_PATH = $(shell readlink -f Makefile | xargs dirname)
$(info ${AMSLAH_PATH})

CC = arm-none-eabi-gcc -B/usr/bin/arm-none-eabi-
CXX = arm-none-eabi-g++ -B/usr/bin/arm-none-eabi-
LD = arm-none-eabi-gcc -B/usr/bin/arm-none-eabi-
AR = arm-none-eabi-ar
OBJCOPY = arm-none-eabi-objcopy
SIZE = arm-none-eabi-size


CFLAGS = -x c -mthumb -DDEBUG -O2 -ffunction-sections -mlong-calls -Wall -g3
CFLAGS += -c -D__SAMD21J18A__ -mcpu=cortex-m0plus

LFLAGS = -T"$(AMSLAH_PATH)/core/samd21j18a_flash.ld"
LFLAGS += -Wl,--gc-sections -mcpu=cortex-m0plus  -lm

INCLUDE = -I"$(AMSLAH_PATH)/core" -I"$(AMSLAH_PATH)/config" -I"$(AMSLAH_PATH)/freertos/include" -I"$(AMSLAH_PATH)/freertos/portable"

BUILD_PATH = build
APP = app

CSRC = $(wildcard *.c)
CSRC += $(AMSLAH_PATH)/core/startup_samd21.c
CSRC += $(AMSLAH_PATH)/core/gpio.c
CSRC += $(AMSLAH_PATH)/core/util.c
CSRC += $(AMSLAH_PATH)/core/uart.c
CSRC += $(AMSLAH_PATH)/core/spi.c
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

CPPOBJ := $(CPPSRC:%.cpp=%.o)
COBJ := $(CSRC:%.c=%.o)

OBJ := $(COBJ) $(CPPOBJ)
BUILTOBJ := $(addprefix $(BUILD_PATH)/,$(OBJ))

$(APP): $(BUILTOBJ)
	$(LD) -o build/amslah.elf $(BUILTOBJ) $(LFLAGS)
	$(OBJCOPY) --strip-unneeded -O binary build/amslah.elf build/amslah.bin
	arm-none-eabi-size "build/amslah.elf"

$(BUILD_PATH)/%.o: %.cpp
	mkdir -p $(@D)
	$(CXX) $(INCLUDE) $(CFLAGS) -o "$@" -c "$<"

$(BUILD_PATH)/%.o: %.c
	mkdir -p $(@D)
	$(CC) $(INCLUDE) $(CFLAGS) -o "$@" -c "$<"

clean:
	rm -rf build

u: $(APP)
	edbg -bpv -t samd21 -f build/amslah.bin

o:
	openocd
