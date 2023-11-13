#!/bin/bash

kill $(ps --ppid $(cat pid.txt) | grep -oE "^  [0-9]+ " | head -n 3)

