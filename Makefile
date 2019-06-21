CC = arm-none-eabi-gcc -B/usr/bin/arm-none-eabi-
CXX = arm-none-eabi-g++ -B/usr/bin/arm-none-eabi-
LD = arm-none-eabi-gcc -B/usr/bin/arm-none-eabi-
AR = arm-none-eabi-ar
OBJCOPY = arm-none-eabi-objcopy
SIZE = arm-none-eabi-size


CFLAGS = -x c -mthumb -DDEBUG -O2 -ffunction-sections -mlong-calls -Wall -g3
CFLAGS += -c -D__SAMD21J18A__ -mcpu=cortex-m0plus

LFLAGS = -T"core/samd21j18a_flash.ld"
LFLAGS += -Wl,--gc-sections -mcpu=cortex-m0plus  -lm

INCLUDE = -I"./core" -I"./config" -I"./freertos/include" -I"./freertos/portable"

BUILD_PATH = build
APP = app

CSRC += main.c
CSRC += bmp3.c
CSRC += core/startup_samd21.c
CSRC += core/gpio.c
CSRC += core/util.c
CSRC += core/uart.c
CSRC += core/spi.c
CSRC += core/sercoms.c
CSRC += core/printf.c
CSRC += freertos/croutine.c
CSRC += freertos/event_groups.c
CSRC += freertos/list.c
CSRC += freertos/queue.c
CSRC += freertos/tasks.c
CSRC += freertos/stream_buffer.c
CSRC += freertos/timers.c
CSRC += freertos/heap_1.c
CSRC += freertos/portable/port.c

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
