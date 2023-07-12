#! /bin/bash

iternum=1

rm -rf ./tmp &> /dev/null
rm -rf ./linux-4.9.108 &> /dev/null
mkdir tmp

for i in $(seq 1 $iternum)
do
	printf "Compression Benchmark - iteration %d: \n" $i

	START=$(date +%s%3N)
	
	result="$1/$2-result-$(date "+%d:%H:%M:%S").txt"

	printf "tar..\n"
	tar -zxvf linux-4.9.108.tar.gz &> /dev/null
    	tar -zcvf ./tmp/linux-4.9.108.tar.gz linux-4.9.108 &> /dev/null
    	rm -rf linux-4.9.108 ./tmp/linux-4.9.108.tar.gz

	printf "zip..\n"
	unzip linux-4.9.108.zip &> /dev/null
    	zip -r ./tmp/linux-4.9.108.zip linux-4.9.108 &> /dev/null
    	rm -rf linux-4.9.108 ./tmp/linux-4.9.108.zip

	printf "tar..\n"
	tar -zxvf linux-4.9.108.tar.gz &> /dev/null
    	tar -zcvf ./tmp/linux-4.9.108.tar.gz linux-4.9.108 &> /dev/null
    	rm -rf linux-4.9.108 ./tmp/linux-4.9.108.tar.gz

	printf "zip..\n"
	unzip linux-4.9.108.zip &> /dev/null
    	zip -r ./tmp/linux-4.9.108.zip linux-4.9.108 &> /dev/null
    	rm -rf linux-4.9.108 ./tmp/linux-4.9.108.zip

	rm -rf ./tmp

	END=$(date +%s%3N)
	
	echo $(bc <<< "scale=3;($END-$START)/1000") > $result
	printf "Runtime: %.3f secs\n\n" $(bc <<< "scale=3;($END-$START)/1000")
done
