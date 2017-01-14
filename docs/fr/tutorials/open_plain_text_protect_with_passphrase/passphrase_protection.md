# Ouvrir un fichier texte en clair, le protéger par phrase de passe et l'enregistrer comme fichier GPG

We are going to open a plain text file, protect it with a passphrase and save as a GPG file.

## Open a plain text file

1\. Click the Open File tool button

![Bouton « Ouvrir un fichier »](images/open_text_file.png)

2\. Select Plain Text file type and click Save button

![Type de fichier « Texte en clair »](images/open_file_dialog_text_file.png)

3\. The plain text file opened

![Fichier texte ouvert](images/text_file_opened.png

Now EncryptPad is in unprotected plain text mode. It works as an ordinary text editor.

![Mode non protégé](images/unprotected_status.png)

## Protect with a passphrase and save as a GPG file

4\. Click the Save As tool button.

![Bouton « Enregistrer sous »](images/save_as_tool_button.png)

5\. Select GnuPG file type and click Save button

![Boîte de dialogue « Enregistrer sous »](images/save_as_dialog.png)

Please note that the file name has changed to "war and peace.gpg" in the Save As dialogue.

6\. Il vous sera demandé de saisir deux fois une phrase de passe. Saisir « MdpDémo » et cliquez sur OK.

![Définir une phrase de passe](images/set_passphrase.png)

7\. Now the file has been saved on the disk and it is encrypted with our passphrase. See the file extension and the status on the picture below.

![Fichier GPG enregistré](images/gpg_file_saved.png)

## The GPG file on disk

It is an Open PGP file. You can send it to someone who does not have EncryptPad. It is easily decrypted with [Gpg4Win](https://www.gpg4win.org/) for example.

![Kleopatra a déchiffré notre fichier](images/kleopatra_decrypted.png)

GPG packet --list-packets command output:

`gpg --list-packets "war and peace.gpg"`<br/>

`:symkey enc packet: version 4, cipher 9, s2k 3, hash 8`<br/>
`        salt 9cd701f1d2e0aca8, count 65536 (96)`<br/>
`gpg: AES256 encrypted data`<br/>
`:encrypted data packet:`<br/>
`        length: 1201175`<br/>
`        mdc_method: 2`<br/>
`gpg: encrypted with 1 passphrase`<br/>
`:compressed packet: algo=2`<br/>
`:literal data packet:`<br/>
`        mode t (74), created 0, name="CONSOLE",`<br/>
`        raw data: unknown length`<br/>

## Update the file

If you modify the text now and click Save tool button, EncryptPad will not ask you to enter the passphrase again because it stores several salted hashes of your passphrase.

## Open another plain text file

If you try to open a plain text file now, it will fail with the following message.

![Impossible d'ouvrir le texte en clair](images/open_another_plain_text.png)

EncryptPad "thinks" that the file is encrypted with a passphrase because its mode is "passphrase protected" as you can see in the status bar. It tried to decrypt the plain text and failed. To proceed with opening the file, clear passphrase protection by clicking the Clear passphrase tool button or even better click Close and Reset. The latter will close the text file and reset both parts of the protection.

![Effacer la phrase de passe ou Fermer et réinitialiser](images/clear_pwd_or_close_and_reset.png)

Now you can open your plain text file.