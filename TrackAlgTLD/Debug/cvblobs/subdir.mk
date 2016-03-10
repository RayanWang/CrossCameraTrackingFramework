################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../cvblobs/BlobContour.cpp \
../cvblobs/BlobOperators.cpp \
../cvblobs/BlobProperties.cpp \
../cvblobs/BlobResult.cpp \
../cvblobs/ComponentLabeling.cpp \
../cvblobs/blob.cpp 

OBJS += \
./cvblobs/BlobContour.o \
./cvblobs/BlobOperators.o \
./cvblobs/BlobProperties.o \
./cvblobs/BlobResult.o \
./cvblobs/ComponentLabeling.o \
./cvblobs/blob.o 

CPP_DEPS += \
./cvblobs/BlobContour.d \
./cvblobs/BlobOperators.d \
./cvblobs/BlobProperties.d \
./cvblobs/BlobResult.d \
./cvblobs/ComponentLabeling.d \
./cvblobs/blob.d 


# Each subdirectory must supply rules for building sources it contributes
cvblobs/%.o: ../cvblobs/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -I/home/rayan/workspace/include -O0 -g3 -Wall -c -fmessage-length=0 -fopenmp -std=c++11 -fPIC -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


