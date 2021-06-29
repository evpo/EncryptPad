#!/usr/bin/env bash

set -e
set -o pipefail

USAGE="random_content.sh <path_to_encryptcli>"
if [ $# -lt 1 ]
then
    echo $USAGE
    exit 1
fi

TMP_DIR="./tmp"

mkdir -p $TMP_DIR

PLAIN_TEXT_FILE=$TMP_DIR/random.dat
export PLAIN_TEXT_FILE

rm -f $PLAIN_TEXT_FILE
rm -fR ./epd_encrypted_last

dd if=/dev/urandom of=$PLAIN_TEXT_FILE bs=1M count=300

sh -x ./encrypt_with_epd.sh $1 $PLAIN_TEXT_FILE
sh -x ./decryption_test.sh $1 ./epd_encrypted_last
