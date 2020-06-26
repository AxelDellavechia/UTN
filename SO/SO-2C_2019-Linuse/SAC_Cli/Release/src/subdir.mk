################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/Generales.c \
../src/Sockets.c \
../src/funcionesFuse.c 

OBJS += \
./src/Generales.o \
./src/Sockets.o \
./src/funcionesFuse.o 

C_DEPS += \
./src/Generales.d \
./src/Sockets.d \
./src/funcionesFuse.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	gcc -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


