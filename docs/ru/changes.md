##Изменения Релизов##

###Версия 0.3.2.3

* Configurable s2k iteration count. It can be set per file and the default value for new files. That value is also used for protecting file keys.
* Support for the timestamp bytes in GPG files.
* Renamed the executable to low case `encryptpad` except on Mac OS.
* Improved English interface consistency in GUI. All `password`s became `passphrase`s.
* Translation to Russian.
* Translation to French.
* Added the `SHA-512` hashing algorithm.
* CLI: resilience in edge cases when TTY is not available for input / output.
* Bug fixed: In the file encryption dialogue, when clearing the passphrase and setting it again, the passphrase is ignored and the file is saved as "key only".
* Bug fixed: In the file encryption, gpg files were encrypted with 't' flag. It lead to removal of 0x0D bytes when GPG decrypted the files because it thought that the files were textual. The problem did not manifest on Windows and when EncryptPad was used for decryption. It was only in the direction from EncryptPad to Linux/Unix GPG.

###Версия 0.3.2.2

* Bug fixed: In file encryption dialogue, when the key file password was wrong, a message box did
    not inform the user.
* Support BAK files. It only creates a bak file at first save of an editing sessions.
* Changed the order of sections in new wad files. It is now Directory, key, payload. It does not
    affect existing wad files.
* encryptcli can read passphrases from file descriptors
* encryptcli can read from stdin and write to stdout
* encryptcli arguments cleaned up and improved
* Updated Botan version to 1.10.12
* 64 bit build
* Fixed build permissions
* Changed build to work with g++ on Linux
* Build can now use system libraries on Linux including Zlib and Botan
* Fixed build on Fedora 23
* Fixed warnings

###Версия 0.3.2.1

* Initial open source release

