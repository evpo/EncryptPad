# Ouvrir un fichier texte en clair, le protéger à la fois par phrase de passe et fichier clé et l'enregistrer comme fichier EPD]

## Ouvrir un fichier texte en clair

1\. Cliquez sur le bouton «&nbsp;Ouvrir un fichier&nbsp;».

![Bouton «&nbsp;Ouvrir un fichier&nbsp;»](images/open_text_file.png)

2\. Sélectionnez le type de fichier «&nbsp;Texte en clair&nbsp;» et cliquez sur Enregistrer.

![Type de fichier «&nbsp;Texte en clair»](images/open_file_dialog_text_file.png)

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

![Phrase de passe du fichier clé](images/set_passphrase_for_key.png)

Saisir `MdpClé`

7\. Cliquez sur Oui pour utiliser le nouveau fichier clé pour chiffrer votre texte.

![Boîte de dialogue «&nbsp;Utiliser une nouvelle clé&nbsp;»](images/use_new_key_dialog.png)

8\. Vous trouverez le fichier clé dans votre répertoire personnel sous `encryptpad`.

![Clé dans l'explorateur Windows](images/key_in_explorer.png)

Voici le contenu du fichier (il sera différent pour vous, car la clé est aléatoire) :<br/> 

`91Oamyib4o+Tbry/4NzoFOWhWpIfQE0TyMHiYtnOK9w=

C'est une séquence aléatoire en Base64 à partir de laquelle la clé de chiffrement est dérivée.

## Protéger avec le fichier clé et enregistrer comme fichier GPG

9\. Cliquez sur le bouton «&nbsp;Enregistrer sous&nbsp;.

![Bouton «&nbsp;Enregistrer sous&nbsp;»](images/save_as_tool_button.png)

10\. Sélectionnez le type de fichier EncryptPad (\*.epd) et cliquez sur le bouton Enregistrer.

![Boîte de dialogue «&nbsp;Enregistrer sous&nbsp;»](images/select_epd_in_save_as.png)

Veuillez prendre note que le nom de fichier a changé en `guerre et paix.epd` dans la boîte de dialogue «&nbsp;Enregistrer sous&nbsp;».

11\. Il vous sera demandé de saisir une phrase de passer deux fois. Saisissez `MdpDémo` pour cet exemple et cliquez sur OK.

![Définir une phrase de passe](images/set_passphrase.png)

Veuillez prendre note que si vous laissez la phrase de passe vide et cliquez sur OK, le fichier ne sera protégé que par le fichier clé et vous ne bénéficierez pas de la double protection. 

12\. Le fichier a maintenant été enregistré sur le disque, chiffré avec le fichier clé généré et la phrase de passe. Voir l'état dans l'image ci-dessous.

![Fichier GPG enregistré](images/double_protection_status.png)

## Emplacement de clé persistant dans le fichier chiffré

Maintenant, si vous effacez la protection, fermez le fichier et rouvrez-le. La phrase de passe et le fichier clé vous seront demandés. Il pourrait être fastidieux de saisir le chemin du fichier clé chaque fois, particulièrement s'il n'est pas dans le dépôt. Pour faciliter le tout, vous pouvez activer `Emplacement de clé persistant dans le fichier chiffré`. (cette fonction n'est prise en charge que pour le type de fichier EPD).

13\. Cliquez sur le bouton «&nbsp;Définir la clé&nbsp;.

![Bouton «&nbsp;Définir la clé&nbsp;»](images/set_key_tool_button.png)

14\. Activez `Emplacement de clé persistant dans le fichier chiffré` dans la boîte de dialogue «&nbsp;Définir la clé&nbsp;» et cliquez sur OK.

![Activer la clé persistente](images/enable_persist_key.png)

15\. La barre d'état a changé (voir l'image ci-dessous).

![A persisté dans la barre d'état](images/persisted_in_status_bar.png)

Si vous effacez la protection et rouvrez ce fichier, l'emplacement du fichier clé ne vous sera pas demandé, car il persiste à même le fichier chiffré. L'emplacement est aussi à l'abri des regards indiscrets, car il est chiffré avec la phrase de passe.

16\. Cliquez sur le bouton Enregistrer pour enregistrer le fichier.

## Ouvrir un autre fichier texte en clair

Si vous tentez d'ouvrir un fichier texte en clair maintenant, cela échouera avec le message suivant :

![Impossible d'ouvrir le texte en clair](images/open_another_plain_text.png)

EncryptPad pense que le fichier est chiffré avec une phrase de passe et un fichier clé, car son mode est `Protégé par une clé` et `Protégé par phrase de passe`, comme vous pouvez le voir dans la barre d'état. Elle a tenté de déchiffrer le texte en clair et a échoué. Pour poursuivre l'ouverture du fichier, effacez la protection par phrase de passe et fichier clé en cliquant sur les boutons «&nbsp;Effacer la clé de chiffrement&nbsp;» et «&nbsp;Effacer la phrase de passe&nbsp;», ou mieux encore, en cliquant sur «&nbsp;Fermer et réinitialiser&nbsp;» ce qui fermera le fichier texte et réinitialisera les deux parties de la protection.

![Effacer la clé ou Fermer et réinitialiser](images/clear_key_and_pwd_or_close_and_reset.png)

Vous pouvez maintenant ouvrir votre fichier texte en clair.

## Le fonctionnement interne du fichier EPD

Le fichier EPD est un fichier OpenPGP contenant un fichier [WAD](https://fr.wikipedia.org/wiki/.wad) qui contient un autre fichier OpenPGP. Il y a trois niveaux d'imbrication. Pour le démontrer, nous pouvons extraire le fichier chiffré sans utiliser EncryptPad.

17\. Renommez le fichier «&nbsp;guerre et paix.epd&nbsp;» en «&nbsp;guerre et paix.gpg&nbsp;».

18\. Déchiffrez le fichier avec [Gpg4Win](https://www.gpg4win.org/). Utilisez «&nbsp;MdpDémo&nbsp;» comme phrase de passe.

![Kleopatra a déchiffré notre fichier](images/kleopatra_decrypted.png)

19\. Renommez le fichier produit par Kleopatra de «&nbsp;guerre et paix&nbsp;» en «&nbsp;guerre_et_paix.wad&nbsp;».

20\. Pour la prochaine étape, il vous faut obtenir un outil pour travailler avec les fichiers WAD. Je recommande [Slade](https://github.com/sirjuddington/SLADE).

21\. Extrayez le contenu du fichier WAD avec Slade. Voir la fenêtre ci-dessous pour plus de détails.

![Fenêtre de Slade](images/slade_window.png)

Notez que le fichier «&nbsp;__X2_KEY&nbsp;» contient le nom de notre fichier clé «&nbsp;ma_nouvelle_clé.key&nbsp;». «&nbsp;_PAYLOAD&nbsp;» est le fichier OpenPGP chiffré avec le fichier clé. 

22\. Enregistrez «&nbsp;_PAYLOAD&nbsp;» sur le disque. Cliquez sur Entrée->Exporter et enregistrez le fichier sous **guerre et paix.gpg**. Écrasez les fichiers existants que nous avons utilisés dans les étapes précédentes.

![Exportation Slade](images/slade_export.png)

23\. Déchiffrez le fichier clé «&nbsp;ma_nouvelle_clé&nbsp;» avec [Gpg4Win](https://www.gpg4win.org/). Utilisez «&nbsp;MdpClé&nbsp;» comme phrase de passe.

24\. Maintenant que nous avons la clé non chiffrée, déchiffrez le fichier de l'étape 22 avec [Gpg4Win](https://www.gpg4win.org/). Utilisez le contenu du fichier clé de l'étape 23 comme phrase de passe.

![Kleopatra a déchiffré notre fichier](images/kleopatra_decrypted.png)

25\. Le résultat de ce déchiffrement sera votre fichier texte en clair original.