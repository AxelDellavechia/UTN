################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../hilolay/hilolay_internal.c \
../hilolay/implementacion.c 

OBJS += \
./hilolay/hilolay_internal.o \
./hilolay/implementacion.o 

C_DEPS += \
./hilolay/hilolay_internal.d \
./hilolay/implementacion.d 


# Each subdirectory must supply rules for building sources it contributes
hilolay/%.o: ../hilolay/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


