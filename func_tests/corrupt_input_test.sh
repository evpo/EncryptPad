#!/bin/bash

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
ENC_DIR="./corrupt_input"

TEST_FILE=plain_text.txt

mkdir -p $TMP_DIR

echo "non zero exit codes are expected:"

result_1=0
cat $PASSPHRASE_FILE | $CMD -d --pwd-file $PASSPHRASE_FILE --key-pwd-fd 0 -o $TMP_DIR/out_file_1.txt $ENC_DIR/bad_mdc_hash.gpg || result_1=$?

result_2=0
cat $PASSPHRASE_FILE | $CMD -d --pwd-file $PASSPHRASE_FILE --key-pwd-fd 0 -o $TMP_DIR/out_file_2.txt $ENC_DIR/bad_mdc_header.gpg || result_2=$?

result_3=0
cat $PASSPHRASE_FILE | $CMD -d --pwd-file $PASSPHRASE_FILE --key-pwd-fd 0 -o $TMP_DIR/out_file_3.txt $ENC_DIR/corrupt_compression_packet.gpg || result_3=$?

RESULT=0
for i in 1 2 3
do
    if (( result_${i} != 0 ))
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
