#!/usr/bin/env bash

set -x
set -e
set -o pipefail

USAGE="compat_test.sh <path-to-encryptcli>"
if [ $# -lt 1 ]
then
    echo $USAGE
    exit 1
fi

PASSPHRASE_FILE="passphrase.txt"
CMD="$1"

TMP_DIR="./tmp"
ENC_DIR="./back_compat"

TEST_FILE=plain_text.txt

mkdir -p $TMP_DIR

cat $PASSPHRASE_FILE | $CMD -d --pwd-file $PASSPHRASE_FILE --key-pwd-fd 0 -o $TMP_DIR/out_file_1.txt $ENC_DIR/0_3_2_1_persist_key.epd
cat $PASSPHRASE_FILE | $CMD -d --pwd-file $PASSPHRASE_FILE --key-pwd-fd 0 -key 0_3_2_1.key -o $TMP_DIR/out_file_2.txt $ENC_DIR/0_3_2_1.epd
cat $PASSPHRASE_FILE | $CMD -d --key-pwd-fd 0 --key-only -o $TMP_DIR/out_file_3.txt $ENC_DIR/0_3_2_1_key_only.epd

for i in 1 2 3
do
    if diff $TEST_FILE $TMP_DIR/out_file_${i}.txt
    then
        echo ok
    else
        RESULT=1
        echo fail
        break
    fi
done

rm -f $TMP_DIR/out_file_1.txt
rm -f $TMP_DIR/out_file_2.txt
rm -f $TMP_DIR/out_file_3.txt

exit $RESULT
