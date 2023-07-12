#! /bin/bash

iternum=1

for i in $(seq 1 $iternum)
do
	printf "Video Benchmark - iteration %d: \n" $i

	START=$(date +%s%3N)
	result="$1/$2-result-$(date "+%d:%H:%M:%S").txt"

	for file in $(find . -name "*.mp4")
	do
		printf "playing %s..\n" $file
		totem $file &> /dev/null &
		sleep 170
		totem --quit
	done

	END=$(date +%s%3N)

	echo $(bc <<< "scale=3;($END-$START)/1000") > $result
        printf "Runtime: %.3f secs\n\n" $(bc <<< "scale=3;($END-$START)/1000")
done
