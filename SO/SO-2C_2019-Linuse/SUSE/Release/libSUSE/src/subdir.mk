################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../libSUSE/src/Generales.c \
../libSUSE/src/Sockets.c \
../libSUSE/src/libsuse.c 

OBJS += \
./libSUSE/src/Generales.o \
./libSUSE/src/Sockets.o \
./libSUSE/src/libsuse.o 

C_DEPS += \
./libSUSE/src/Generales.d \
./libSUSE/src/Sockets.d \
./libSUSE/src/libsuse.d 


# Each subdirectory must supply rules for building sources it contributes
libSUSE/src/%.o: ../libSUSE/src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


