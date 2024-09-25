#! /usr/bin/env bash

filename=big_test_file
bs=256K
count=49152
flags=sync

for i in {1..5}; do
    echo "----RUN $i----"
    echo WRITE
    dd if=/dev/zero of=$filename bs=$bs count=$count oflag=$flags
    echo READ
    dd if=$filename of=/dev/null bs=$bs count=$count iflag=$flags
    rm $filename
done
