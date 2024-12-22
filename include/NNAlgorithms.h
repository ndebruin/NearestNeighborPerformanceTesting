#ifndef NNALGORITHMS_h
#define NNALGORITHMS_h

#include <Arduino.h>


typedef uint32_t (*DistanceFunctionInt)(uint8_t dimensions, uint16_t* v1, uint16_t* v2);
typedef float (*DistanceFunctionFloat)(uint8_t dimensions, uint16_t* v1, uint16_t* v2);

enum SearchAlgorithm{
    LinearSearch = 0,
    kdTree = 1
};


class NNAlgorithms
{
    public:
        NNAlgorithms();
        void setDimensionality(uint8_t dimensions){ k = dimensions; }
        void setDatasetSize(uint64_t DatasetSize){ datasetSize = DatasetSize; }
        void setDataset(const uint16_t *dataset){ datasetPointer = dataset; }

        // this is kinda stupid but we want to benchmark everything so here we are
        void setDistanceFunctionInt(DistanceFunctionInt distanceFunction);
        void setDistanceFunctionFloat(DistanceFunctionFloat distanceFunction);

        void setSearchAlgorithm(SearchAlgorithm searchAlgorithm){ searchAlgo = searchAlgorithm; }

        // this will only do something in the case of a k-d tree, where it will build the tree
        void buildTree();

        // this will return the index of the nearest neighbor to the supplied vector given the distance function and search algorithm chosen
        uint64_t getClosestIndex(uint16_t* v); 


    private:
        uint64_t linearSearch(uint16_t* v);
        uint64_t kdSearch(uint16_t* v);

        const uint16_t *datasetPointer;

        uint16_t* getDatasetEntry(uint64_t index);

        uint8_t k; // dimensions
        uint64_t datasetSize;

        SearchAlgorithm searchAlgo;

        DistanceFunctionInt distanceFuncInt;
        DistanceFunctionFloat distanceFuncFloat;
        bool useFloatsForDistance = false;
};


uint32_t squaredEuclidianDistance(uint8_t dimensions, uint16_t* v1, uint16_t* v2){
    uint32_t squaredDist = 0;
    for(int i = 0; i<dimensions; i++){
        int32_t diff = static_cast<int32_t>(v1[i]) - static_cast<int32_t>(v2[i]);
        squaredDist += diff * diff;
    }
    return squaredDist;
}

float euclidianDistance(uint8_t dimensions, uint16_t* v1, uint16_t* v2){
    return sqrtf(squaredEuclidianDistance(dimensions, v1, v2));
}

uint32_t manhattanDistance(uint8_t dimensions, uint16_t* v1, uint16_t* v2){
    uint32_t dist = 0;
    for (size_t i = 0; i < dimensions; ++i) {
        dist += abs(static_cast<int32_t>(v1[i]) - static_cast<int32_t>(v2[i]));
    }
    return dist;
}



#endif // NNALGORITHMS_h