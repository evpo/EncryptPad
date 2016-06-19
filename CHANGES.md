##Release Notes##

###Version 0.3.2.2

* Bug fixed. In file encryption dialogue, when the key file password was wrong, a message box did
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

###Version 0.3.2.1

* Initial open source release

