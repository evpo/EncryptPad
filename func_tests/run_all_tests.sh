#!/bin/sh
set -e

USAGE="run_all_tests.sh <path_to_encrypt_cli>"
if [ $# -lt 1 ]
then
    echo $USAGE
    exit 1
fi

BIN="$1"
TMP_DIR="./tmp"
OS=$(uname)

PLAIN_TEXT_FILE=$TMP_DIR/random.dat

export PLAIN_TEXT_FILE

rm -Rf $TMP_DIR
rm -Rf ./epd_encrypted_last
rm -Rf ./gpg_encrypted_last

mkdir -p $TMP_DIR
dd if=/dev/urandom of=$PLAIN_TEXT_FILE bs=1M count=15

# diffrent key file and passphrase combinations
./epd_encryption_test.sh $BIN "$PLAIN_TEXT_FILE"

#encryption with an empty file
./epd_encryption_test.sh $BIN empty.txt

# encrypt files with gpg
./encrypt_with_gpg.sh

# decrypt gpg files with epd
./decryption_test.sh $BIN ./gpg_encrypted_last
rm -R ./gpg_encrypted_last

# encrypt with epd
./encrypt_with_epd.sh $BIN "$PLAIN_TEXT_FILE"

# decrypt the above with gpg
./decryption_test.sh gpg ./epd_encrypted_last
rm -R ./epd_encrypted_last

# input output redirection and pipes
./io_redir_test.sh $BIN
./decryption_test.sh "$1" ./epd_encrypted_last
rm -Rf ./epd_encrypted_last
rm -Rf ./gpg_encrypted_last
rm -Rf ./tmp
