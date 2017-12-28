#!/bin/bash
set -e
set -x
set -o pipefail
GPG="gpg2"
CMD="$GPG -c --verbose --batch --no-use-agent --passphrase-file passphrase.txt"

if [ "$PLAIN_TEXT_FILE" = "" ]
then
    PLAIN_TEXT_FILE=plain_text.txt
fi
OUT_DIR="./gpg_encrypted_last"
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
            if [ "$COMPRESS" = "none" ]
            then
                COMPRESS_CLAUSE="--compress-level 0"
            else
                COMPRESS_CLAUSE="--compress-algo $COMPRESS"
            fi

            $CMD -o ${OUT_DIR}/${CIPHER}_${S2K_ALGO}_${COMPRESS}.gpg $COMPRESS_CLAUSE --s2k-digest-algo $S2K_ALGO --cipher-algo $CIPHER $PLAIN_TEXT_FILE
        done
    done
done
