#!/bin/bash

set -e

script_folder=`dirname $0`
pip install --user unidiff

doxygen --version &> /dev/null
if [[ $? -ne 0 ]]
then
  echo "doxygen not installed"
  echo "Ensure doxygen is installed, then run again"
  exit 1
fi

cd src && doxygen &> ../doxygen.log && cd ..

if [ "$TRAVIS_PULL_REQUEST" == "false" ]; then
  $script_folder/run_diff.sh 'cat doxygen.log' HEAD~1 # Check for errors introduced in last commit
else
  $script_folder/run_diff.sh 'cat doxygen.log' $TRAVIS_BRANCH # Check for errors compared to merge target
fi
