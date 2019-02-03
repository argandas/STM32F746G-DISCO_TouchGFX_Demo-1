################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/freertos.c \
../Core/Src/stm32f7xx_hal_msp.c \
../Core/Src/stm32f7xx_hal_timebase_tim.c \
../Core/Src/stm32f7xx_it.c \
../Core/Src/system_stm32f7xx.c 

CPP_SRCS += \
../Core/Src/main.cpp 

OBJS += \
./Core/Src/freertos.o \
./Core/Src/main.o \
./Core/Src/stm32f7xx_hal_msp.o \
./Core/Src/stm32f7xx_hal_timebase_tim.o \
./Core/Src/stm32f7xx_it.o \
./Core/Src/system_stm32f7xx.o 

C_DEPS += \
./Core/Src/freertos.d \
./Core/Src/stm32f7xx_hal_msp.d \
./Core/Src/stm32f7xx_hal_timebase_tim.d \
./Core/Src/stm32f7xx_it.d \
./Core/Src/system_stm32f7xx.d 

CPP_DEPS += \
./Core/Src/main.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/%.o: ../Core/Src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU GCC Compiler'
	@echo $(PWD)
	arm-none-eabi-gcc -mcpu=cortex-m7 -mthumb -mfloat-abi=hard -mfpu=fpv5-sp-d16 '-D__weak=__attribute__((weak))' '-D__packed=__attribute__((__packed__))' -DUSE_HAL_DRIVER -DSTM32F746xx -I"C:/TouchGFXProjects/MyApplication5/Core/Inc" -I"C:/TouchGFXProjects/MyApplication5/TouchGFX/target" -I"C:/TouchGFXProjects/MyApplication5/TouchGFX/generated/fonts/include" -I"C:/TouchGFXProjects/MyApplication5/TouchGFX/generated/texts/include" -I"C:/TouchGFXProjects/MyApplication5/TouchGFX/generated/images/include" -I"C:/TouchGFXProjects/MyApplication5/TouchGFX/generated/gui_generated/include" -I"C:/TouchGFXProjects/MyApplication5/TouchGFX/gui/include" -I"C:/TouchGFXProjects/MyApplication5/Middlewares/ST/TouchGFX/touchgfx/framework/include" -I"C:/TouchGFXProjects/MyApplication5/Drivers/STM32F7xx_HAL_Driver/Inc" -I"C:/TouchGFXProjects/MyApplication5/Drivers/STM32F7xx_HAL_Driver/Inc/Legacy" -I"C:/TouchGFXProjects/MyApplication5/Middlewares/Third_Party/FreeRTOS/Source/include" -I"C:/TouchGFXProjects/MyApplication5/Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS" -I"C:/TouchGFXProjects/MyApplication5/Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM7/r0p1" -I"C:/TouchGFXProjects/MyApplication5/Drivers/CMSIS/Device/ST/STM32F7xx/Include" -I"C:/TouchGFXProjects/MyApplication5/Drivers/CMSIS/Include"  -Og -g3 -Wall -fmessage-length=0 -ffunction-sections -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

Core/Src/%.o: ../Core/Src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: MCU G++ Compiler'
	@echo $(PWD)
	arm-none-eabi-g++ -mcpu=cortex-m7 -mthumb -mfloat-abi=hard -mfpu=fpv5-sp-d16 '-D__weak=__attribute__((weak))' '-D__packed=__attribute__((__packed__))' -DUSE_HAL_DRIVER -DSTM32F746xx -I"C:/TouchGFXProjects/MyApplication5/Core/Inc" -I"C:/TouchGFXProjects/MyApplication5/TouchGFX/target" -I"C:/TouchGFXProjects/MyApplication5/TouchGFX/generated/fonts/include" -I"C:/TouchGFXProjects/MyApplication5/TouchGFX/generated/texts/include" -I"C:/TouchGFXProjects/MyApplication5/TouchGFX/generated/images/include" -I"C:/TouchGFXProjects/MyApplication5/TouchGFX/generated/gui_generated/include" -I"C:/TouchGFXProjects/MyApplication5/TouchGFX/gui/include" -I"C:/TouchGFXProjects/MyApplication5/Middlewares/ST/TouchGFX/touchgfx/framework/include" -I"C:/TouchGFXProjects/MyApplication5/Drivers/STM32F7xx_HAL_Driver/Inc" -I"C:/TouchGFXProjects/MyApplication5/Drivers/STM32F7xx_HAL_Driver/Inc/Legacy" -I"C:/TouchGFXProjects/MyApplication5/Middlewares/Third_Party/FreeRTOS/Source/include" -I"C:/TouchGFXProjects/MyApplication5/Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS" -I"C:/TouchGFXProjects/MyApplication5/Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM7/r0p1" -I"C:/TouchGFXProjects/MyApplication5/Drivers/CMSIS/Device/ST/STM32F7xx/Include" -I"C:/TouchGFXProjects/MyApplication5/Drivers/CMSIS/Include"  -Og -g3 -Wall -fmessage-length=0 -ffunction-sections -c -fno-exceptions -fno-rtti -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


