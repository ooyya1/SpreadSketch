#!/bin/bash

for i in `seq 0 0.1 1.5`;
do
    ./src_zipf 2 $i >> ss_src_skew_new
done
