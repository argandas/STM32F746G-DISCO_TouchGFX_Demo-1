################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../TouchGFX/generated/images/src/BitmapDatabase.cpp \
../TouchGFX/generated/images/src/bg.cpp \
../TouchGFX/generated/images/src/black_arrow.cpp \
../TouchGFX/generated/images/src/black_arrow_up.cpp \
../TouchGFX/generated/images/src/btn_long.cpp \
../TouchGFX/generated/images/src/btn_long_pressed.cpp \
../TouchGFX/generated/images/src/counter_box.cpp \
../TouchGFX/generated/images/src/orange_arrow.cpp \
../TouchGFX/generated/images/src/orange_arrow_up.cpp \
../TouchGFX/generated/images/src/small_btn.cpp \
../TouchGFX/generated/images/src/small_btn_disable.cpp \
../TouchGFX/generated/images/src/small_btn_pressed.cpp 

OBJS += \
./TouchGFX/generated/images/src/BitmapDatabase.o \
./TouchGFX/generated/images/src/bg.o \
./TouchGFX/generated/images/src/black_arrow.o \
./TouchGFX/generated/images/src/black_arrow_up.o \
./TouchGFX/generated/images/src/btn_long.o \
./TouchGFX/generated/images/src/btn_long_pressed.o \
./TouchGFX/generated/images/src/counter_box.o \
./TouchGFX/generated/images/src/orange_arrow.o \
./TouchGFX/generated/images/src/orange_arrow_up.o \
./TouchGFX/generated/images/src/small_btn.o \
./TouchGFX/generated/images/src/small_btn_disable.o \
./TouchGFX/generated/images/src/small_btn_pressed.o 

CPP_DEPS += \
./TouchGFX/generated/images/src/BitmapDatabase.d \
./TouchGFX/generated/images/src/bg.d \
./TouchGFX/generated/images/src/black_arrow.d \
./TouchGFX/generated/images/src/black_arrow_up.d \
./TouchGFX/generated/images/src/btn_long.d \
./TouchGFX/generated/images/src/btn_long_pressed.d \
./TouchGFX/generated/images/src/counter_box.d \
./TouchGFX/generated/images/src/orange_arrow.d \
./TouchGFX/generated/images/src/orange_arrow_up.d \
./TouchGFX/generated/images/src/small_btn.d \
./TouchGFX/generated/images/src/small_btn_disable.d \
./TouchGFX/generated/images/src/small_btn_pressed.d 


# Each subdirectory must supply rules for building sources it contributes
TouchGFX/generated/images/src/%.o: ../TouchGFX/generated/images/src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: MCU G++ Compiler'
	@echo $(PWD)
	arm-none-eabi-g++ -mcpu=cortex-m7 -mthumb -mfloat-abi=hard -mfpu=fpv5-sp-d16 '-D__weak=__attribute__((weak))' '-D__packed=__attribute__((__packed__))' -DUSE_HAL_DRIVER -DSTM32F746xx -I"C:/TouchGFXProjects/MyApplication5/Core/Inc" -I"C:/TouchGFXProjects/MyApplication5/TouchGFX/target" -I"C:/TouchGFXProjects/MyApplication5/TouchGFX/generated/fonts/include" -I"C:/TouchGFXProjects/MyApplication5/TouchGFX/generated/texts/include" -I"C:/TouchGFXProjects/MyApplication5/TouchGFX/generated/images/include" -I"C:/TouchGFXProjects/MyApplication5/TouchGFX/generated/gui_generated/include" -I"C:/TouchGFXProjects/MyApplication5/TouchGFX/gui/include" -I"C:/TouchGFXProjects/MyApplication5/Middlewares/ST/TouchGFX/touchgfx/framework/include" -I"C:/TouchGFXProjects/MyApplication5/Drivers/STM32F7xx_HAL_Driver/Inc" -I"C:/TouchGFXProjects/MyApplication5/Drivers/STM32F7xx_HAL_Driver/Inc/Legacy" -I"C:/TouchGFXProjects/MyApplication5/Middlewares/Third_Party/FreeRTOS/Source/include" -I"C:/TouchGFXProjects/MyApplication5/Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS" -I"C:/TouchGFXProjects/MyApplication5/Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM7/r0p1" -I"C:/TouchGFXProjects/MyApplication5/Drivers/CMSIS/Device/ST/STM32F7xx/Include" -I"C:/TouchGFXProjects/MyApplication5/Drivers/CMSIS/Include"  -Og -g3 -Wall -fmessage-length=0 -ffunction-sections -c -fno-exceptions -fno-rtti -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


