#!/bin/bash

cmake   -S . \
        -B build \
        -DTYPES="FAST_FIXED(32,16), FIXED(64,8)" \
        -DSIZES="S(50,150), S(36,84)"

cmake --build build

echo "STARTED"

./build/Fluid   --p-type="FAST_FIXED(32,16)" \
                --v-type="FAST_FIXED(32,16)" \
                --v-flow-type="FAST_FIXED(32,16)" \
                --field-path=test_field \
                --num-threads=8
