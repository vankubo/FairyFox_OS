
.PHONY: clean
#######################################
# binaries
#######################################
PREFIX = arm-none-eabi-
# The gcc compiler bin path can be either defined in make command via GCC_PATH variable (> make GCC_PATH=xxx)
# either it can be added to the PATH environment variable.
ifdef GCC_PATH
CC = $(GCC_PATH)/$(PREFIX)gcc
AS = $(GCC_PATH)/$(PREFIX)gcc -x assembler-with-cpp
CP = $(GCC_PATH)/$(PREFIX)objcopy
SZ = $(GCC_PATH)/$(PREFIX)size
else
CC = $(PREFIX)gcc
AS = $(PREFIX)gcc -x assembler-with-cpp
CP = $(PREFIX)objcopy
SZ = $(PREFIX)size
endif
HEX = $(CP) -O ihex
BIN = $(CP) -O binary -S

#openocd
OCD_INTERFACE =interface/cmsis-dap.cfg 
OCD_TARGET =target/stm32f4x.cfg
######################################
# target
######################################
TARGET = FoxFairyRTOS_gcc

# macros for gcc
# AS defines
AS_DEFS = 

# C defines
C_DEFS =  \
-DSTM32F401xx \
-DUSE_STDPERIPH_DRIVER \
-DARM_MATH_CM4 \
-DHSE_VALUE=((uint32_t)25000000) \
######################################
# building variables
######################################
# debug build?
DEBUG = 1
# optimization
#-Og
OPT = -Og

#######################################
# paths
#######################################
# Build path
BUILD_DIR = BUILD
# AS includes
AS_INCLUDES = 

# C includes
#startup
C_INCLUDES =  \
-ICMSIS/startup \
-ICMSIS/Include \
#std periph
C_INCLUDES += \
-ICMSIS/StdPeriph_Driver/inc \
#FreeRTOS
C_INCLUDES += \
-IRTOS/arch \
-IRTOS/kernel \
-IRTOS/memfox \
#user
C_INCLUDES += \
-IUSER \
-ISYSTEM/delay \
-ISYSTEM/sys \
-ISYSTEM/usart \
-IHARDWARE/LED \
-IMIDWARE/Serial \
-IHARDWARE/TIMER \
######################################
# source
######################################
# ASM sources
ASM_SOURCES =  \
CMSIS/startup/startup_stm32f40_41xxx.s \

# C sources
#CORE
C_SOURCES =  \
CMSIS/Startup/system_stm32f4xx.c \
#ST
C_SOURCES +=  \
CMSIS/StdPeriph_Driver/src/misc.c \
CMSIS/StdPeriph_Driver/src/stm32f4xx_rcc.c \
CMSIS/StdPeriph_Driver/src/stm32f4xx_gpio.c \
CMSIS/StdPeriph_Driver/src/stm32f4xx_usart.c \
CMSIS/StdPeriph_Driver/src/stm32f4xx_tim.c \
#RTOS
C_SOURCES +=  \
RTOS/arch/foxport.c \
RTOS/kernel/thread.c \
RTOS/memfox/memfox.c \

#USER
C_SOURCES +=  \
USER/main.c \
SYSTEM/delay/delay_Fox.c \
SYSTEM/sys/sys.c \
SYSTEM/usart/usart.c \
HARDWARE/TIMER/timer.c \
#MIDWARE/Serial/sendwave.c \
#HARDWARE/LED/led.c \

#######################################
# LDFLAGS
#######################################
# link script
LDSCRIPT = STM32F401VC_FLASH.ld

# libraries
LIBS =	-lc -lm -lnosys \
#-larm_cortexM4lf_math \

LIBDIR = \
#-LCMSIS/DSP_Lib \


#######################################
# CFLAGS
#######################################
# cpu
CPU = -mcpu=cortex-m4

# fpu
# NONE for Cortex-M0/M0+/M3
FPU = -mfpu=fpv4-sp-d16
# float-abi
FLOAT-ABI = -mfloat-abi=hard

# mcu
MCU = $(CPU) -mthumb $(FPU) $(FLOAT-ABI)

# compile gcc flags
ASFLAGS = $(MCU) $(AS_DEFS) $(AS_INCLUDES) $(OPT) -Wall -fdata-sections -ffunction-sections

CFLAGS = $(MCU) $(C_DEFS) $(C_INCLUDES) $(OPT) -Wall -fdata-sections -ffunction-sections

ifeq ($(DEBUG), 1)
CFLAGS += -g -gdwarf-2
endif


# Generate dependency information
CFLAGS += -MMD -MP -MF"$(@:%.o=%.d)"


LDFLAGS = $(MCU) -specs=nano.specs -T$(LDSCRIPT) $(LIBDIR) $(LIBS) -Wl,-Map=$(BUILD_DIR)/$(TARGET).map,--cref -Wl,--gc-sections

# default action: build all
all: $(BUILD_DIR)/$(TARGET).elf $(BUILD_DIR)/$(TARGET).hex $(BUILD_DIR)/$(TARGET).bin


#######################################
# build the application
#######################################
# list of objects
OBJECTS = $(addprefix $(BUILD_DIR)/,$(notdir $(C_SOURCES:.c=.o)))
vpath %.c $(sort $(dir $(C_SOURCES)))
# list of ASM program objects
OBJECTS += $(addprefix $(BUILD_DIR)/,$(notdir $(ASM_SOURCES:.s=.o)))
vpath %.s $(sort $(dir $(ASM_SOURCES)))

$(BUILD_DIR)/%.o: %.c Makefile | $(BUILD_DIR) 
	$(CC) -c $(CFLAGS) -Wa,-a,-ad,-alms=$(BUILD_DIR)/$(notdir $(<:.c=.lst)) $< -o $@

$(BUILD_DIR)/%.o: %.s Makefile | $(BUILD_DIR)
	$(AS) -c $(CFLAGS) $< -o $@

$(BUILD_DIR)/$(TARGET).elf: $(OBJECTS) Makefile
	$(CC) $(OBJECTS) $(LDFLAGS) -o $@
	$(SZ) $@

$(BUILD_DIR)/%.hex: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
	$(HEX) $< $@
	
$(BUILD_DIR)/%.bin: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
	$(BIN) $< $@	
	
$(BUILD_DIR):
	mkdir $@		

#######################################
# clean up
#######################################
clean:
	del  BUILD\*.* /q
##########################
# write to flash
##########################
write:
	openocd -f $(OCD_INTERFACE)  -f $(OCD_TARGET) -c init -c halt -c "flash write_image erase $(BUILD_DIR)/$(TARGET).hex" -c reset -c shutdown
 

#######################################
# dependencies
#######################################
-include $(wildcard $(BUILD_DIR)/*.d)

# *** EOF ***
