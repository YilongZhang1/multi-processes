#include "fileParsing.h"

#define PI 3.14159265



/**
args: 
	filename: path+filename of the csv file;
return: map(key, value), where key = (X, Y); value = vector of 21 features
*/
map<pair<float, float>, vector<float>> csvFileParser(char* filename)
{
	ifstream infile( filename );
	if( infile.is_open() == false )
	{
		cout<<"Error: cannot find the file, try again\n";
		return map<pair<float, float>, vector<float>>();
	}

	istringstream tempIStr;
	string tempStr;

	float locationInfo[4];		// to store lon:lat:delta_x:delta_y temporarily
	map<pair<float, float>, vector<float>> dataMap;		// store the parsed data in vec
	
/*
	// the first line are labels rather than data, kick it out.
	if(!getline( infile, tempStr, '\n' ))
		return dataMap;
*/	
	auto start = std::chrono::system_clock::now();	// start ticking
	cout<<"Start parsing the csv file...\n";
	
	int countLines = 0;
	while( !infile.eof() )
	{
		getline( infile, tempStr, '\n' );
		// in case the current line is empty
		if(tempStr.empty()) 
            break;
		countLines++;
		tempIStr.clear();			// flush the istringstream obj before re-using
		tempIStr.str( tempStr );
		
		vector<float> features;			// current line of float numbers
		int counter = 0;
		while(!tempIStr.eof())
		{
			getline(tempIStr, tempStr, ':');
			if(counter < 4)
				locationInfo[counter++] = atof(tempStr.c_str());
			else
				features.push_back(atof(tempStr.c_str()));
		}	
		float X = (locationInfo[2]/221200.0)*cos(locationInfo[1]*PI/180.0)+locationInfo[0];
		float Y = (locationInfo[3]/221200.0) + locationInfo[1];
		pair <float, float> key (X, Y);
		dataMap.insert( pair< pair<float, float>, vector<float> > (key, features) );
	}

	auto end = std::chrono::system_clock::now();	// stop ticking
	std::chrono::duration<double> elapsed_seconds = end-start;	
	
	// print stuff
	cout<<"File parsing finished!\nSUMMARY: \n";
	cout<<"\t\t 1. # of parsed lines: "<<countLines<<";"<<endl;
	cout<<"\t\t 2. # of data structure entries: "<<dataMap.size()<<";"<<endl;
	cout<<"\t\t 3. elapsed time in parsing the file: " << elapsed_seconds.count() << " s.\n\n";
	
	return dataMap;
}