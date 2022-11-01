#!/bin/bash

ip3=$1
range_0=$2
range_1=$3

# echo $ip3
# echo $(($range_0 + 0))
# echo $range_1

s=$((range_0))
e=$((range_1))

for ip in $(seq $range_0 $range_1)
do
    if ping -i 0.2 -c 1 $ip3.$ip &> /dev/null
    then
        echo $ip3.$ip
fi
done


