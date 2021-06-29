#!/usr/bin/env bash

set -e
set -o pipefail

USAGE="encrypt_with_epd.sh <path_to_encrypt_cli> <encrypted_file>"
if [ $# -lt 2 ]
then
    echo $USAGE
    exit 1
fi

CMD="$1 -e --pwd-fd 0"
IN=$2
OUT_DIR="./epd_encrypted_last"
ALGOS=algos.txt
PASSPHRASE_FILE="passphrase.txt"
COMPRESSIONS=`awk '$1 == "compress" { print $2 }' algos.txt`
CIPHERS=`awk '$1 == "cipher" { print $2 }' algos.txt`
S2K_ALGOS=`awk '$1 == "s2k_algo" { print $2 }' algos.txt`

mkdir -p $OUT_DIR
for EXT in epd asc
do
    for COMPRESS in $COMPRESSIONS
    do
        for CIPHER in $CIPHERS
        do
            for S2K_ALGO in $S2K_ALGOS
            do
                ARMOR_CLAUSE=""
                if [[ "$EXT" == "asc" ]]; then
                    ARMOR_CLAUSE="--armor"
                fi
                echo "encrypting ${CIPHER} ${S2K_ALGO} ${COMPRESS} ${EXT}"
                cat $PASSPHRASE_FILE | $CMD -o ${OUT_DIR}/${CIPHER}_${S2K_ALGO}_${COMPRESS}.${EXT} --compress-algo $COMPRESS --s2k-digest-algo $S2K_ALGO --cipher-algo $CIPHER $ARMOR_CLAUSE $IN
            done
        done
    done
done
