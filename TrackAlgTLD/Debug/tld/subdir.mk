################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../tld/Clustering.cpp \
../tld/DetectionResult.cpp \
../tld/DetectorCascade.cpp \
../tld/EnsembleClassifier.cpp \
../tld/ForegroundDetector.cpp \
../tld/MedianFlowTracker.cpp \
../tld/NNClassifier.cpp \
../tld/TLD.cpp \
../tld/TLDUtil.cpp \
../tld/VarianceFilter.cpp 

OBJS += \
./tld/Clustering.o \
./tld/DetectionResult.o \
./tld/DetectorCascade.o \
./tld/EnsembleClassifier.o \
./tld/ForegroundDetector.o \
./tld/MedianFlowTracker.o \
./tld/NNClassifier.o \
./tld/TLD.o \
./tld/TLDUtil.o \
./tld/VarianceFilter.o 

CPP_DEPS += \
./tld/Clustering.d \
./tld/DetectionResult.d \
./tld/DetectorCascade.d \
./tld/EnsembleClassifier.d \
./tld/ForegroundDetector.d \
./tld/MedianFlowTracker.d \
./tld/NNClassifier.d \
./tld/TLD.d \
./tld/TLDUtil.d \
./tld/VarianceFilter.d 


# Each subdirectory must supply rules for building sources it contributes
tld/%.o: ../tld/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -I/home/rayan/workspace/include -O0 -g3 -Wall -c -fmessage-length=0 -fopenmp -std=c++11 -fPIC -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


