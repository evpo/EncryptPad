#!/usr/bin/env bash

set -x
set -e
set -o pipefail

USAGE="crlf_end_of_line.sh <path-to-encryptcli>"
if [ $# -lt 1 ]
then
    echo $USAGE
    exit 1
fi

PASSPHRASE_FILE="passphrase.txt"
CMD="$1"

TMP_DIR="./tmp"

TEST_FILE=plain_text.txt
OUT_FILE=${TMP_DIR}/out_file.txt

mkdir -p $TMP_DIR
$CMD -d --pwd-file $PASSPHRASE_FILE -o $OUT_FILE crlf_end_of_line.asc

RESULT=0
if diff $TMP_DIR/out_file.txt $TEST_FILE
then
    echo ok
else
    RESULT=1
    echo fail
fi
rm -f $TMP_DIR/out_file.txt
exit $RESULT
