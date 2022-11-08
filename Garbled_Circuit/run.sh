#!/bin/sh
num_parties=3

num_and_gates=0
num_xor_gates=0

while getopts 'a:x:' opt; do
    case "$opt" in
        a)
            num_and_gates=${OPTARG}
            echo "#AND Gates: $num_and_gates"
            ;;

        x)
            num_xor_gates=${OPTARG}
            echo "#XOR Gates: $num_xor_gates"
            ;;
    esac
done 

for (( i=1; i<=$num_parties; i++ ))
do
    ./Server/Server $i 12345 -a $num_and_gates -x $num_xor_gates &
done

wait
kill -9 $(lsof -t -i tcp:8888)