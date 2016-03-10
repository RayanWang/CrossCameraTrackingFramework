################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../mftracker/BB.cpp \
../mftracker/BBPredict.cpp \
../mftracker/FBTrack.cpp \
../mftracker/Lk.cpp \
../mftracker/Median.cpp 

OBJS += \
./mftracker/BB.o \
./mftracker/BBPredict.o \
./mftracker/FBTrack.o \
./mftracker/Lk.o \
./mftracker/Median.o 

CPP_DEPS += \
./mftracker/BB.d \
./mftracker/BBPredict.d \
./mftracker/FBTrack.d \
./mftracker/Lk.d \
./mftracker/Median.d 


# Each subdirectory must supply rules for building sources it contributes
mftracker/%.o: ../mftracker/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -I/home/rayan/workspace/include -O0 -g3 -Wall -c -fmessage-length=0 -fopenmp -std=c++11 -fPIC -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


