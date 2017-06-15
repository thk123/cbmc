#Usage: extract.sh cbmc-test-dir

function match {
 if [[ $1 =~ \^EXIT=.*\$ ]]; then
  if [ $2 -eq 1 ]; then
   echo "ERROR, NEGATIVE MATCH ON EXIT" > /dev/stderr
  fi
  exitcode=`echo "$1" | sed "s/^[^=]*=\(.*\).$/\1/"`
  echo "* Exit code matches \"$exitcode\""
 elif [[ $1 =~ \^SIGNAL=.*\$ ]]; then
  if [ $2 -eq 1 ]; then
   echo "*ERROR, NEGATIVE MATCH ON SIGNAL" > /dev/stderr
  fi
  exitcode=`echo "$1" | sed "s/^[^=]*=\(.*\).$/\1/"`
  echo "* Signal code matches \"$exitcode\""
 elif [ $2 -eq 0 ];
 then
   if [ $3 -eq 0 ];
   then
    echo "* Match \"${1//\"/\\\"/}\""
   else
    echo "* Match multiline \"$1\""
   fi
 else
   if [ $3 -eq 0 ];
   then
    echo "* Do not match \"$1\""
   else
    echo "* Do not match multiline \"$1\""
   fi
 fi
}

function extractBinary {
 binaryraw=`grep "test.pl -c " $regdirfull/Makefile | head -n 1 | sed "s/^.*test.pl -c [\"]*\([.a-zA-Z0-9\/_-]*\).*$/\1/"`
 if [[ $binaryraw =~ ../../../src/.* ]]; then
  binary="${binaryraw:9}"
 else
  binary="$regdir/${binaryraw:3}"
 fi
 echo "* Binary is \"$binary\""
}

for regdirfull in $1/*/; do
regdir=`basename $regdirfull`;
 for filename in `find $regdirfull -name test.desc`; do
  dir=`dirname $filename`
  testname=`basename $dir`
  (
   echo "# $regdir/$testname";
   echo
   extractBinary $regdirfull $regdir
   echo
   echo "## Scenario"
   read level
   read input
   read options

   echo
   echo "Tags: $level"
   echo
   echo "* Run with options \"$options\" on file \"$input\" in directory \"regression/$regdir/$testname\""
   inverted=0
   multiline=0
   read -r match
   if [ "$match" == "bla" ];
   then
    multiline=1;
    echo
   elif [ "$match" == "--" ];
   then
    inverted=1
   else
    match "$match" $inverted $multiline
   fi

   while read -r match; do
    if [ "$match" == "--" ];
    then
     inverted=1
    else
     match "$match" $inverted $multiline
    fi
   done
   echo
  ) > $regdirfull/${testname}.spec < $filename
 done
done
