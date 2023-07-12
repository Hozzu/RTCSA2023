#! /bin/bash

iternum=1

for i in $(seq 1 $iternum)
do
	printf "DL test Program - iteration %d: \n" $i

	START=$(date +%s%3N)

	result="$1/$2-result-$(date "+%d:%H:%M:%S").txt"

	detectnet --network=ssd-inception-v2 --input-rate=60 Drive.mp4 &> /dev/null

	END=$(date +%s%3N)

	echo $(bc <<< "scale=3;($END-$START)/1000") > $result
	printf "Runtime: %.3f secs\n\n" $(bc <<< "scale=3;($END-$START)/1000")
done

