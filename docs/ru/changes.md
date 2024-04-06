## Изменения ##

### Версия 0.5.0.4

* Обновить Botan до версии 2.19.4
* Использовать SVG иконы из проекта Breeze для исправления проблемы отображения икон на 2K/4K мониторах
* Добавить возможность распечатать файл
* Показать путь в свойствах файла
* Добавить меню для закрытия файла
* Добавить возможность вычислить математическое выражение в текущей строке
* Показать количество S2K хешей, с помощью которых можно записать документ, не запрашивая пароль.
Создать новый файл с помощью параметра командной строки

### Версия 0.5.0.3

* Исправлен фокус на текстовое поле в диалоге запроса пароля или ключа
* Добавлен Испанский перевод
Исправление: В FakeVim Alt+Shift обычно используется для переключения языков вставляет 0x00 символ
* Исправление: переключение перенос предложения не обновляет номеров строк
* Бибилиотека Botan обновлена до версии 2.19.3
* Добавлено больше логирования при работе с файломи. Сообщения об ошибках будут доступны в логах во время файловых операций.
* Добавлены параметры системы конфигурации при компиляции для пути установки. /usr/local теперь корень установки по умолчанию.
* Отключить зависимость от архитектуры SSE4.1 и SSE4.2 для поддержки более старых компьютеров.

### Version 0.5.0.2

* An option has been added to display line numbers in the editor
* Bug fix: invalid characters entered by the user are not checked and a bad key file is created on Windows
* Bug fix: in the file encryption dialogue clearing the passphrase sometimes causes invalid behaviour
* FakeVim mode: the cursor has become more consistent and presentable
* FakeVim mode: the native file open dialogue can now be used
* FakeVim mode: support relative line numbers (rnu)
* Bug fix: in FakeVim mode the CPU consumption is 100% due to an infinite loop in the Paint event
* Botan has been updated to version 2.19.2
* Localization has been updated

### Version 0.5.0.1

* FakeVim mode to enable editing files with Vim-like interface
* The lock icon has been replaced with a page and lock with the higher quality of the image.
* Botan has been updated to version 2.18.1
* The build system has been improved to support more distributions
* Bug fix: Opening search dialog by using 'CTRL-F' doesn't focus on the find text field

### Version 0.5.0.0

* ASCII armor
* Added cipher algorithms: CAMELLIA128, CAMELLIA192, CAMELLIA256, TWOFISH
* Added S2K hash digest algorithms: SHA384, SHA512, SHA224
* Added compression algorithm: BZIP2
* Botan has been updated to version 2.13
* Translation fixes for context menu
* New Python build system ported from Botan
* Bug fix: MDC failure was not reported when opening files
* Bug fix: the special case in which the last partial packet was smaller than 5 bytes was not handled correctly
* Bug fix: --key-file-length parameter was broken in CLI

### Version 0.4.0.4

* Portable software should be able to store all its files in a single removable location such as a memory stick. EncryptPad creates its setting file in `$HOME/.encryptpad`. After this change EncryptPad will check if there is `encryptpad_repository` directory in the application path and use it for settings and key files.
* Botan has been updated to version 2.7.
* Plog has been updated to version 1.1.4.
* Translation to Lithuanian

### Version 0.4.0.0

* Support for large files. EncryptPad can now process disk images, backups and other types of multi-gigabyte files.
* A progress bar and cancellation button in the file encryption dialogue.
* Tab size adjustment in the text editor.
* Botan has been updated to version 2.3.
* STLplus was updated to version 3.15.
* OpenPGP functionality has moved to its own library libencryptmsg
* WAD format that is used for files encrypted with the persistent key file location and double protection has changed. Files of this type that are larger than 64 kilobytes are not backward compatible and cannot be opened with 0.3.2.5. This does not apply to gpg and epd files with single encryption and without persistent key file.

### Version 0.3.2.5

* Bug fixed: if a decrypted passphrase-only EPD file contained less than 4 characters, the content was ignored and EncryptPad produced an empty file. It happened because EncryptPad expected to find IWAD marker, which was 4 character long. Such files will now open correctly.
* Bug fixed: when opening a plain-text file and saving it as encrypted, the encryption parameters did not reset to the default values but used the parameters of the last encrypted file.
* Bug fixed: the `encryptpad` `file` command line parameter did not support non ASCII characters. As result it was not possible to open a file directly from Windows Explorer by double clicking on it if the file was in a directory which name contained non ASCII characters.
* More properties in the preferences to control default encryption parameters: key file random sequence length, key file encryption properties, default file encryption properties (cipher, s2k, iterations, compression), the number of encryption keys to save or load without prompting the passphrase again.
* Bug fixed: when multiple EncryptPad instances were opened and preferences updated, the last instance overwrote the preferences changed in other instances on closing.
* In File Encryption dialogue, a radio button was added to select between EPD and GPG. Previously the user had to edit the extension manually to output to the GPG format.
* Bug fixed: salt was displayed in File Encryption dialogue after encrypting or decrypting a file, which did not make sense as it was not related to the next encryption.
* There are only 256 possible S2K iterations sets. The file properties dialogue now has a combo box to list all of them. It is easier to select the maximum or minimum possible number of iterations.
* The default number of iterations has been changed to 1015808.
* Prevented the unnecessary prompt for a key file passphrase if the key file is local and not encrypted.
* Bug fixed: after opening a file the mouse cursor changed to arrow shape when it was over the text edit area. It is now `I` shaped as it should.
* Use one EncryptPad binary for all languages. EncryptPad will recognize the OS preferred language for localization. It will simplify the release management especially for packagers.
* `--lang` switch to force a specific language instead of taking the OS's locale. Example: `encryptpad --lang FR` to load the French localization strings. Example: `encryptpad --lang FR` to load the French localization strings.
* EncryptPad GUI was translated to Chinese.
* README.md translated to French.
* stlplus was updated from upstream. It is now 3.13.


### Version 0.3.2.3

* Configurable s2k iteration count. It can be set per file and the default value for new files. That value is also used for protecting file keys.
* Support for the timestamp bytes in GPG files.
* Renamed the executable to low case `encryptpad` except on macOS.
* Improved English interface consistency in GUI. All `password`s became `passphrase`s.
* Translation to Russian.
* Translation to French.
* Added the `SHA-512` hashing algorithm.
* CLI: encryptcli crashed with a segfault when TTY was not available. Now it exits gracefully with an error code.
* Bug fixed: In the file encryption dialogue, when clearing the passphrase and setting it again, the passphrase was ignored and the file was saved as "key only".
* Bug fixed: In the file encryption, gpg files were encrypted with 't' flag. It lead to removal of 0x0D bytes when GPG decrypted the files because it thought that the files were textual. The problem did not manifest on Windows and when EncryptPad was used for decryption. It was only in the direction from EncryptPad to Linux/Unix GPG.

### Version 0.3.2.2

* Bug fixed: In file encryption dialogue, when the key file passphrase was wrong, a message box did not inform the user.
* BAK files support. When editing, a bak file is only created when saving for the first time.
* Changed the order of sections in new wad files. It is now the directory, key, payload. It does not affect existing wad files.
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

### Version 0.3.2.1

* Initial open source release

