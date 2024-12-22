#include "NNAlgorithms.h"

uint16_t* NNAlgorithms::getDatasetEntry(uint64_t index){
    uint64_t startIndex = index * k; // this is an artifact of the fact that were storing "2d" data in a 1d array.
    return (uint16_t*)datasetPointer[startIndex];
}


void NNAlgorithms::setDistanceFunctionInt(DistanceFunctionInt distanceFunction){
    distanceFuncInt = distanceFunction;
    distanceFuncFloat = nullptr;
    useFloatsForDistance = false;
}

void NNAlgorithms::setDistanceFunctionFloat(DistanceFunctionFloat distanceFunction){
    distanceFuncInt = nullptr;
    distanceFuncFloat = distanceFunction;
    useFloatsForDistance = true;
}

void NNAlgorithms::buildTree(){
// TO BE IMPLEMENTED
}

uint64_t NNAlgorithms::getClosestIndex(uint16_t* v){
    if(searchAlgo == LinearSearch){ 
        return linearSearch(v);
    }

    else if(searchAlgo == kdTree){
        return kdSearch(v);
    }
}

uint64_t NNAlgorithms::linearSearch(uint16_t* v){ // simplest search algo
    uint64_t currentLowestIndex;

    if(useFloatsForDistance){
        float lowestDistance;

        // must traverse the entire dataset as we want to find the absolute minimum, not just the local minimum
        for(uint64_t i=0;i<datasetSize;i++){
            float distance = distanceFuncFloat(k, v, getDatasetEntry(i));
            if(distance < lowestDistance){
                currentLowestIndex = i;
            }
        }
    }
    
    else{ // using uint32_t
        uint32_t lowestDistance;

        // must traverse the entire dataset as we want to find the absolute minimum, not just the local minimum
        for(uint64_t i=0;i<datasetSize;i++){
            uint32_t distance = distanceFuncInt(k, v, getDatasetEntry(i));
            if(distance < lowestDistance){
                currentLowestIndex = i;
            }
        }
    }

    return currentLowestIndex;
}