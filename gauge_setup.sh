#!/bin/bash
# Copyright 2016-2017 DiffBlue Limited. All Rights Reserved.

log_file=gauge_env.log
libs_only=false

# Check that the previous command succeded, if not exit.
command_status()
{
  if [ $? -ne 0 ]; then
    printf "[ERROR]\n"
    echo "ERROR: See ./logs/$log_file for more information"
    exit 1
  fi
  printf "[OK]\n"
}

# Print usage
print_usage()
{
  echo "Usage: $0 [-l]"
  echo "    -l Download libraries only (if you already have gauge installed)"
}

#Check the options that the user provided
while getopts ":l" opt
do
  case $opt in
    l)
      libs_only=true
      ;;
    \?)
      echo "ERROR: Invalid option: -$opt"
      print_usage
      exit 1
      ;;
    esac
done

printf "INFO: Removing existing logs "
rm -rf logs
command_status

printf "INFO: Creating the logs directory "
mkdir logs
command_status

printf "INFO: Creating the libs directory [OK]\n"
mkdir libs

date > ./logs/$log_file 2>&1

if [ $libs_only == false ]; then
  printf "INFO: Installing Java plugin "
  gauge --install java >> ./logs/$log_file 2>&1
  command_status
fi

printf "INFO: Starting to get libraries "
cd libs
command_status

printf "INFO: Downloading json libaries "
wget --no-check-certificate https://storage.googleapis.com/google-code-archive-downloads/v2/code.google.com/json-simple/json-simple-1.1.1.jar >> ../logs/$log_file 2>&1
command_status

printf "INFO: Downloading testng library "
wget --no-check-certificate https://oss.sonatype.org/content/repositories/snapshots/org/testng/testng/6.9.14-SNAPSHOT/testng-6.9.14-20161210.180517-52.jar >> ../logs/$log_file 2>&1
command_status