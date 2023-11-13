#!/bin/bash

echo $$ > pid.txt

for i in {1..10}; do
	sleep 300 &
done

sleep 3600


