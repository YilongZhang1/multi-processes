#include "fileParsing.h"

/** boot up the program here */

int main( int argc, char * args[] )
{
	if( argc < 3 ) 
	{
		printf("Error: incorrect count of parameters.\n");
		printf("Please input in this format: multiProcesses [number of processes] [the input file]\n");
		return 0;
	}
	
	int P = stoi(args[1]);
	if(P != 1 && P != 2 && P != 4 && P != 6 && P != 8)
	{
		printf("Please check if your process number is one of these {1,2,4,6,8}\n");
		return 0;
	}
	// parsing by calling the csvFileParser function
  cout<<"---------------------------------------------------------------------------"<<endl;
  cout<<"---------------------------------------------------------------------------"<<endl;
	cout<<endl<<"Child process count: "<<P<<", tackle the file: "<<args[2]<<endl;
	map<pair<float, float>, vector<float>> dataMap = csvFileParser(args[2]);
	// segment the data into 21 buckets and create the distance matrices for all of them
	vector<map<pair<pair<float,float>, pair<float, float>>, float>> Dist_Matrices = argMax_and_distanceMatrix(dataMap, P);

	return 0;
}
	
	

	
	
	
	
	