################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/cr_startup_lpc17.c \
../src/crp.c \
../src/main.c 

OBJS += \
./src/cr_startup_lpc17.o \
./src/crp.o \
./src/main.o 

C_DEPS += \
./src/cr_startup_lpc17.d \
./src/crp.d \
./src/main.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C Compiler'
	arm-none-eabi-gcc -DDEBUG -D__CODE_RED -DCORE_M3 -D__USE_CMSIS=CMSIS_CORE_LPC17xx -D__LPC17XX__ -D__REDLIB__ -I"C:\Users\sergio\Documents\LPCXpresso_8.2.2_650\workspace\CMSIS_CORE_LPC17xx\inc" -I"C:\Users\sergio\Documents\LPCXpresso_8.2.2_650\workspace\SOTp3_Embds\FreeRTOS" -I"C:\Users\sergio\Documents\LPCXpresso_8.2.2_650\workspace\SOTp3_Embds\FreeRTOS\include" -I"C:\Users\sergio\Documents\LPCXpresso_8.2.2_650\workspace\SOTp3_Embds\FreeRTOS\portable" -I"C:\Users\sergio\Documents\LPCXpresso_8.2.2_650\workspace\SOTp3_Embds\src" -I"C:\Users\sergio\Documents\LPCXpresso_8.2.2_650\workspace\SOTp3_Embds\RecTrace\config" -I"C:\Users\sergio\Documents\LPCXpresso_8.2.2_650\workspace\SOTp3_Embds\RecTrace\include" -O0 -fno-common -g3 -Wall -c -fmessage-length=0 -fno-builtin -ffunction-sections -fdata-sections -mcpu=cortex-m3 -mthumb -specs=redlib.specs -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


