################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../streamer/core/AudioEncoder.cpp \
../streamer/core/AudioEncoderFAAC.cpp \
../streamer/core/AudioEncoderG711.cpp \
../streamer/core/AudioReceiveThread.cpp \
../streamer/core/BitStream.cpp \
../streamer/core/Debug.cpp \
../streamer/core/EncoderThread.cpp \
../streamer/core/EncoderTypes.cpp \
../streamer/core/FlvWritter.cpp \
../streamer/core/H264Parser.cpp \
../streamer/core/Log.cpp \
../streamer/core/MemoryPool.cpp \
../streamer/core/RTMPThread.cpp \
../streamer/core/RTMPWriter.cpp \
../streamer/core/VideoEncoder.cpp \
../streamer/core/videoReceiveThread.cpp 

C_SRCS += \
../streamer/core/videobuf.c 

OBJS += \
./streamer/core/AudioEncoder.o \
./streamer/core/AudioEncoderFAAC.o \
./streamer/core/AudioEncoderG711.o \
./streamer/core/AudioReceiveThread.o \
./streamer/core/BitStream.o \
./streamer/core/Debug.o \
./streamer/core/EncoderThread.o \
./streamer/core/EncoderTypes.o \
./streamer/core/FlvWritter.o \
./streamer/core/H264Parser.o \
./streamer/core/Log.o \
./streamer/core/MemoryPool.o \
./streamer/core/RTMPThread.o \
./streamer/core/RTMPWriter.o \
./streamer/core/VideoEncoder.o \
./streamer/core/videoReceiveThread.o \
./streamer/core/videobuf.o 

C_DEPS += \
./streamer/core/videobuf.d 

CPP_DEPS += \
./streamer/core/AudioEncoder.d \
./streamer/core/AudioEncoderFAAC.d \
./streamer/core/AudioEncoderG711.d \
./streamer/core/AudioReceiveThread.d \
./streamer/core/BitStream.d \
./streamer/core/Debug.d \
./streamer/core/EncoderThread.d \
./streamer/core/EncoderTypes.d \
./streamer/core/FlvWritter.d \
./streamer/core/H264Parser.d \
./streamer/core/Log.d \
./streamer/core/MemoryPool.d \
./streamer/core/RTMPThread.d \
./streamer/core/RTMPWriter.d \
./streamer/core/VideoEncoder.d \
./streamer/core/videoReceiveThread.d 


# Each subdirectory must supply rules for building sources it contributes
streamer/core/%.o: ../streamer/core/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	arm-hisiv100-linux-uclibcgnueabi-g++ -I/home/usr/targetfs/include -I"/home/user/svn/code/videostreamer" -Os -Wall -c -fmessage-length=0 -march=armv6 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

streamer/core/%.o: ../streamer/core/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	arm-hisiv100-linux-uclibcgnueabi-gcc -Os -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


