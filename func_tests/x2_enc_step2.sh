#!/bin/bash
GPG="gpg2"
$GPG -c --verbose --passphrase-file passphrase.txt -o gpg_encrypted/CAST5_SHA1_none_key_file.gpg x2_encrypted/payload.wad
