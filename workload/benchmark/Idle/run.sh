#! /bin/bash

iternum=1

for i in $(seq 1 $iternum)
do
	printf "Idle Test Program - iteration %d: \n" $i
	sleep 120
	printf "120 secs passed.\n\n"
done
