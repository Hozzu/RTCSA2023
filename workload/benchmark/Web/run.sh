#! /bin/bash

iternum=1

for i in $(seq 1 $iternum)
do
	printf "Web Benchmark - iteration %d: \n" $i
	
	START=$(date +%s%3N)
	result="$1/$2-result-$(date "+%d:%H:%M:%S").txt"
    	python3 run-benchmark.py > $result

	WM_ID=$(wmctrl -lx | awk '$3~/chromium/ {print $1}') &> /dev/null
        wmctrl -i -c $WM_ID &> /dev/null

	END=$(date +%s%3N)
	printf "Runtime: %.3f secs\n\n" $(bc <<< "scale=3;($END-$START)/1000")
done
