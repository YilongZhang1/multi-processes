See PDF for full details.

Data Files on Teaching Cluster:

/data/scottgs/hpc_1m.csv
/data/scottgs/hpc_2m.csv
/data/scottgs/hpc_3m.csv
/data/scottgs/hpc_4m.csv
/data/scottgs/hpc_5m.csv
/data/scottgs/hpc_6m.csv
/data/scottgs/hpc_7m.csv
/data/scottgs/hpc_8m.csv
/data/scottgs/hpc_9m.csv

1. to run all the cases just run "sbatch analysis.sh" in command line.
2. timing results for all cases will be stored in results.out file after running completion.
3. if there is "srun: error: tc-m710-hpc0-node625: task 0: Out Of Memory" error, give up using srun, and directly run the executable.
4. since my memory cannot hold big distance matrices, I have to limit their sizes to (500*500). But if sizes of matrices are limited, comparisons of timings among cases of different parallelization will not make sense.
5. some timing statistics are shown below.

time in segmenting data into buckets:
     process count        file size(in million lines)      time (s)
------------------------------------------------------------------------     
           1                        1                       1.09343                    
           2                        1                       1.09486   
           4                        1                       1.35333
           6                        1                       1.28874 
           8                        1                       1.40717
           1                        2                       2.43882                    
           2                        2                       2.05233
           4                        2                       1.39837
           6                        2                       1.29372
           8                        2                       1.243
           1                        3                       3.70965
           2                        3                       2.69072
           4                        3                       2.14931
           6                        3                       2.01449
           8                        3                       1.9227
           1                        4                       5.13125
           2                        4                       3.59523
           4                        4                       2.81577
           6                        4                       2.61688
           8                        4                       2.42282
           1                        5                       5.97915
           2                        5                       4.54823
           4                        5                       3.44678
           6                        5                       3.16731
           8                        5                       3.05834 
           1                        6                       7.55047
           2                        6                       5.27574
           4                        6                       7.90553
           6                        6                       3.55515
           8                        6                       3.52317
           1                        7                       8.33837
           2                        7                       6.24576 
           4                        7                       4.97448
           6                        7                       4.43165
           8                        7                       4.26829
           1                        8                       10.3481
           2                        8                       7.20348
           4                        8                       5.72491
           6                        8                       5.10853
           8                        8                       4.87601
           1                        9                       11.4674
           2                        9                       8.26855
           4                        9                       6.40242
           6                        9                       5.81277
           8                        9                       5.46130
           
time in creating distance matrices (up to 500*500 in size):     
     process count        file size(in million lines)      time (s)
------------------------------------------------------------------------       
           1                        1                      9.24688
           2                        1                      9.37953            
           4                        1                      10.2441
           6                        1                      10.2284
           8                        1                      10.281
     
     
     
     
     
     
     