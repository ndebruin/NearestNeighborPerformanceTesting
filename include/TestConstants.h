#include <Arduino.h>

#define testValueCount 100

#define consistencyIndex 0
#define consistencyTestCount 100

const uint16_t* getTestValue(uint32_t index){
    uint32_t startIndex = index * 5; // this is an artifact of the fact that were storing "2d" data in a 1d array.
    return &testData[startIndex];
}