#! /bin/bash

iternum=1

for i in $(seq 1 $iternum)
do
	printf "Synthetic Workload - iteration %d: \n" $i

	START=$(date +%s)
	./synthetic_cpu_workload 10 6
	END=$(date +%s)

	printf "Runtime: %d secs\n\n" $((END-START))
done
