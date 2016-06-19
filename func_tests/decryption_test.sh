#!/bin/bash
set -e
set -o pipefail
USAGE1="decryption_test.sh <path_to_decryptor> <directory_with_encrypted_files>"
USAGE2="path_to_decryptor - path to encryptcli or gpg"
if [ $# -lt 2 ]
then
    echo $USAGE1
    echo $USAGE2
    exit 1
fi

DECRYPTOR="$1"
IN_DIR="$2"
TMP_DIR="./tmp"
PASSPHRASE_FILE="passphrase.txt"
if [ "$PLAIN_TEXT_FILE" = "" ]
then
    PLAIN_TEXT_FILE=plain_text.txt
fi

mkdir -p $TMP_DIR

RESULT=0

for FILE in ${IN_DIR}/*
do
    if [ $DECRYPTOR = "gpg" ]
    then
        gpg -d --no-use-agent --passphrase-file $PASSPHRASE_FILE -o $TMP_DIR/out_file.txt $FILE
    else
        cat $PASSPHRASE_FILE | $DECRYPTOR -d --pwd-fd 0 -o $TMP_DIR/out_file.txt $FILE
    fi

    if ! [ -r $TMP_DIR/out_file.txt ]
    then
        continue
    fi

    if diff $PLAIN_TEXT_FILE $TMP_DIR/out_file.txt
    then
        echo ok
    else
        RESULT=1
        echo fail
    fi
    rm $TMP_DIR/out_file.txt
done

exit $RESULT
