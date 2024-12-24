#!/bin/bash

cmake   -S . \
        -B build \
        -DTYPES="FAST_FIXED(32,16), FIXED(64,8)" \
        -DSIZES="S(10,10), S(34,86)"

cmake --build build

echo "STARTED"

./build/Fluid   --p-type="FAST_FIXED(32,16)" \
                --v-type="FAST_FIXED(32,16)" \
                --v-flow-type="FAST_FIXED(32,16)" \
                --field-path=base_field \
                --num-threads=8
