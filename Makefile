CC = arm-none-eabi-gcc
CXX = arm-none-eabi-g++
LD = arm-none-eabi-gcc
AR = arm-none-eabi-ar
OBJCOPY = arm-none-eabi-objcopy
SIZE = arm-none-eabi-size


CFLAGS = -x c -mthumb -DDEBUG -O2 -ffunction-sections -mlong-calls -Wall -g3
CFLAGS += -c -D__SAMD21J18A__ -mcpu=cortex-m0plus

LFLAGS = -T"core/samd21j18a_flash.ld"
LFLAGS += -Wl,--gc-sections -mcpu=cortex-m0plus  -lm

INCLUDE = -I"./core"

BUILD_PATH = build
APP = app

CSRC += main.c
CSRC += core/startup_samd21.c
CSRC += core/gpio.c
CSRC += core/util.c

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
