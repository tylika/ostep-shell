#!/bin/bash

for i in $(seq 1 20); do
    if [ "$i" = "3" ]; then
        ./wish tests/3.in > /tmp/out_out 2> /tmp/err_out
        if grep -q "An error has occurred" /tmp/err_out; then
            echo "test 3: failed"
        else
            echo "test 3: passed"
        fi
        continue
    fi

    if [ "$i" = "13" ]; then
        ./wish tests/13.in tests/13.in 2> /tmp/err_out
    elif [ "$i" = "14" ]; then
        ./wish tests/no_such_file.in 2> /tmp/err_out
    else
        ./wish tests/$i.in > /tmp/out_out 2> /tmp/err_out
    fi

    if diff -q /tmp/err_out tests/$i.err > /dev/null; then
        echo "test $i: passed"
    else
        echo "test $i: failed"
    fi
done