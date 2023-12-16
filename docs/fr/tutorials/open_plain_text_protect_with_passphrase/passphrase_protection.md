# Ouvrir un fichier texte en clair, le protéger par phrase de passe et l’enregistrer comme fichier GPG

Nous allons ouvrir un fichier texte en clair, le protéger par phrase de passe et l’enregistrer comme fichier GPG.

## Ouvrir un fichier texte en clair

1\. Cliquez sur le bouton Ouvrir un fichier

![Bouton Ouvrir un fichier](images/open_text_file.png)

2\. Sélectionnez le type de fichier Texte en clair et cliquer sur Enregistrer

![Type de fichier Texte en clair](images/open_file_dialog_text_file.png)

3\. Le fichier texte en clair est ouvert

![Fichier texte ouvert](images/text_file_opened.png)

EncryptPad est maintenant en mode texte en clair non protégé. Elle fonctionne comme un éditeur de texte ordinaire.

![Mode non protégé](images/unprotected_status.png)

## Protéger avec une phrase de passe et enregistrer comme fichier GPG

4\. Cliquez sur le bouton Enregistrer sous

![Bouton Enregistrer sous](images/save_as_tool_button.png)

5\. Sélectionnez le type de fichier GnuPG et cliquez sur Enregistrer

![Boîte de dialogue Enregistrer sous](images/save_as_dialog.png)

Veuillez prendre note que le nom de fichier est maintenant « guerre et paix.gpg » dans la boîte de dialogue Enregistrer sous.

6\. Il vous sera demandé de saisir deux fois une phrase de passe. Saisissez `MdpDémo` et cliquez sur Valider

![Définir une phrase de passe](images/set_passphrase.png)

7\. Le fichier a maintenant été enregistré sur le disque, chiffré avec notre phrase de passe. Vous pouvez voir l’extension du fichier et son état dans l’image ci-dessous

![Fichier GPG enregistré](images/gpg_file_saved.png)

## Le fichier GPG sur le disque

C’est un fichier OpenPGP. Vous pouvez l’envoyer à quelqu’un qui n’a pas EncryptPad. Ce fichier peut par exemple être facilement déchiffré avec [Gpg4Win](https://www.gpg4win.org/).

![Kleopatra a déchiffré notre fichier](images/kleopatra_decrypted.png)

Sortie de la commande --list-packets du paquet GPG :

`gpg --list-packets "guerre et paix.gpg"`<br/>

`:symkey enc packet: version 4, cipher 9, s2k 3, hash 8`<br/>
`salt 9cd701f1d2e0aca8, count 65536 (96)`<br/>
`gpg: données chiffrées avec AES256`<br/>
`:encrypted data packet:`<br/>
` length: 1201175`<br/>
` mdc_method: 2`<br/>
`gpg: chiffré avec 1 phrase secrète`<br/>
`:compressed packet: algo=2`<br/>
`:literal data packet:`<br/>
`mode t (74), created 0, name="CONSOLE",`<br/>
`raw data: unknown length`<br/>

## Mettre le fichier à jour

Si vous modifiez maintenant le texte et cliquez sur le bouton Enregistrer, EncryptPad ne vous demandera pas de saisir la phrase de passe de nouveau, car elle stocke plusieurs hachages salés de votre phrase de passe.

## Ouvrir un autre fichier texte en clair

Si vous tentez d’ouvrir un fichier texte en clair maintenant, cela échouera avec le message suivant :

![Impossible d’ouvrir le texte en clair](images/open_another_plain_text.png)

EncryptPad pense que le fichier est chiffré avec une phrase de passe, car son mode est « Protégé par phrase de passe », comme vous pouvez le voir dans la barre d’état. Elle a tenté de déchiffrer le texte en clair et a échoué. Afin de poursuivre l’ouverture du fichier, effacez la protection par phrase de passe en cliquant sur le bouton Effacer la phrase de passe, ou mieux encore, en cliquant sur Fermer et réinitialiser ce qui fermera le fichier texte et réinitialisera la protection.

![Effacer la phrase de passe ou Fermer et réinitialiser](images/clear_pwd_or_close_and_reset.png)

Vous pouvez maintenant ouvrir votre fichier texte en clair.
