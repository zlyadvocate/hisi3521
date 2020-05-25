################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../streamer/amf/types/AMF0Boolean.cpp \
../streamer/amf/types/AMF0EcmaArray.cpp \
../streamer/amf/types/AMF0Number.cpp \
../streamer/amf/types/AMF0Object.cpp \
../streamer/amf/types/AMF0Property.cpp \
../streamer/amf/types/AMF0String.cpp \
../streamer/amf/types/AMFType.cpp 

OBJS += \
./streamer/amf/types/AMF0Boolean.o \
./streamer/amf/types/AMF0EcmaArray.o \
./streamer/amf/types/AMF0Number.o \
./streamer/amf/types/AMF0Object.o \
./streamer/amf/types/AMF0Property.o \
./streamer/amf/types/AMF0String.o \
./streamer/amf/types/AMFType.o 

CPP_DEPS += \
./streamer/amf/types/AMF0Boolean.d \
./streamer/amf/types/AMF0EcmaArray.d \
./streamer/amf/types/AMF0Number.d \
./streamer/amf/types/AMF0Object.d \
./streamer/amf/types/AMF0Property.d \
./streamer/amf/types/AMF0String.d \
./streamer/amf/types/AMFType.d 


# Each subdirectory must supply rules for building sources it contributes
streamer/amf/types/%.o: ../streamer/amf/types/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	arm-hisiv100-linux-uclibcgnueabi-g++ -I/home/usr/targetfs/include -I"/home/user/svn/code/videostreamer" -Os -Wall -c -fmessage-length=0 -march=armv6 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


