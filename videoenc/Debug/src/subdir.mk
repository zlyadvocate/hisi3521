################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/venc.cpp 

C_SRCS += \
../src/fbsetting.c \
../src/osdtime.c 

OBJS += \
./src/fbsetting.o \
./src/osdtime.o \
./src/venc.o 

C_DEPS += \
./src/fbsetting.d \
./src/osdtime.d 

CPP_DEPS += \
./src/venc.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	arm-hisiv100-linux-uclibcgnueabi-gcc -I/home/user/work/Hi3521_SDK_V1.0.4.0/mpp/include_hi3521 -I/usr/include/ffmpeg -I/media/sf_shared/aac/vo-aacenc-0.1.3 -I../ -I/shared/include -I"/home/user/svn/code/videoenc" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/%.o: ../src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	arm-hisiv100-linux-uclibcgnueabi-g++ -I/usr/include/ffmpeg -I../ -I/shared/include -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


