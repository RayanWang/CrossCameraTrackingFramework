################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/main/Config.cpp \
../src/main/Main.cpp \
../src/main/Trajectory.cpp 

OBJS += \
./src/main/Config.o \
./src/main/Main.o \
./src/main/Trajectory.o 

CPP_DEPS += \
./src/main/Config.d \
./src/main/Main.d \
./src/main/Trajectory.d 


# Each subdirectory must supply rules for building sources it contributes
src/main/%.o: ../src/main/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -I/home/rayan/cuda-workspace/include -O0 -g3 -Wall -c -fmessage-length=0 -fopenmp -std=c++11 -fPIC -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


