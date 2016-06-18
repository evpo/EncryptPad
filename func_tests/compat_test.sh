#!/bin/bash

set -e
set -o pipefail

USAGE="compat_test.sh <path-to-encryptcli>"
if [ $# -lt 1 ]
then
    echo $USAGE
    exit 1
fi

PASSPHRASE_FILE="passphrase.txt"
CMD="$1 --pwd-file $PASSPHRASE_FILE"

TMP_DIR="./tmp"
ENC_DIR="./back_compat"

TEST_FILE=plain_text.txt

mkdir -p $TMP_DIR
cat $PASSPHRASE_FILE | $CMD -d -k $ENC_DIR/key0_3_2_1.key --key-pwd-fd 0 -o $TMP_DIR/out_file.txt $ENC_DIR/plain_text0_3_2_1.epd

if diff $TEST_FILE $TMP_DIR/out_file.txt
then
    echo ok
else
    RESULT=1
    echo fail
fi
rm $TMP_DIR/out_file.txt
exit $RESULT
