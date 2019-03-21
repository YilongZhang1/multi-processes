#! /bin/bash

#SBATCH -o results2.out  # job output
#SBATCH -e errors.log # job error output

#SBATCH -N 1  # number of nodes
#SBATCH -n 1  # number of cores (AKA tasks)
#SBATCH -c 1  # number of cpus per task

#SBATCH --mem 10G
#SBATCH -t 0-02:00  # two hour time limit

cmake3 .
make

# remove possible ipcs left by previous work
ipcrm -a

srun ./multiProcesses 1 /data/scottgs/hpc_1m.csv
srun ./multiProcesses 2 /data/scottgs/hpc_1m.csv
srun ./multiProcesses 4 /data/scottgs/hpc_1m.csv
srun ./multiProcesses 6 /data/scottgs/hpc_1m.csv
srun ./multiProcesses 8 /data/scottgs/hpc_1m.csv

srun ./multiProcesses 1 /data/scottgs/hpc_2m.csv
srun ./multiProcesses 2 /data/scottgs/hpc_2m.csv
srun ./multiProcesses 4 /data/scottgs/hpc_2m.csv
srun ./multiProcesses 6 /data/scottgs/hpc_2m.csv
srun ./multiProcesses 8 /data/scottgs/hpc_2m.csv

srun ./multiProcesses 1 /data/scottgs/hpc_3m.csv
srun ./multiProcesses 2 /data/scottgs/hpc_3m.csv
srun ./multiProcesses 4 /data/scottgs/hpc_3m.csv
srun ./multiProcesses 6 /data/scottgs/hpc_3m.csv
srun ./multiProcesses 8 /data/scottgs/hpc_3m.csv

srun ./multiProcesses 1 /data/scottgs/hpc_4m.csv
srun ./multiProcesses 2 /data/scottgs/hpc_4m.csv
srun ./multiProcesses 4 /data/scottgs/hpc_4m.csv
srun ./multiProcesses 6 /data/scottgs/hpc_4m.csv
srun ./multiProcesses 8 /data/scottgs/hpc_4m.csv

srun ./multiProcesses 1 /data/scottgs/hpc_5m.csv
srun ./multiProcesses 2 /data/scottgs/hpc_5m.csv
srun ./multiProcesses 4 /data/scottgs/hpc_5m.csv
srun ./multiProcesses 6 /data/scottgs/hpc_5m.csv
srun ./multiProcesses 8 /data/scottgs/hpc_5m.csv

srun ./multiProcesses 1 /data/scottgs/hpc_6m.csv
srun ./multiProcesses 2 /data/scottgs/hpc_6m.csv
srun ./multiProcesses 4 /data/scottgs/hpc_6m.csv
srun ./multiProcesses 6 /data/scottgs/hpc_6m.csv
srun ./multiProcesses 8 /data/scottgs/hpc_6m.csv

srun ./multiProcesses 1 /data/scottgs/hpc_7m.csv
srun ./multiProcesses 2 /data/scottgs/hpc_7m.csv
srun ./multiProcesses 4 /data/scottgs/hpc_7m.csv
srun ./multiProcesses 6 /data/scottgs/hpc_7m.csv
srun ./multiProcesses 8 /data/scottgs/hpc_7m.csv

srun ./multiProcesses 1 /data/scottgs/hpc_8m.csv
srun ./multiProcesses 2 /data/scottgs/hpc_8m.csv
srun ./multiProcesses 4 /data/scottgs/hpc_8m.csv
srun ./multiProcesses 6 /data/scottgs/hpc_8m.csv
srun ./multiProcesses 8 /data/scottgs/hpc_8m.csv

srun ./multiProcesses 1 /data/scottgs/hpc_9m.csv
srun ./multiProcesses 2 /data/scottgs/hpc_9m.csv
srun ./multiProcesses 4 /data/scottgs/hpc_9m.csv
srun ./multiProcesses 6 /data/scottgs/hpc_9m.csv
srun ./multiProcesses 8 /data/scottgs/hpc_9m.csv