################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/cvblobs/BlobContour.cpp \
../src/cvblobs/BlobOperators.cpp \
../src/cvblobs/BlobProperties.cpp \
../src/cvblobs/BlobResult.cpp \
../src/cvblobs/ComponentLabeling.cpp \
../src/cvblobs/blob.cpp 

OBJS += \
./src/cvblobs/BlobContour.o \
./src/cvblobs/BlobOperators.o \
./src/cvblobs/BlobProperties.o \
./src/cvblobs/BlobResult.o \
./src/cvblobs/ComponentLabeling.o \
./src/cvblobs/blob.o 

CPP_DEPS += \
./src/cvblobs/BlobContour.d \
./src/cvblobs/BlobOperators.d \
./src/cvblobs/BlobProperties.d \
./src/cvblobs/BlobResult.d \
./src/cvblobs/ComponentLabeling.d \
./src/cvblobs/blob.d 


# Each subdirectory must supply rules for building sources it contributes
src/cvblobs/%.o: ../src/cvblobs/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -I/home/rayan/cuda-workspace/include -O0 -g3 -Wall -c -fmessage-length=0 -fopenmp -std=c++11 -fPIC -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


