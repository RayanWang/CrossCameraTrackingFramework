################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/mftracker/BB.cpp \
../src/mftracker/BBPredict.cpp \
../src/mftracker/FBTrack.cpp \
../src/mftracker/Lk.cpp \
../src/mftracker/Median.cpp 

OBJS += \
./src/mftracker/BB.o \
./src/mftracker/BBPredict.o \
./src/mftracker/FBTrack.o \
./src/mftracker/Lk.o \
./src/mftracker/Median.o 

CPP_DEPS += \
./src/mftracker/BB.d \
./src/mftracker/BBPredict.d \
./src/mftracker/FBTrack.d \
./src/mftracker/Lk.d \
./src/mftracker/Median.d 


# Each subdirectory must supply rules for building sources it contributes
src/mftracker/%.o: ../src/mftracker/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -I/home/rayan/workspace/include -O3 -g3 -Wall -c -fmessage-length=0 -fopenmp -fPIC -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


