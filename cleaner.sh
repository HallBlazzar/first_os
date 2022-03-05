#!/bin/bash

ls -a > /dev/null
shopt -s dotglob
shopt -s nullglob

dirs=(*/)

for dir in "${dirs[@]}"; 
do 
    echo "Clean $dir"
    cd "$dir"
    make clean
    cd ..
done



