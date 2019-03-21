#include "fileParsing.h"
#define PI 3.14159265
#define R 6378137.0		// earth's radius in meter

// since it's hard to have random access to data in a map, how about we first convert it to a 2D array?
float** convert_map_array(map<pair<float, float>, vector<float>> dataMap)
{
	int mapRows = dataMap.size();
	float** dataArray = new float*[mapRows];
	for(int i = 0; i < mapRows; i++)
		*(dataArray+i) = new float[23];		// format in a row: X, Y, feature0,..., feature21
	
	int rowCnt = 0;
	for (map<pair<float, float>, vector<float>>::iterator iter = dataMap.begin(); iter != dataMap.end(); iter++)
	{
		*(*(dataArray+rowCnt)) = iter->first.first;
		*(*(dataArray+rowCnt)+1) = iter->first.second;
		vector<float> curVec = iter->second;	// access the current vector
		for(int i = 0; i < 21; i++)
			*(*(dataArray+rowCnt)+i+2) = curVec.at(i);
		rowCnt++;
	}	
	return dataArray;
	// remember to free dataArray after usage by calling clear_dataArray
}

void clear_dataArray(float** dataArray, int len)
{
	for(int i = 0; i < len; i++)
		delete[] *(dataArray+i);
	delete[] dataArray;
}


/**
args: Buckets: output from argMax;
return: vector of distance matrices
*/
float Haversine(pair<float, float> M, pair<float, float> N)
{
	float M_lon = M.first*PI/180, M_lat = M.second*PI/180;
	float N_lon = N.first*PI/180, N_lat = N.second*PI/180;
	return 2.0*R*asin(sqrt(pow(sin((M_lat-N_lat)/2.0), 2) + cos(M_lat)*cos(N_lat)*pow(sin((M_lon-N_lon)/2.0), 2)));
}


// assign the task of generating distance matrices to each process
vector<vector<int>> assignTask(long long* elemCount, int P)
{
	// convert elemCount into a 2D array
	long long elemCountArr[21][2];
	for(int i = 0; i < 21; i++)
	{
		elemCountArr[i][0] = pow(*(elemCount+2*i), 2);	// # of elements in the result distance matrix
		elemCountArr[i][1] = *(elemCount+2*i+1);		// index of bucket
	}

	// sort elemCountArr inversely
	long long temp[2];
	for(int i = 0; i < 20; i++)
		for(int j = i+1; j < 21; j++)
			if(elemCountArr[i][0] < elemCountArr[j][0])
			{
				temp[0] = elemCountArr[i][0]; temp[1] = elemCountArr[i][1];
				elemCountArr[i][0] = elemCountArr[j][0]; elemCountArr[i][1] = elemCountArr[j][1];
				elemCountArr[j][0] = temp[0]; elemCountArr[j][1] = temp[1];
			}

	// now assign the task for each process
	long long* accumulateBurden = new long long[P];	// accumulated burden for each process
	
	// procMatrix is a 2D array, assigning matrix # to processes
	int** procMatrix = new int*[P];
	// initialize procMatrix	
	for(int i = 0; i < P; i++)
	{
		*(procMatrix+i) = new int[21];
		for(int j = 0; j < 21; j++)
			procMatrix[i][j] = -1;
	}

	// first, assign one matrix task to each process	
	for(int i = 0; i < P; i++)
	{
		accumulateBurden[i] = elemCountArr[i][0];
		procMatrix[i][0] = elemCountArr[i][1];
	}
	// second, iteratively add one task to the process with the least burden
	for(int i = P; i < 21; i++)
	{
		long long min = accumulateBurden[0];
		int pos = 0;	// index of process
		for(int j = 1; j < P; j++)
		{
			if(min > accumulateBurden[j])
			{
				min = accumulateBurden[j];
				pos = j;
			}
		}
		// now, we get the index of the process with least burden
		accumulateBurden[pos] += elemCountArr[i][0];
		int k = 0;
		while(procMatrix[pos][k] != -1)
			k++;
		procMatrix[pos][k] = elemCountArr[i][1];
	}
	
	delete[] accumulateBurden;
	vector<vector<int>> procTaskVec;
	// how about printing out the assignment for each process?
	cout<<endl<<"\t Note that the distance matrices are not equal in size for each bucket,"<<endl<<"\t   thus processes should intelligently share the burden.\n";
	cout<<"\t burden assignment result: "<<endl;
	for(int i = 0; i < P; i++)
	{
		vector<int> curProcTaskVec;
		int j = 0;
		cout<<"\t\t child process "<<i<<" will tackle bucket: ";
		while(j < 21 && procMatrix[i][j] != -1)
		{
			curProcTaskVec.push_back(procMatrix[i][j]);
			cout<<procMatrix[i][j]<<"  ";
			j++;
		}
		cout<<endl;
		procTaskVec.push_back(curProcTaskVec);
		delete[] *(procMatrix+i);
	}
	delete[] procMatrix;
	return procTaskVec;
}


/**
args:
	dataMap: output from csvFileParser;
	P: number of processes in calculating argmax;
return: a pair of float pointer points to shared memory. First denotes counts of elements in each bucket; second denotes the data in each bucket.
*/
vector<map<pair<pair<float,float>, pair<float, float>>, float>> argMax_and_distanceMatrix(map<pair<float, float>, vector<float>> dataMap, int P)
{
	// to traverse part of the map, we have to first convert it to an array
	float** dataArray = convert_map_array(dataMap);
	auto start1 = std::chrono::system_clock::now();	// start ticking
	cout<<"---------------------------------------------------------------------------"<<endl;
	cout<<endl<<"Now segment the (X, Y) tuples into 21 buckets based on argmax...\n";	
	// ==================================================================================
	// BEGIN: Do the Shared Memory Segment Setup
	// ==================================================================================	
	cout<<"\t configure the shared memory for 21 buckets...\n";
	
	int shmId1[22]; 		// ID of shared memory segment. 1st one for elemCount, the other 21 for each bucket	
	key_t shmKey1 = 5362231;			// key to pass to shmget(), key_t is an IPC key type defined in sys/types
	int shmFlag1 = IPC_CREAT | 0666; // Flag to create with rw permissions

	// This will be shared:
	long long * elemCount;	// record # of elements in each bucket as well as its index
	float* buckets[21];		// store the real data here
	
	// data size in shared memory
	int mapRows = dataMap.size();	
	int bucket_size = mapRows * 2;		// store (X, Y) tuple is enough. Since we don't know bucket size, we can only take use of its worst case.

	// allocate shared memory for elemCount
	if ((shmId1[0] = shmget(shmKey1, 21*2*sizeof(long long), shmFlag1)) < 0)
	{
		std::cerr << "Init: Failed to initialize shared memory (" << shmId1[0] << ")" << std::endl; 
		clear_dataArray(dataArray, mapRows);
		exit(1);
	}
	if ((elemCount = (long long *)shmat(shmId1[0], NULL, 0)) == (long long *) -1)
	{
		std::cerr << "Init: Failed to attach shared memory (" << shmId1[0] << ")" << std::endl; 
		clear_dataArray(dataArray, mapRows);
		exit(1);
	}	
	
	// then allocate shared memory for each pointer of buckets
	for(int i = 0; i < 21; i++)
	{
		if ((shmId1[i+1] = shmget(shmKey1+i+1, bucket_size*sizeof(float), shmFlag1)) < 0)
		{
			std::cerr << "Init: Failed to initialize shared memory (" << shmId1[i+1] << ")" << std::endl; 
			clear_dataArray(dataArray, mapRows);
			exit(1);
		}	
		if ((buckets[i] = (float *)shmat(shmId1[i+1], NULL, 0)) == (float *) -1)
		{
			std::cerr << "Init: Failed to attach shared memory (" << shmId1[i+1] << ")" << std::endl; 
			clear_dataArray(dataArray, mapRows);
			exit(1);
		}		
	}
	
	cout<<"\t shared memory configuration for buckets finished!\n";
	// ==================================================================================
	// END: Do the Shared Memory Segment Setup
	// ==================================================================================
	
	// initialize elemCount
	for(int i = 0; i < 21; i++)
	{
		*(elemCount+2*i) = 0;		// counts in each bucket
		*(elemCount+2*i+1) = i;		// index of each bucket
	}
	
	// ==================================================================================
	// BEGIN: Do the Semaphore Setup
	// The semaphore will be a mutex for each bucket
	// ==================================================================================
	cout<<"\t configure semaphores for 21 buckets...	\n";
	int semId[21]; 					// ID of semaphore set
	key_t semKey = 953241; 		// key to pass to semget(), key_t is an IPC key type defined in sys/types
	int semFlag = IPC_CREAT | 0666; // Flag to create with rw permissions

	int semCount = 1; 		// number of semaphores to pass to semget(), equals to number of buckets
	int numOps = 1; 		// number of operations to do	
	
	// Initialize the semaphore
	union semun 
	{
		int val;
		struct semid_ds *buf;
		ushort * array;
	} argument;

	argument.val = 1; // NOTE: We are setting this to one to make it a MUTEX
			
	// Create the semaphore
	// The return value is the semaphore set identifier
	for(int i = 0; i < 21; i++)
	{
		if ((semId[i] = semget(semKey+i, semCount, semFlag)) == -1)
		{
			std::cerr << "Failed to semget(" << semKey+i << "," << semCount << "," << semFlag << ")" << std::endl;
			clear_dataArray(dataArray, mapRows);
			exit(1);
		}		
		if( semctl(semId[i], 0, SETVAL, argument) < 0)
		{
			std::cerr << "Init: Failed to initialize (" << semId[i] << ")" << std::endl; 
			clear_dataArray(dataArray, mapRows);
			exit(1);
		}		
	}

	cout<<"\t semaphore configuration for buckets finished!\n";
	// ==================================================================================
	// END: Do the Semaphore Setup
	// ==================================================================================

	// start and end row index for each process
	int* start_row = new int[P];
	int* end_row = new int[P];
	int interval = mapRows / P;
	
	*(start_row) = 0;
	for(int s = 1; s < P; s++)
	{
		*(start_row+s) = interval*s;
		*(end_row+s-1) = interval*s - 1;
	}
	*(end_row+P-1) = mapRows-1;

	if(P == 1)
	{
		cout<<"\t create "<<P<<" child process...\n";
		cout<<"\t "<<P<<" child process successfully created! Child process\n";
	}		
	else
	{
		cout<<"\t create "<<P<<" children processes...\n";
		cout<<"\t "<<P<<" children processes created! Children processes\n";
	}
	cout<<"\t details:"<<endl;
	for(int num1 = 0; num1 < P; num1++)
	{
		pid_t pid1 = fork();	// fork, which replicates the process 
		if ( pid1 < 0 )
		{ 
			std::cerr << "Could not fork!!! ("<< pid1 <<")" << std::endl;
			clear_dataArray(dataArray, mapRows);
			delete[] start_row;
			delete[] end_row;
			exit(1);
		}  
		else if ( pid1 == 0 ) // Child process, do the argmax work!
		{
			cout<<"\t\t child process "<<num1<<", pid = "<<getpid()<<", starts its work..."<<endl;
			for(int i = start_row[num1]; i <= end_row[num1]; i++)
			{
				float max = *(*(dataArray+i)+2);	// initialize max to feature0
				int pos = 0;						// pos denotes bucket #
				for(int j = 1; j < 21; j++)
				{
					float curVal = *(*(dataArray+i)+2+j);
					if( max < curVal )
					{
						max = curVal;
						pos = j;
					}					
				}

				// now we are clear which bucket to drop our (X,Y) tuple, right?
				// go ahead and set up the sembuf structure
				struct sembuf operations[1];
				operations[0].sem_num = 0; 	// use the corresponding bucket #
				operations[0].sem_flg = 0 ;	// allow the calling process to block and wait
				operations[0].sem_op = -1; 	// this is the operation... the value is added to semaphore (a P-Op = -1)

				int retval = semop(semId[pos], operations, numOps);
				if(0 == retval)
				{
					// drop (X, Y) into corresponding bucket, denoted by pos
					*( *(buckets+pos) + (*(elemCount+2*pos))*2 ) = *(*(dataArray+i));			// copy X from dataArray
					*( *(buckets+pos) + (*(elemCount+2*pos))*2 + 1 ) = *(*(dataArray+i) + 1);	// copy Y from dataArray
					(*(elemCount+2*pos))++;
				}
				else
				{
					std::cerr << "In the child (if): Failed P-operation on (" << semId << ")" << std::endl; 
					clear_dataArray(dataArray, mapRows);
					delete[] start_row;
					delete[] end_row;				
					_exit(1);
				}	
				
				// Release the semaphore (V-op)
				operations[0].sem_op = 1; 	// this the operation... the value is added to semaphore (a V-Op = 1)
				retval = semop(semId[pos], operations, numOps);
				if(0 == retval)
				{	}
				else
				{
					std::cerr << "In the child (if): Failed V-operation on (" << semId << ")" << std::endl; 
				}			
			}
			_exit(0);
		}
		else		// Parent Process
		{
			// do nothing, just wait for the child processes to complete
		}		
    }	
	
	int status1 = 0;
	while ((wait(&status1)) > 0); // this way, the father waits for all the child processes 
	if(P == 1)
		cout<<"\t work done! child process is recycled!"<<endl;	
	else
		cout<<"\t work done! children processes are recycled!"<<endl;
	
	auto end1 = std::chrono::system_clock::now();	// stop ticking
	std::chrono::duration<double> elapsed_seconds1 = end1-start1;	
	
	cout<<"Segmenting finished!\n";
	cout<<"SUMMARY:\n";	
	cout<<"\t Elapsed time in segmenting data into buckets: " << elapsed_seconds1.count() << " s\n";
	cout<<"\t # of (X, Y) tuples in each bucket are: \n";
	int total = 0;
	for(int i = 0; i < 21; i++)
	{
		total += *(elemCount+2*i);
		cout<<"\t\t bucket "<<i<<":\t"<<*(elemCount+2*i)<<endl;
	}
	cout<<"\t\t Total: "<<total<<endl;
		
	// free all memory before leaving
	delete[] start_row;
	delete[] end_row;	
	clear_dataArray(dataArray, mapRows);

	// the semaphores can be deleted now
	for(int i = 0; i < 21; i++)
		semctl(semId[i], 0, IPC_RMID);









	
	
	/// making the distance matrices below
	// cut down size of matrices for test purpose 
	cout<<"\t big matrices will blow up the heap, I must cut them down to (500*500)"<<endl;
/*	for(int i = 0; i < 21; i++)
	if(*(elemCount+2*i) > 500)
		*(elemCount+2*i) = i+1;
*/
	auto start2 = std::chrono::system_clock::now();	// start ticking
	cout<<endl<<"---------------------------------------------------------------------------"<<endl;	
	cout<<endl<<"Now calculate distance matrices for (X, Y) tuples in 21 buckets...\n";

	// assign tasks for each process to make it as even as possible
	vector<vector<int>> procTaskVec = assignTask(elemCount, P);		
	// ==================================================================================
	// BEGIN: Do the Shared Memory Segment Setup
	// ==================================================================================
	cout<<"\t configure the shared memory for 21 distance matrices..."<<endl;
	int shmId2[21]; 			// ID of shared memory segment
	key_t shmKey2 = 7761572; 		// key to pass to shmget(), key_t is an IPC key type defined in sys/types
	int shmFlag2 = IPC_CREAT | 0666; // Flag to create with rw permissions
	
	// This will be shared:
	float* matrices[21];	//matrices store the addresses of 21 matrices
	for(int i = 0; i < 21; i++)
	{
		if ((shmId2[i] = shmget(shmKey2+i, pow(*(elemCount+2*i), 2) * sizeof(float), shmFlag2)) < 0)
		{
			std::cerr << "Init: Failed to initialize shared memory (" << shmId2[i] << ")" << std::endl; 
			exit(1);
		}
		if ((matrices[i] = (float *)shmat(shmId2[i], NULL, 0)) == (float *) -1)
		{
			std::cerr << "Init: Failed to attach shared memory (" << shmId2[i] << ")" << std::endl; 
			exit(1);
		}
	}
	cout<<"\t shared memory configuration for distance matrices finished!"<<endl;	
	// ==================================================================================
	// END: Do the Shared Memory Segment Setup
	// ==================================================================================	
	
	// in calculating the distances, there is no need to set mutex among them.
	for(int num2 = 0; num2 < P; num2++)
	{
		pid_t pid2 = fork();	// fork, which replicates the process 
		if ( pid2 < 0 )
		{ 
			std::cerr << "Could not fork!!! ("<< pid2 <<")" << std::endl;
			exit(1);
		}
		else if(pid2 == 0)
		{
			cout<<"\t\t child process "<<num2<<", pid = "<<getpid()<<" starts its work..."<<endl;
			for(int i = 0; i < (int)procTaskVec.at(num2).size(); i++)
			{
				int curBucketIdx = procTaskVec.at(num2).at(i);
				int offset = *(elemCount+2*curBucketIdx);		// equals to column size of current matrix
				int size = offset;
//				cout<<"\t child "<<num2<<" now i = "<<i<<", curBucketIdx = "<<curBucketIdx<<", offset = "<<offset<<endl;
				for(int j = 0; j < size; j++)
				{
					for(int k = 0; k < size; k++)
					{
						if(k > j)
						{						
							pair<float, float> point1 (*(*(buckets + curBucketIdx)+2*j), *(*(buckets + curBucketIdx)+2*j+1));
							pair<float, float> point2 (*(*(buckets + curBucketIdx)+2*k), *(*(buckets + curBucketIdx)+2*k+1));
							*(*(matrices+curBucketIdx) + j*offset + k) = Haversine(point1, point2);	
						}
						else if (k == j)
							*(*(matrices+curBucketIdx) + j*offset + k) = 0;
						else
							*(*(matrices+curBucketIdx) + j*offset + k) = *(*(matrices+curBucketIdx) + k*offset + j);
					}
				}
//				cout<<"\t\t child process "<<num2<<" created distance matrix "<<curBucketIdx<<endl;
			}
			exit(0);			
		}
		else
		{
			// do nothing. Go ahead and fork more processes
		}
    }	

	int status2 = 0;
	while ((wait(&status2)) > 0); // this way, the father waits for all the child processes 	
	if(P == 1)
		cout<<"\t child process is recycled!"<<endl;	
	else
		cout<<"\t children processes are recycled!"<<endl;
	
	cout<<"\t all distance matrices are created in shared memory!\n";
	cout<<"\t now, copy the distance matrices from shared memory back to parent process,"<<endl<<"\t   and build the std::map data structure...\n";
	// map (point1, point2) --> distance between them
	vector<map<pair<pair<float,float>, pair<float, float>>, float>> Dist_Matrices;
	for(int i = 0; i < 21; i++)
	{
		map<pair<pair<float,float>, pair<float, float>>, float> Dist_matrix;
		int curBucketSize = *(elemCount+2*i);
		int offset = curBucketSize;
		// calculate distances for each bucket
		for(int j = 0; j < curBucketSize; j++)
			for(int k = 0; k < curBucketSize; k++)
				if(j != k)	// no need to store elements along the diagonals
				{
					pair<float,float> M(*(*(buckets+i)+2*j), *(*(buckets+i)+2*j+1));
					pair<float,float> N(*(*(buckets+i)+2*k), *(*(buckets+i)+2*k+1));
					pair<pair<float,float>, pair<float, float>> MN(M, N);				
					Dist_matrix.insert(pair<pair<pair<float,float>, pair<float, float>>, float>(MN, *(*(matrices+i) + j*offset + k)));
				}

		Dist_Matrices.push_back(Dist_matrix);
//		cout<<"\t\t distance matrix "<<i<<" copied...\n";
	}
	
	auto end2 = std::chrono::system_clock::now();	// stop ticking
	std::chrono::duration<double> elapsed_seconds2 = end2-start2;	
	cout<<"\t the map structure is created!\n";
	cout<<"Distance matrices calculations done!\n";
	cout<<"SUMMARY:\n";	
	cout<<"\t Elapsed time in calculating distance matrices and constructing maps: " << elapsed_seconds2.count() << " s\n\n";
	
	// now we have to detach and free the shared memory before exiting the program
	for(int i = 0; i < 21; i++)
	{
		shmdt(matrices[i]);
		shmctl(shmId2[i], IPC_RMID, NULL);
	}
	// now it's safe to free elemCount and buckets
	for(int i = 0; i < 22; i++)
	{
		if(i == 0)
			shmdt(elemCount);	
		else
			shmdt(buckets[i-1]);	
		shmctl(shmId1[i], IPC_RMID, NULL);	
	}

	return Dist_Matrices;
}




