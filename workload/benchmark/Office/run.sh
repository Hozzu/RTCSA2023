#! /bin/bash

iternum=1

responseTime=0

function openOffice(){
	libreoffice --norestore $1 &>/dev/null &

	openTime=0
	WM_ID=$(wmctrl -lx 2>/dev/null | awk '$3~/libreoffice/ {print $1}')
	while [[ $WM_ID == "" ]]
	do
        	sleep 0.05
		openTime=$(bc <<< "scale=2;$openTime+0.05")
		WM_ID=$(wmctrl -lx 2>/dev/null | awk '$3~/libreoffice/ {print $1}')
        done

	#printf "time consumed to open: %.2fs\n" $openTime
	
	if (( $(bc <<< "15>$openTime") ))
	then
		sleep $(bc <<< "scale=2;15-$openTime")
	fi

	wmctrl -i -c $WM_ID

	sleep 5
	
	responseTime=$(bc <<< "scale=2;$openTime+$responseTime")
}

for i in $(seq 1 $iternum)
do
	printf "Office Viewer Test Program - iteration %d: \n" $i

	START=$(date +%s%3N)

	result="$1/$2-result-$(date "+%d:%H:%M:%S").txt"

	for file in $(find . -name "*.odp" -o -name "*.ods" -o -name "*.odt")
	do
		printf "opening file %s..\n" $file
		openOffice $file
	done

	for file in $(find . -name "*.odp" -o -name "*.ods" -o -name "*.odt")
        do
                printf "opening file %s..\n" $file
                openOffice $file
        done

	END=$(date +%s%3N)

	echo $responseTime > $result

	printf "Runtime: %.3f secs\n\n" $(bc <<< "scale=3;($END-$START)/1000")
done
