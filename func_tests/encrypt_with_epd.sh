#!/bin/sh

set -e

USAGE="encrypt_with_epd.sh <path_to_encrypt_cli> <encrypted_file>"
if [ $# -lt 2 ]
then
    echo $USAGE
    exit 1
fi

CMD="$1 -e -p passphrase.txt"
IN=$2
OUT_DIR="./epd_encrypted_last"
ALGOS=algos.txt
COMPRESSIONS=`awk '$1 == "compress" { print $2 }' algos.txt`
CIPHERS=`awk '$1 == "cipher" { print $2 }' algos.txt`
S2K_ALGOS=`awk '$1 == "s2k_algo" { print $2 }' algos.txt`

mkdir -p $OUT_DIR
for COMPRESS in $COMPRESSIONS
do
    for CIPHER in $CIPHERS
    do
        for S2K_ALGO in $S2K_ALGOS
        do
            $CMD -o ${OUT_DIR}/${CIPHER}_${S2K_ALGO}_${COMPRESS}.epd -m $COMPRESS -g $S2K_ALGO -c $CIPHER $IN
        done
    done
done
