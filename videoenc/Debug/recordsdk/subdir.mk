################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../recordsdk/fileBak.c \
../recordsdk/fileDelete.c \
../recordsdk/hdPowerManage.c \
../recordsdk/indexFile.c \
../recordsdk/indexFileExt.c \
../recordsdk/mp4File.c \
../recordsdk/recordCtrl.c \
../recordsdk/recordFile.c \
../recordsdk/recordSDK.c \
../recordsdk/sdCard.c \
../recordsdk/util.c 

OBJS += \
./recordsdk/fileBak.o \
./recordsdk/fileDelete.o \
./recordsdk/hdPowerManage.o \
./recordsdk/indexFile.o \
./recordsdk/indexFileExt.o \
./recordsdk/mp4File.o \
./recordsdk/recordCtrl.o \
./recordsdk/recordFile.o \
./recordsdk/recordSDK.o \
./recordsdk/sdCard.o \
./recordsdk/util.o 

C_DEPS += \
./recordsdk/fileBak.d \
./recordsdk/fileDelete.d \
./recordsdk/hdPowerManage.d \
./recordsdk/indexFile.d \
./recordsdk/indexFileExt.d \
./recordsdk/mp4File.d \
./recordsdk/recordCtrl.d \
./recordsdk/recordFile.d \
./recordsdk/recordSDK.d \
./recordsdk/sdCard.d \
./recordsdk/util.d 


# Each subdirectory must supply rules for building sources it contributes
recordsdk/%.o: ../recordsdk/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	arm-hisiv100-linux-uclibcgnueabi-gcc -I/home/user/work/Hi3521_SDK_V1.0.4.0/mpp/include_hi3521 -I/usr/include/ffmpeg -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


