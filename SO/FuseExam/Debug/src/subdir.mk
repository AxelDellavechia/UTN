################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/fuse_example.c 

OBJS += \
./src/fuse_example.o 

C_DEPS += \
./src/fuse_example.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -DFUSE_USE_VERSION=27 -D_FILE_OFFSET_BITS=64 -I/usr/include/fuse -pthread -lfuse -lrt -ldl -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


