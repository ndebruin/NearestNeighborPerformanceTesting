#include "NNAlgorithms.h"

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

Vector NNAlgorithms::getDatasetEntry(uint32_t index){
    uint32_t startIndex = index * k; // this is an artifact of the fact that were storing "2d" data in a 1d array.
    return &datasetPointer[startIndex];
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

uint32_t NNAlgorithms::getClosestIndex(Vector v){
    if(searchAlgo == LinearSearch){ 
        return linearSearch(v);
    }

    else if(searchAlgo == kdTree){
        return kdSearch(v);
    }
    return 0;
}

uint32_t NNAlgorithms::linearSearch(Vector v){ // simplest search algo
    uint32_t currentLowestIndex = 0;

    if(useFloatsForDistance){
        float lowestDistance = MAXFLOAT;

        // must traverse the entire dataset as we want to find the absolute minimum, not just the local minimum
        for(uint32_t i=0;i<datasetSize;i++){
            float distance = distanceFuncFloat(k, v, getDatasetEntry(i));
            if(distance < lowestDistance){
                currentLowestIndex = i;
            }
        }
    }
    
    else{ // using uint32_t
        uint32_t lowestDistance = UINT32_MAX;

        // must traverse the entire dataset as we want to find the absolute minimum, not just the local minimum
        for(uint32_t i=0;i<datasetSize;i++){
            uint32_t distance = distanceFuncInt(k, v, getDatasetEntry(i));
            if(distance < lowestDistance){
                currentLowestIndex = i;
            }
        }
    }

    return currentLowestIndex;
}

// k-d tree specific code

void NNAlgorithms::buildTree(){
    indexMap = (uint32_t*)malloc(datasetSize * sizeof(uint32_t)); // reserve memory for the entire index map
    for (uint32_t i = 0; i < datasetSize; i++) {
        indexMap[i] = i; // Fill the index map
    }
    rootNode = buildKDTree(0, datasetSize, 0); // start with the whole thing, an initial depth of zero
}

void NNAlgorithms::deleteKDNode(kdTreeNode* node){
    if(!node) return; // protects from errors where node = nullptr
    deleteKDNode(node->left);
    deleteKDNode(node->right);
    delete node; 
}

void NNAlgorithms::clearTree(){
    deleteKDNode(rootNode); 
    free(indexMap); // free the memory that stores the index map
    indexMap = NULL;
}

kdTreeNode* NNAlgorithms::buildKDTree(uint32_t startIndex, uint32_t endIndex, uint32_t depth){
    if(startIndex >= endIndex) return nullptr; // safety thing

    uint32_t currentDimension = depth % k;
    uint32_t middleIndex = (startIndex + endIndex) / 2;

    // Sort indices based on the current dimension
    // this will place the median value at the middle index, 
    // with all less in that dimension left of the middle index,
    // and all greater in that dimension right of the middle index
    // remember: the k-d tree only sorts on 1 dimension at a time, therefore this works.
    std::nth_element(indexMap + startIndex, indexMap + middleIndex, indexMap + endIndex, [this, currentDimension](uint32_t v1, uint32_t v2) {
        return this->greaterThanInDimension(v1, v2, currentDimension);
    });
    
    kdTreeNode* node = new kdTreeNode();

    node->index = indexMap[middleIndex];
    // go from the start to the middle value, at one further depth
    node->left = buildKDTree(startIndex, middleIndex, depth+1); // it's the same function! recursive
    // go from just after the middle index to the end, at one further depth
    node->right = buildKDTree(middleIndex+1, endIndex, depth +1); // it's the same function! recursive
    
    return node;
}

uint32_t NNAlgorithms::kdSearch(Vector v){
    uint32_t bestIndex = 0;
    uint32_t bestDistanceInt = UINT32_MAX;
    float bestDistanceFloat = MAXFLOAT;
    searchNode(rootNode, v, bestIndex, bestDistanceInt, bestDistanceFloat, 0);
    return bestIndex;
}


void NNAlgorithms::searchNode(kdTreeNode* node, Vector searchVector, uint32_t& bestIndex, uint32_t& bestDistanceInt, float& bestDistanceFloat, uint32_t depth){
    if(!node){ return; } // this saves us if it's a nullptr node (ex: end of a branch)

    Vector currentVector = getDatasetEntry(node->index);

    // see if our current node's distance is better than what we currently know to be best
    if(useFloatsForDistance){
        float currentDist = distanceFuncFloat(k, searchVector, currentVector);

        if(currentDist < bestDistanceFloat){
            bestDistanceFloat = currentDist;
            bestIndex = node->index;
        }
    }
    else{
        uint32_t currentDist = distanceFuncInt(k, searchVector, currentVector);

        if(currentDist < bestDistanceInt){
            bestDistanceInt = currentDist;
            bestIndex = node->index;
        }
    }

    uint32_t currentDimension = depth % k;
    
    // this is to determine whether we should search the left or right side of the node first

    int32_t diffOnCurrentDim = static_cast<int32_t>(searchVector[currentDimension]) - static_cast<int32_t>(currentVector[currentDimension]);

    kdTreeNode* nearNode;
    kdTreeNode* farNode;

    // our search value is less than the current node's value on this dimension, so we should use the left side first
    if(diffOnCurrentDim < 0){
        nearNode = node->left;
        farNode = node->right;
    }
    else{ // search value is greater than current node's value, use right side as closer guess
        nearNode = node->right;
        farNode = node->left;
    }

    // search the closer subtree first
    searchNode(nearNode, searchVector, bestIndex, bestDistanceInt, bestDistanceFloat, depth +1);

    // check if the far side might have a better value
    // if so, also traverse the far side subtree to check for a good value
    if(static_cast<uint32_t>(diffOnCurrentDim * diffOnCurrentDim) < bestDistanceInt // check both for the integer distance type
    || static_cast<float>(diffOnCurrentDim*diffOnCurrentDim) < bestDistanceFloat){ // and the float distance type
        searchNode(farNode, searchVector, bestIndex, bestDistanceInt, bestDistanceFloat, depth +1);
    }
}