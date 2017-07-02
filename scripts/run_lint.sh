#!/bin/bash

script_folder=`dirname $0`

$script_folder/run_diff.sh '$script_folder/cpplint.py $file 2>&1 >/dev/null' "$@"
