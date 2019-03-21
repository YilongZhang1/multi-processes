#ifndef FILEPARSING_H__
#define FILEPARSING_H__


#include <iostream>     // std::cout
#include <sstream>      // std::istringstream
#include <fstream>
#include <vector>
#include <map>
#include <utility>      // std::pair, std::make_pair
#include <string>
#include <stdlib.h>		/* atoi */
#include <math.h>       /* cos */
#include <limits.h>		// INT_MAX and INT_MIN
#include <chrono>		// timing the processes


#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <unistd.h>
#include <cstdlib>
#include <cstdint>
#include <cstring>


#define COLUMNS 21		// # of columns of the feature data in each line 

using namespace std;

// function prototypes
/**
args: 
	filename: path to file of the csv file;
return: map(key, value), where key = (X, Y); value = vector of 21 features
*/
map<pair<float, float>, vector<float>> csvFileParser(char* filename);

/**
args: dataMap: output from csvFileParser;
return: vector of distance matrices
*/
vector<map<pair<pair<float,float>, pair<float, float>>, float>> argMax_and_distanceMatrix(map<pair<float, float>, vector<float>> dataMap, int P);

// for the single process case
//vector<vector<pair<float, float>>> argMax_single (map<pair<float, float>, vector<float>> dataMap);
//vector<map<pair<pair<float,float>, pair<float, float>>, float>> distanceMatrix_single (vector<vector<pair<float, float>>> Buckets);

#endif