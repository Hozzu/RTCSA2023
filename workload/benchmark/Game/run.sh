#! /bin/bash

openarena=/usr/games/openarena
opengl=libGLX_nvidia.so.0

iternum=1

for i in $(seq 1 $iternum)
do
	printf "Game Benchmark - iteration %d: \n" $i

	START=$(date +%s%3N)

	result="$1/$2-result-$(date "+%d:%H:%M:%S").txt"

	LIBFRAMETIME_FILE=./openarena.record LD_PRELOAD="libframetime.so $opengl" DBUS_FATAL_WARNINGS=0 $openarena +exec pts-openarena-088 &> /dev/null

	./stats.awk < openarena.record | awk '/FPS/{print $5}' > $result

	rm -f ./openarena.record

	END=$(date +%s%3N)

	printf "Runtime: %.3f secs\n\n" $(bc <<< "scale=3;($END-$START)/1000")
done
