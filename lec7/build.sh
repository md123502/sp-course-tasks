#!/bin/bash

mkdir ./build

gcc -Wall -pedantic -g -o ./build/test test.c student_w_ops.c students_array_w_ops.c
