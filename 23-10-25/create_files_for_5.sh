#!/bin/bash

for i in {1..10}; do
    for j in $(seq 1 $i); do
        mkdir $i
        cd $i
    done
    touch $i.txt
    for j in $(seq 1 $i); do
        cd ..
    done
done
