################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../common/HI264RTPPACK.c \
../common/hi_faac.c \
../common/loadbmp.c \
../common/sample_comm_audio.c \
../common/sample_comm_sys.c \
../common/sample_comm_vda.c \
../common/sample_comm_vdec.c \
../common/sample_comm_venc.c \
../common/sample_comm_vi.c \
../common/sample_comm_vo.c \
../common/sample_comm_vpss.c \
../common/videobuf.c \
../common/zini.c 

OBJS += \
./common/HI264RTPPACK.o \
./common/hi_faac.o \
./common/loadbmp.o \
./common/sample_comm_audio.o \
./common/sample_comm_sys.o \
./common/sample_comm_vda.o \
./common/sample_comm_vdec.o \
./common/sample_comm_venc.o \
./common/sample_comm_vi.o \
./common/sample_comm_vo.o \
./common/sample_comm_vpss.o \
./common/videobuf.o \
./common/zini.o 

C_DEPS += \
./common/HI264RTPPACK.d \
./common/hi_faac.d \
./common/loadbmp.d \
./common/sample_comm_audio.d \
./common/sample_comm_sys.d \
./common/sample_comm_vda.d \
./common/sample_comm_vdec.d \
./common/sample_comm_venc.d \
./common/sample_comm_vi.d \
./common/sample_comm_vo.d \
./common/sample_comm_vpss.d \
./common/videobuf.d \
./common/zini.d 


# Each subdirectory must supply rules for building sources it contributes
common/%.o: ../common/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	arm-hisiv100-linux-uclibcgnueabi-gcc -I/home/user/work/Hi3521_SDK_V1.0.4.0/mpp/include_hi3521 -I/usr/include/ffmpeg -I/media/sf_shared/aac/vo-aacenc-0.1.3 -I../ -I/shared/include -I"/home/user/svn/code/videoenc" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


