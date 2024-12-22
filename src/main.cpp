#include <Arduino.h>
#include "CICalibrationValues.h"
#include "CIAngleValues.h"
#include "CITestValues.h"

#include "NNAlgorithms.h"

// if your platform using something other than the default Serial for the USB serial connection, please replace it here
#define SERIALPORT Serial

NNAlgorithms nnAlgorithm;

uint16_t* getTestValue(uint64_t index);

void runTest(){
  for(uint64_t i = 0; i<testValueCount; i++){
    uint32_t startTime = micros();
    uint64_t guessedIndex = nnAlgorithm.getClosestIndex(getTestValue(i));
    uint32_t timeTaken = micros() - startTime;

    bool correct = false;
    if(guessedIndex == realIndexes[i]){
      correct = true;
    }

    SERIALPORT.printf("%lu,%d,%.2f,%.2f\n",timeTaken, correct, outputData[2*i + 0], outputData[2*i + 1]); // access is slightly weird bc it's a 1d array that's conceptually 2d
    // we can get the true angle measurements through code run on the desktop when we get the data
  }
}


void setup() {
  SERIALPORT.begin(115200); // begin USB serial connection

  SERIALPORT.println("Welcome to the Nearest Neighbor Performance Test Program. Press any key to begin.");

  while(SERIALPORT.available() == 0){}; // tight loop that works crossplatform to wait to begin program

  nnAlgorithm.setDimensionality(5);
  nnAlgorithm.setDatasetSize(sizeof(calibrationData) / sizeof(calibrationData[0]));
  nnAlgorithm.setDataset(calibrationData); 

  // first test: linear search with standard euclidian distance
  SERIALPORT.println("Linear Search - Euclidian Distance");
  nnAlgorithm.setSearchAlgorithm(LinearSearch);
  nnAlgorithm.setDistanceFunctionFloat(euclidianDistance);

  runTest();

  // second test: linear search with squared euclidian distance
  SERIALPORT.println("Linear Search - Squared Euclidian Distance");
  nnAlgorithm.setSearchAlgorithm(LinearSearch);
  nnAlgorithm.setDistanceFunctionInt(squaredEuclidianDistance);

  runTest();

  // third test: linear search with manhattan distance
  SERIALPORT.println("Linear Search - Manhattan Distance");
  nnAlgorithm.setSearchAlgorithm(LinearSearch);
  nnAlgorithm.setDistanceFunctionInt(manhattanDistance);

  runTest();


  // fourth test: k-d tree search with standard euclidian distance
  SERIALPORT.println("K-D Tree Search - Euclidian Distance");
  nnAlgorithm.setSearchAlgorithm(kdTree);
  nnAlgorithm.setDistanceFunctionFloat(euclidianDistance);

  nnAlgorithm.buildTree(); // reset and rebuild the tree with the new distance function

  runTest();

  // fifth test: k-d tree search with squared euclidian distance
  SERIALPORT.println("K-D Tree Search - Squared Euclidian Distance");
  nnAlgorithm.setSearchAlgorithm(kdTree);
  nnAlgorithm.setDistanceFunctionInt(squaredEuclidianDistance);

  nnAlgorithm.buildTree(); // reset and rebuild the tree with the new distance function

  runTest();

  // sixth test: k-d tree search with manhattan distance
  SERIALPORT.println("K-D Tree Search - Manhattan Distance");
  nnAlgorithm.setSearchAlgorithm(kdTree);
  nnAlgorithm.setDistanceFunctionInt(manhattanDistance);

  nnAlgorithm.buildTree(); // reset and rebuild the tree with the new distance function

  runTest();

  SERIALPORT.println("This concludes the performance testing.");
  SERIALPORT.println("Please send ALL the serial data to Nic DeBruin on Slack, along with the details of your tested microcontroller, including clockspeed and any other performance options.");
  SERIALPORT.println("Thanks for helping out!");
}

// unused since we only run once
void loop() {
  // put your main code here, to run repeatedly:
}
