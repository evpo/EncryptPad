<style type="text/css">
h2
{
    text-align: center;
}
</style>
##Notes de version##

###Version 0.3.2.3

* Nombre configurable d'itérations s2k. Il peut être été fini par fichier, et comme valeur par défaut pour les nouveaux fichiers. Cette valeur est aussi utilisée pour la protection des fichiers.
* Prise en charge des octets d'estampille temporelle dans les fichiers GPG.
* l'exécutable a été renommer en « encryptpad » (minuscules) sauf sur macOS.
* Amélioration de la cohérence de l'anglais dans l'IUG. Tous les « mot de passe » sont devenus des « phrase de passe ».
* Traduction en russe.
* Traduction en français.
* Ajout de l'algorithme de hachage SHA-512.
* CLI: resilience in edge cases when TTY is not available for input / output.
* Correctif de bogue : dans la boîte de dialogue du chiffrement de fichiers, quand la phrase de passe était effacée puis redéfinie, la phrase de passe était ignorée et le fichier était enregistré comme « clé seulement ».
* Correctif de bogue : dans le chiffrement des fichiers, les fichiers gpg étaient chiffrés avec le drapeau « t », ce qui entraînait la suppression de 0x0D octets quand GPG déchiffrait les fichiers, car il croyait que les fichiers étaient textuels. Le problème ne se produisait pas sous Windows ni quand EncryptPad était utilisé pour le déchiffrement. Cela ne se produisait que dans la direction EncryptPad vers GPG pour Linux/Unix.

###Version 0.3.2.2

* Correctif de bogue : dans la boîte de dialogue du chiffrement, lorsque la phrase de passe du fichier clé était erronée, un message
    n'en informait pas l'utilisateur.
* Prise en charge des fichiers BAK. Lors d'une modification, un fichier bak n'est crée que lors du premier enregistrement.
* L'ordre des sections a été changé dans les nouveaux fichiers wad. Il est maintenant répertoire, clé, contenu. Cela n'affecte pas
    les fichiers wad existants.
* encryptcli peut lire les phrases de passe des descripteurs de fichier
* encryptcli peut lire de stdin et écrire vers stdout
* Les arguments d'encryptcli ont été revus et améliorés
* Botan a été mis à jour vers la version 1.10.12 
* Version 64 bits
* Correctif des droits de compilation
* La compilation peut maintenant fonctionner avec g++ sous Linux
* La compilation peut maintenant utiliser les bibliothèques système sous Linux incluant Zlib et Botan
* Correctif de la compilation sous Fedora 23
* Correctif des avertissements

###Version 0.3.2.1

* Première version à code source ouvert
