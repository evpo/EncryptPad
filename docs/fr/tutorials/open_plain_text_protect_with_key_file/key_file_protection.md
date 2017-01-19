# Ouvrir un fichier texte en clair, le protéger par fichier clé et l'enregistrer comme fichier GPG

## Ouvrir un fichier texte en clair

1\. Cliquez sur le bouton «&nbsp;Ouvrir un fichier&nbsp;».

![Bouton «&nbsp;Ouvrir un fichier&nbsp;»](images/open_text_file.png)

2\. Sélectionnez le type de fichier texte en clair et cliquez sur le bouton Enregistrer.

![Type de fichier «&nbsp;Texte en clair&nbsp;»](images/open_file_dialog_text_file.png)

3\. Le fichier texte en clair ouvert.

![Fichier texte ouvert](images/text_file_opened.png)

EncryptPad est maintenant en mode texte en clair non protégé. Elle fonctionne comme un éditeur de texte ordinaire.

![Mode non protégé](images/unprotected_status.png)

## Générer un nouveau fichier

4\. Cliquez sur le bouton «&nbsp;Générer une clé&nbsp;».

![Bouton «&nbsp;Générer une clé&nbsp;»](images/generate_key_tool_button.png)

5\. Sélectionnez le bouton radio «&nbsp;Clé dans le dépôt&nbsp;», saisissez `ma_nouvelle_clé` dans la boîte de texte correspondante et cliquez sur OK.

![Boîte de dialogue «&nbsp;Générer une clé&nbsp;»](images/generate_key_dialog.png)

6\. Il vous sera demandé de saisir une phrase de passe pour protéger le fichier clé sur le disque.

![Phrase de passe du fichier](images/set_passphrase_for_key.png)

Saisir `MdpClé`

7\. Cliquez sur Oui pour utiliser le nouveau fichier clé pour chiffrer votre texte.

![Boîte de dialogue «&nbsp;Utiliser une nouvelle clé&nbsp;»](images/use_new_key_dialog.png)

8\. Vous trouverez le fichier clé dans votre répertoire personnel sous `encryptpad`.

![Clé dans l'explorateur Windows](images/key_in_explorer.png)

Voici le contenu du fichier (il sera différent pour vous, car la clé est aléatoire) :<br/> 

    -----BEGIN PGP MESSAGE-----
    
    ww0ECQMIwHjM/7M8CP1g0lwBTWLhkWz6OgNJ++rjgwES+WIWttapxGFG76nANeuC
    FtCIN9xGbsFFm1g9iJRxMgWlUz9s97+tC7PVhxgCDTsfbfbFA54UAdRClR0hqoPA
    S27p6/6u5ztvfGWGrw==
    =bXn9
    -----END PGP MESSAGE-----

C'est une séquence aléatoire en Base64 à partir de laquelle la clé de chiffrement est dérivée. Elle est chiffrée avec la phrase de passe du fichier clé que vous avez fournie et peut aussi être déchiffrée avec GnuGPG. Si [Gpg4Win](https://www.gpg4win.org/) est installé, vous pouvez cliquer à droite sur le fichier dans l'explorateur Windows et choisir «&nbsp;Plus d'options GpgEX&nbsp;» et «&nbsp;Déchiffrer&nbsp;». Saisissez `MdpClé` comme phrase de passe et gpg déchiffrera le fichier et créera ma_nouvelle_clé_.key.out. Le contenu du fichier sera semblable à ce qui suit :

    T6/vlnb0aysdr+xUriwwwrC844iVImiG4T/sCgF+ETQ=

## Protéger avec le fichier clé et enregistrer comme fichier GPG

9\. Cliquez sur le bouton «&nbsp;Enregistrer sous&nbsp;.

![Bouton «&nbsp;Enregistrer sous&nbsp;»](images/save_as_tool_button.png)

10\. Sélectionnez le type de fichier GnuPG et cliquez sur le bouton Enregistrer.

![Boîte de dialogue « Enregistrer sous »](images/save_as_dialog.png)

Veuillez prendre note que le nom de fichier a changé en `guerre et paix.gpg` dans la boîte de dialogue «&nbsp;Enregistrer sous&nbsp;».

11\. Le fichier a maintenant été enregistré sur le disque, chiffré avec le fichier clé généré. Voir l'état dans l'image ci-dessous.

![Fichier GPG enregistré](images/key_protected_status.png)

## Le fichier GPG sur le disque

C'est un fichier OpenPGP. Vous pouvez l'envoyer à quelqu'un qui n'a pas EncryptPad. On peut le déchiffrer facilement avec [Gpg4Win](https://www.gpg4win.org/). Utilisez le contenu de ce fichier clé comme phrase de passe `T6/vlnb0aysdr+xUriwwwrC844iVImiG4T/sCgF+ETQ=` (voir étape 8).   

![Kleopatra a déchiffré notre fichier](images/kleopatra_decrypted.png)

Sortie de la commande --list-packets du paquet GPG :

    gpg --list-packets "guerre et paix.gpg"
    
    :symkey enc packet: version 4, cipher 9, s2k 3, hash 8
            salt 1c73cc8df90a54d5, count 65536 (96)
    gpg: données chiffrées avec AES256
    :encrypted data packet:
            length: 1201175
            mdc_method: 2
    gpg: chiffré avec 1 phrase secrète
    :compressed packet: algo=2
    :literal data packet:
            mode t (74), created 0, name="CONSOLE",
            raw data: unknown length

## Visualiser ou changer le fichier clé utilisé comme protection

12\. Cliquez sur le bouton «&nbsp;Définir la clé&nbsp;.

![Bouton «&nbsp;Définir la clé&nbsp;»](images/set_key_tool_button.png)

13\. Visualisez le fichier clé sélectionné dans la boîte de dialogue «&nbsp;Définir la clé&nbsp;».

![Boîte de dialogue «&nbsp;Définir une clé&nbsp;»](images/set_key_dialog.png)

Les fichiers sans chemin sont d'abord cherchés dans le répertoire d'EncryptPad, puis dans le dépôt du dossier personnel (voir le texte de l'image ci-dessus). Si votre clé ne se trouve pas dans le dépôt, vous pouvez indiquer le chemin complet vers votre clé. 

L'«&nbsp;emplacement de clé persistant dans le fichier chiffré&nbsp;» n'est pas pris en charge pour les fichiers GPG (voir README.md). Laissez vide pour l'instant. À FAIRE : ajouter le lien vers le guide sur le double chiffrement  EPD.

## Ouvrir un autre fichier texte en clair

Si vous tentez d'ouvrir un fichier texte en clair maintenant, cela échouera avec le message suivant :

![Impossible d'ouvrir le texte en clair](images/open_another_plain_text.png)

EncryptPad pense que le fichier est chiffré avec un fichier clé, car son mode est «&nbsp;Protégé par une clé&nbsp;», comme vous pouvez le voir dans la barre d'état. Elle a tenté de déchiffrer le texte en clair et a échoué. Pour poursuivre l'ouverture du fichier, effacez la protection par fichier clé en cliquant sur le bouton «&nbsp;Effacer la clé de chiffrement&nbsp;», ou mieux encore, en cliquant sur «&nbsp;Fermer et réinitialiser&nbsp;» ce qui fermera le fichier texte et réinitialisera la protection.

![Effacer la clé ou Fermer et réinitialiser](images/clear_key_or_close_and_reset.png)

Vous pouvez maintenant ouvrir votre fichier texte en clair.