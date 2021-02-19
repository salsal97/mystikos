#!/bin/bash

function getline() {
    LNO=$1
    LINE=$(cat failed.txt | head -$1 | tail -1)
}

for test in $(cat failed.txt); do
    2>&1 make TEST=$test 
done;