################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/Generales.c \
../src/funcionesFuse.c \
../src/protocolos_comunicacion.c \
../src/sockets.c 

OBJS += \
./src/Generales.o \
./src/funcionesFuse.o \
./src/protocolos_comunicacion.o \
./src/sockets.o 

C_DEPS += \
./src/Generales.d \
./src/funcionesFuse.d \
./src/protocolos_comunicacion.d \
./src/sockets.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -DFUSE_USE_VERSION=27 -D_FILE_OFFSET_BITS=64 -I/usr/include/fuse -pthread -lfuse -lrt -ldl -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


