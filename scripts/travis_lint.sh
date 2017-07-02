#!/bin/bash

set -e

script_folder=`dirname $0`
pip install --user unidiff

if ! [[ -e $script_folder/cpplint.py ]]
then
  echo "CPP Linter script could not be found in the $script_folder directory"
  echo "Ensure cpplint.py is inside the $script_folder directory then run again"
  exit 1
fi

if [ "$TRAVIS_PULL_REQUEST" == "false" ]; then
  $script_folder/run_diff.sh '$script_folder/cpplint.py $file 2>&1 >/dev/null' HEAD~1 # Check for errors introduced in last commit
else
  $script_folder/run_diff.sh '$script_folder/cpplint.py $file 2>&1 >/dev/null' $TRAVIS_BRANCH # Check for errors compared to merge target
fi
