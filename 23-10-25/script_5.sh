#!/bin/bash

find . -regex ".*[0-9]+.txt" -exec ln -s {} \;
