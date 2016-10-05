#Contributions

##Overview

This directory is for utilities, scripts, documents and examples related to EncryptPad. You will
find descriptions and feedback contacts in the list below and source files in each subdirectory.

##List

## `gpgvim`

Commandline utility that allows using `vim` (or your preferred editor, if you set the `EDITOR` environment variable) in a mode compatible with EncryptPad. It uses `gpg2` for encryption and `vipe` (from `moreutils`). Useful to open and edit files without a GUI.

Also useful to validate EncryptPad's encryption mechanism.

__Note:__ `gpgvim` makes every effort to ensure safe handling of temp files. However, tempfiles (in case the editor shuts down unexpectedly) or free space (unclaimed bytes on disk after the temp file was deleted) revealing your secrets might be left behind. In addition, some editors create tempfiles or backup files outside of `TMPDIR`. The `vim` default has options set to prevent this, if you override `EDITOR` you should check your editor's behaviour.

## `markdown2web`

Converts a directory with markdown files to a directory with HTML that can be used as a static web directory.
