#ifndef NNALGORITHMS_h
#define NNALGORITHMS_h

#include <Arduino.h>
#include <limits>

// this is really just a pointer to the first dimension of the vector,
// but using the Vector keyword makes things more understandable imo
// it allows you to look and understand the logic without needing to look at the programming as much
typedef const uint16_t* Vector; 


typedef uint32_t (*DistanceFunctionInt)(uint8_t dimensions, Vector v1, Vector v2);
typedef float (*DistanceFunctionFloat)(uint8_t dimensions, Vector v1, Vector v2);

enum SearchAlgorithm{
    LinearSearch = 0,
    kdTree = 1
};


// tree data struct. we only store the index to the place in the dataset, to save memory
// storing the actually data in the struct would end up duplicating the dataset into SRAM, rather than keeping it in PROGRAM
struct kdTreeNode{
    uint32_t index;
    kdTreeNode* left;
    kdTreeNode* right;
};


class NNAlgorithms
{
    public:
        NNAlgorithms();
        void setDimensionality(uint8_t dimensions){ k = dimensions; }
        void setDatasetSize(uint32_t DatasetSize){ datasetSize = DatasetSize; }
        void setDataset(const uint16_t *dataset){ datasetPointer = dataset; }

        // this is kinda stupid but we want to benchmark everything so here we are
        void setDistanceFunctionInt(DistanceFunctionInt distanceFunction);
        void setDistanceFunctionFloat(DistanceFunctionFloat distanceFunction);

        void setSearchAlgorithm(SearchAlgorithm searchAlgorithm){ searchAlgo = searchAlgorithm; }

        // this will only do something in the case of a k-d tree, where it will build the tree
        void buildTree();
         // memory cleanup
        void clearTree();

        // this will return the index of the nearest neighbor to the supplied vector given the distance function and search algorithm chosen
        uint32_t getClosestIndex(Vector v); 


    private:
        uint32_t linearSearch(Vector v);
        uint32_t kdSearch(Vector v);

        const uint16_t *datasetPointer;

        Vector getDatasetEntry(uint32_t index);

        uint8_t k; // dimensions
        uint32_t datasetSize;

        SearchAlgorithm searchAlgo;

        DistanceFunctionInt distanceFuncInt;
        DistanceFunctionFloat distanceFuncFloat;
        bool useFloatsForDistance = false;

        // k-d tree specific stuff
        kdTreeNode* rootNode = nullptr;

        void searchNode(kdTreeNode* node, Vector searchVector, uint32_t& bestIndex, uint32_t& bestDistanceInt, float& bestDistanceFloat, uint32_t depth);

        /* this is a little confusing. 
        the data itself is uint16_t.
        however, the index in the dataset is stored in a uint32_t.
        this is an array of the indices of the array, that will be reorganized as part of creating the k-d tree.        
        hence, the name indexMap
        */
        uint32_t* indexMap;

        // creating the k-d tree is a recursive process, this is our recursive function for creating the tree
        kdTreeNode* buildKDTree(uint32_t startIndex, uint32_t endIndex, uint32_t depth);

        void deleteKDNode(kdTreeNode* node);

        // compares two vectors upon a specified dimension. used for k-d tree
        bool greaterThanInDimension(const uint32_t& comparision, const uint32_t& base, uint8_t dimension){
            return (getDatasetEntry(base)[dimension] < getDatasetEntry(comparision)[dimension]);
        }
};

uint32_t squaredEuclidianDistance(uint8_t dimensions, Vector v1, Vector v2){
    uint32_t squaredDist = 0;
    for(int i = 0; i<dimensions; i++){
        int32_t diff = static_cast<int32_t>(v1[i]) - static_cast<int32_t>(v2[i]);
        squaredDist += diff * diff;
    }
    return squaredDist;
}

float euclidianDistance(uint8_t dimensions, Vector v1, Vector v2){
    return sqrtf(squaredEuclidianDistance(dimensions, v1, v2));
}

uint32_t manhattanDistance(uint8_t dimensions, Vector v1, Vector v2){
    uint32_t dist = 0;
    for (uint32_t i = 0; i < dimensions; ++i) {
        dist += abs(static_cast<int32_t>(v1[i]) - static_cast<int32_t>(v2[i]));
    }
    return dist;
}



#endif // NNALGORITHMS_h