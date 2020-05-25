################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../streamer/amf/AMF0.cpp 

OBJS += \
./streamer/amf/AMF0.o 

CPP_DEPS += \
./streamer/amf/AMF0.d 


# Each subdirectory must supply rules for building sources it contributes
streamer/amf/%.o: ../streamer/amf/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	arm-hisiv100-linux-uclibcgnueabi-g++ -I/home/usr/targetfs/include -I"/home/user/svn/code/videostreamer" -Os -Wall -c -fmessage-length=0 -march=armv6 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


