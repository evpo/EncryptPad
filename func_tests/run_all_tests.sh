#!/bin/sh
set -e

USAGE="run_all_tests.sh <path_to_encrypt_cli>"
if [ $# -lt 1 ]
then
    echo $USAGE
    exit 1
fi

BIN="$1"

# diffrent key file and passphrase combinations
./epd_encryption_test.sh $BIN plain_text.txt

#encryption with an empty file
./epd_encryption_test.sh $BIN empty.txt

# encrypt files with gpg
./encrypt_with_gpg.sh

# decrypt gpg files with epd
./decryption_test.sh $BIN ./gpg_encrypted_last
rm -R ./gpg_encrypted_last

# encrypt with epd
./encrypt_with_epd.sh $BIN plain_text.txt

# decrypt the above with gpg
./decryption_test.sh gpg ./epd_encrypted_last
rm -R ./epd_encrypted_last
