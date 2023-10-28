#!/bin/bash

create_files() {
    for i in {1..20}; do
        name="$i.txt"
        touch $name
        echo $name > $name
    done
}

remove_txt_files_with_no_num_in_name() {
    if [ $# -ne 1 ]; then
        echo "remove_txt_files_with_no_num_in_name: Need argument"
        exit 1
    fi

    local num=$1
    find . -name "*.txt" -and ! -name "*$num*.txt" -delete
}

rewrite_contents() {
    if [ $# -ne 2 ]; then
        echo "rewrite_contents: Need filename and new content"
        exit 1
    fi

    local filename=$1
    local content=$2
    local files_number_matching=$(ls | grep $filename | wc -l)

    if [ $files_number_matching -ne 1 ]; then
        echo "rewrite_contents: Incorrect filename"
        exit 2
    fi

    echo $content > $filename
}

create_files
remove_txt_files_with_no_num_in_name 4
rewrite_contents "14.txt" "Hello world"


