# Linux-PAM - Pluggable Authentication Modules for Linux

## Overview

PAM est, sous forme d'API, le système de gestion d'identité de Linux. C'est un système composé de 3 parties :

* L'administrateur système qui va créer des __services__ (fichiers de configuration dans `/etc/pam.d/` qui définissent comment authentifier un utilisateur, etc.).

* Des __programmes__ dits _PAM aware_ qui vont faire appel à ces services.

* Des __modules__ PAM (des bibliothèques partagées, des `.so`) qui vont fournir ces services.

L'intérêt est donc de délester les programmes _PAM aware_ de toute la couche de sécurité liée à la gestion d'utilisateurs. Ces programmes peuvent simplement demander à PAM d'authentifier un utilisateur via un service et attendre de recevoir une réponse booléenne.

Par exemple, le __programme__ `/bin/login` va demander au système d'authentifier l'utilisateur (qui tente de se connecter) via le __service__ `/etc/pam.d/login`. Ce fichier de configuration contiendra une liste de règles (qui nous indiqueront quels __modules__ il faut appeler, dans quel ordre, etc.) pour accepter ou refuser l'utilisateur qui tente de se connecter.

Ce document a pour objectif de :

1. Présenter ce système de façon globale,

2. Montrer comment créer des application basées sur PAM,

3. Montrer comment créer un module PAM.

Ce document sera accompagné d'un Live Coding (vidéo) et le code est disponible sur github.

![Comme toujours, je vous invite fortement à lire les man de chaque fonction avant de les utiliser](http://messages.hellobits.com/warning.svg?message=Comme%20toujours%2C%20je%20vous%20invite%20fortement%20%C3%A0%20lire%20les%20man%20de%20chaque%20fonction%20avant%20de%20les%20utiliser)

Sommaire
---

- [Politiques d'authentification](#politiques-dauthentification)
- [Contenu d'un fichier service](#contenu-dun-fichier-service)
	- [1. Type de tâche à invoquer (auth, account, password, session)](#1-type-de-tâche-à-invoquer-auth-account-password-session)
	- [2. Rôle de la règle dans la chaîne de validation](#2-rôle-de-la-règle-dans-la-chaîne-de-validation)
		- [requisite](#requisite)
		- [required](#required)
		- [sufficient](#sufficient)
		- [optional](#optional)
	- [3. Nom du module](#3-nom-du-module)
	- [4. Éventuels arguments à passer au module](#4-Éventuels-arguments-à-passer-au-module)
- [Rendre une application PAM aware](#rendre-une-application-pam-aware)
	- [1. Dépendances](#1-dépendances)
	- [2. Linker options](#2-linker-options)
	- [3. Headers](#3-headers)
	- [4. Fonctions](#4-fonctions)
		- [pam_start()](#pam_start)
		- [pam_end()](#pam_end)
		- [pam_authenticate()](#pam_authenticate)
		- [pam_acct_mgmt()](#pam_acct_mgmt)
		- [pam_get_item()](#pam_get_item)
		- [misc_conv](#misc_conv)
	- [5. Exemple](#5-exemple)
- [Coder un module PAM](#coder-un-module-pam)
	- [Service auth](#service-auth)
		- [pam_sm_authenticate()](#pam_sm_authenticate)
		- [pam_sm_setcred()](#pam_sm_setcred)
	- [Service account](#service-account)
	- [Service password](#service-password)
	- [Service session](#service-session)
- [Exercice](#exercice)
- [Ressources](#ressources)
- [Auteur](#auteur)
- [Licence](#licence-)

## Politiques d'authentification

C'est l'administrateur système qui crée les politiques d'authentification, càd la succession d'étapes à valider (règles) pour considérer qu'un utilisateur est correctement authentifié ou non.

Pour reprendre l'exemple du programme `/bin/login`, ce n'est pas lui qui va lire sur l'entrée standard login et password, ouvrir les fichiers `/etc/passwd` et `/etc/shadow`, et comparer les informations lues avec le contenu de ces fichiers. Vu que c'est un programme PAM aware, il va faire appel à cette API en lui demandant d'appliquer la politique définie pour le service login, càd celle contenue dans le fichier `/etc/pam.d/login`, et il va laisser PAM s'occuper de ce qu'on appelle la transaction. PAM demandera les identifiants à l'utilisateur, les soumettra aux modules concernés en respectant les règles du service login, et donnera une réponse au programme `/bin/login` à l'issue de cette transaction pour lui dire si oui ou non il peut ouvrir un accès à cet utilisateur.

Ces services, ou politiques d'authentification, sont des fichiers de configuration textes définis dans `/etc/pam.d/`.

Par exemple, j'ai actuellement sur ma machine les services suivants :

```console
$ ls /etc/pam.d
atd
chfn
chpasswd
chsh
common-account
common-auth
common-password
common-session
common-session-noninteractive
cron
cups
gdm-autologin
gdm-launch-environment
gdm-password
login
newusers
other
passwd
polkit-1
ppp
runuser
runuser-l
su
sudo
systemd-user
```

On peut voir qu'il existe le service login. Voici son contenu (débarrassé des commentaires) :

```console
$ cat /etc/pam.d/login
auth       optional   pam_faildelay.so  delay=3000000
auth [success=ok new_authtok_reqd=ok ignore=ignore user_unknown=bad default=die] pam_securetty.so
auth       requisite  pam_nologin.so
session [success=ok ignore=ignore module_unknown=ignore default=bad] pam_selinux.so close
session       required   pam_env.so readenv=1
session       required   pam_env.so readenv=1 envfile=/etc/default/locale
@include common-auth
auth       optional   pam_group.so
session    required   pam_limits.so
session    optional   pam_lastlog.so
session    optional   pam_exec.so type=open_session stdout /bin/uname -snrvm
session    optional   pam_motd.so
session    optional   pam_mail.so standard
session    required     pam_loginuid.so
@include common-account
@include common-session
@include common-password
session [success=ok ignore=ignore module_unknown=ignore default=bad] pam_selinux.so open
```

C'est la succession de règles qu'a prévu l'administrateur système pour les applications invoquant le service login.

Certaines lignes commencent par `@include`. Comme on peut s'en douter, il s'agit d'insérer à cette endroit le contenu d'un autre fichier (à la façon d'un préprocesseur C avec les directives `#include`). Si je reprends ce fichier et fait le pré-processing à l'aide du script `compile_pam_file.sh` (outils disponible sur github), j'obtiens le résultat suivant :

```console
$ ./compile_pam_file.sh login
auth optional pam_faildelay.so delay=3000000
auth [success=ok new_authtok_reqd=ok ignore=ignore user_unknown=bad default=die] pam_securetty.so
auth requisite pam_nologin.so
session [success=ok ignore=ignore module_unknown=ignore default=bad] pam_selinux.so close
session required pam_env.so readenv=1
session required pam_env.so readenv=1 envfile=/etc/default/locale
auth    [success=1 default=ignore]    pam_unix.so nullok_secure
auth    requisite            pam_deny.so
auth    required            pam_permit.so
auth optional pam_group.so
session required pam_limits.so
session optional pam_lastlog.so
session optional pam_exec.so type=open_session stdout /bin/uname -snrvm
session optional pam_motd.so
session optional pam_mail.so standard
session required pam_loginuid.so
account    [success=1 new_authtok_reqd=done default=ignore]    pam_unix.so
account    requisite            pam_deny.so
account    required            pam_permit.so
session    [default=1]            pam_permit.so
session    requisite            pam_deny.so
session    required            pam_permit.so
session    required    pam_unix.so
session    optional    pam_systemd.so
password    [success=1 default=ignore]    pam_unix.so obscure sha512
password    requisite            pam_deny.so
password    required            pam_permit.so
password    optional    pam_gnome_keyring.so
session [success=ok ignore=ignore module_unknown=ignore default=bad] pam_selinux.so open
```

## Contenu d'un fichier service

Chaque ligne du fichier correspond à une règle, et chaque colonne d'une règle donne une information différente :

### [1. Type de tâche à invoquer (auth, account, password, session)](#1-type-de-tâche-à-invoquer-auth-account-password-session)

Les modules PAM peuvent remplir 1 à 4 des tâches présentés ci-dessous. Bien qu'un module puisse endosser toutes ces casquettes, il est préférable de créer un module par type de tâche afin de respecter l'esprit unix (chaque programme fait une seule chose et le fait bien).

#### auth

Vérification de l'identité. En général, il s'agit de vérifier le couple login/password basé sur `/etc/passwd` et `/etc/shadow`. On peut très bien envisager avoir une base de données sqlite au lieu de ces deux fichiers, d'avoir un système distant centralisé type LDAP, d'avoir une authentification forte à deux facteurs, etc.

#### account

Vérification que le compte (cette identité vérifiée à l'étape précédente) a bien le droit d'accéder au système actuellement. C'est ce qui permet, par exemple, de n'autoriser l'accès au système que certains jours de la semaine, etc.

#### password

Mise à jour du password et instauration de règles concernant les password. C'est ici que l'on va pouvoir imposer une longueur minimale aux passwords, la présence de caractères spéciaux, etc.

#### session

Actions à effectuer en début et fin de session. On peut ici définir des variables d'environnements, compter le nombre de sessions ouvertes simultanément, monter/démonter des partitions chiffrées, etc.

### [2. Rôle de la règle dans la chaîne de validation](#2-rôle-de-la-règle-dans-la-chaîne-de-validation)

#### requisite

Validation de cette étape requise, faute de quoi le reste des étapes n'est pas vérifié. L'échec de cette étape arrête tout et renvoit une erreur à l'application qui a fait appel à ce service.

#### required

Validation de cette étape requise. Si cette étape n'est pas validée, les étapes suivantes sont quand même évaluées mais le résultat final est déjà définit : l'application qui fait appel à ce service recevra une réponse négative. Evaluer la suite des étapes permet de brouiller les pistes pour un acteur malveillant. Il ne saura pas à quelle étape son attaque a échoué, il aura donc plus de mal à pénétrer le système ciblé.

#### sufficient

En cas de succès, et si aucune règle required n'a échoué auparavant, l'évaluation est arrêtée et l'application appelante reçoit une réponse positive.

#### optional

Le succès ou l'échec de cette étape n'aura aucune incidence sur la suite des opérations. Si toutes les règles du service sont définies en optional, la réponse est forcément positive.

> A noter que le contenu de cette deuxième colonne (requisite, required, sufficient, optional) liste les options que l'on trouve généralement dans un fichier de configuration. Ce sont en réalité des alias dont on peut se passer si l'on veut avoir un contrôle plus fin. On peut donc trouver des choses comme `[success=1 default=ignore]` ou encore `[success=ok ignore=ignore module_unknown=ignore default=bad]`. Pour avoir plus d'informations sur chacun de ces paramètres, je vous invite à lire le code source de PAM.

### [3. Nom du module](#3-nom-du-module)

C'est un nom de fichier `.so`, par exemple `pam_unix.so`. Sur ma machine les modules sont recherchés dans le dossier `/lib/x86_64-linux-gnu/security/`, donc c'est le fichier `/lib/x86_64-linux-gnu/security/pam_unix.so` qui sera chargé.

Il est aussi possible d'indiquer un chemin absolu plutôt que juste le nom du module à charger, par exemple : `/my/custom/directory/module.so`.

### [4. Éventuels arguments à passer au module](#4-Éventuels-arguments-à-passer-au-module)

A la façon d'argc/argv, on peut passer des paramètres au module (exemple : `debug=1`).

## Rendre une application _PAM aware_

Rendre une application PAM aware veut dire qu'on lui fait utiliser l'API PAM pour gérer l'authentification des utilisateurs. Nous allons écrire une application qui va faire appel au service login.

### [1. Dépendances](#dépendances)

Avant tout, il faut installer le paquet `libpam0g-dev`. C'est le nom du paquet sur debian, je vous laisse trouver l'équivalent si vous êtes sur une autre distribution.

```console
$ sudo apt-get install libpam0g-dev
```

### [2. Linker options](#2-linker-options)

Pour compiler un programme qui utilise la libpam, il faut ajouter à la ligne de compilation les options suivantes : `-lpam -lpam_misc`.

```console
$ gcc *.c -lpam -lpam_misc
```

### [3. Headers](#3-headers)

Les 3 headers qu'on retrouvera au début de nos fichiers .c seront :

```c
#include <security/pam_appl.h>
#include <security/pam_modules.h>
#include <security/pam_misc.h>
```

### [4. Fonctions](#4-fonctions)

Si vous souhaitez avoir une liste complète des fonctions disponibles :

```console
$ man -k pam
```

---

#### pam_start()

```c
int pam_start(
  const char *service_name,
  const char *user,
  const struct pam_conv *pam_conversation,
  pam_handle_t **pamh
);

```

`pam_start()` crée le contexte PAM (le handle) et initialise la transaction. C'est la première fonction à appeler quand on veut utiliser l'API PAM.

Paramètres :

* `service_name`

  Nom du service à invoquer. Ce service doit exister dans `/etc/pam.d/`.

* `user`

  Nom d'utilisateur à authentifier s'il est connu. On peut aussi passer `NULL` et laisser PAM déterminer lui-même le nom d'utilisateur.

* `pam_conversation`

  Structure qui précise la fonction à appeler pour communiquer avec l'utilisateur. Grâce à ce mécanisme, on peut aussi bien dialoguer sur un terminal que via une interface graphique.

* `pamh`

  Contexte (handle) créé par PAM, c'est une sorte d'identifiant de transaction. Cette structure est volontairement opaque. Ni les développeurs d'applications ni les développeurs de modules ne sont censés savoir ce qu'il y a dedans.

---

#### pam_end()

```c
int pam_end(
  pam_handle_t *pamh,
  int pam_status
);
```

Cette fonction mettra fin à la transaction initiée par `pam_start()` quand toutes les opérations liées à PAM seront terminées.
Paramètres :
* `pamh`
* `pam_status`

  Ce doit être la valeur de retour récupérée lors du dernier appel à une fonction PAM avant d'appeler `pam_end()`. Cela peut permettre aux modules de procéder à des opérations de dernière minute.

---

#### pam_authenticate()

```c
int pam_authenticate(
  pam_handle_t *pamh,
  int flags
);
```

Comme son nom l'indique, c'est cette fonction qui va authentifier l'utilisateur.

Paramètres :

* `pamh`

* `flags`

  Permet d'activer les flags `PAM_SILENT` et `PAM_DISALLOW_NULL_AUTHOK`.

---

#### pam_acct_mgmt()

```c
int pam_acct_mgmt(
  pam_handle_t *pamh,
  int flags
);
```

Si l'utilisateur est correctement authentifié, il faut maintenant s'assurer, grâce à cette fonction, qu'il a bien le droit de se connecter actuellement.

Paramètres :

* `pamh`

* `flags`

  Identique à `pam_authenticate()`.

---

#### pam_get_item()

```c
int pam_get_item(
  pam_handle_t *pamh,
  int item_type,
  const void **item
);
```

Cette fonction permet de récupérer des informations PAM comme, par exemple, le nom d'utilisateur ou encore le nom du service invoqué. Ce sont des informations stockées dans le handle (la structure obscure dont on a parlé plus tôt). Les données renvoyées par `pam_get_item()` ne doivent pas être libérées avec `free()` ni modifiées (d'où le modifier `const`). Elle seront automatiquement libérées lors de l'appel à `pam_end()`.

Paramètres :

* `pamh`

* `item_type`

  Information à récupére (voir la liste complète dans le man).

  Exemples : `PAM_USER, PAM_SERVICE`, `PAM_CONV`.

* `item`

  Adresse du pointeur qui recevra l'adresse mémoire où est stockée la valeur actuelle.

---

#### misc_conv

C'est une fonction proposée par `libpam_misc` qui s'occupe pour nous de converser avec l'utilisateur. Elle affichera un prompt et récupèrera l'input utilisateur pour lire login et password par exemple. Si l'on voulait récupérer ces informations via une interface graphique, on devrait alors ne pas utiliser cette fonction et coder la nôtre. De la même façon, si l'on souhaite ne pas échanger avec l'utilisateur sur l'entrée et la sortie standards mais, par exemple, via un périphérique externe (usb, etc.), il faudrait coder une fonction qui remplit cette tâche et l'utiliser à la place de `misc_conv`.

---

### [5. Exemple](#5-exemple)

Voici un exemple de programme qui va récupérer l'identité de l'utilisateur (login et password), vérifier qu'elle est valide, et vérifier que ce compte à bien le droit de se connecter. D'autres actions seraient possibles comme par exemple le fait d'appeler `pam_setcred()` pour la gestion des droits ou encore d'ouvrir/fermer une session avec les fonctions `pam_open_session()` et `pam_close_session()`, mais ce ne sera pas couvert dans le code ci-dessous par soucis de simplicité :

[Voir le code source sur github](https://github.com/yoones/yoonix/tree/master/linux-pam/pam-aware-program)

## Coder un module PAM

### [Service auth](#service-auth)

Les deux fonctions à implémenter sont les suivantes :

* `pam_sm_authenticate()`

* `pam_sm_setcred()`

Avant cela, il faut :

1. Définir la macro `PAM_SM_AUTH`

2. __Ensuite__ inclure `<security/pam_modules.h>`

3. Enfin, implémenter les deux fonctions présentées ci-dessous.

```c
#define PAM_SM_AUTH
#include <security/pam_modules.h>
```

---

#### pam_sm_authenticate()

```c
PAM_EXTERN int pam_sm_authenticate(
  pam_handle_t *pamh,
  int flags,
  int argc,
  const char **argv
);
```

> Quand une application appelle [`pam_authenticate()`](#pam_authenticate) (fonction exposée par l'API PAM), c'est cette fonction `pam_sm_authenticate()` (fonction exposée par le module) qui sera appelée.

Aucune surprise ici, cette fonction sert à implémenter la vérification d'identité.

Paramètres :

* `pamh`

  Contexte PAM (handle).

* `flags`

  Champs de bits servant à activer les modes `PAM_SILENT` et `PAM_DISALLOW_NULL_AUTHTOK`.

* `argc`

  Nombre d'arguments passés au module ([4ème colonne dans le fichier service](#4-Éventuels-arguments-à-passer-au-module)).

* `argv`

  Arguments passés au module ([4ème colonne dans le fichier service](#4-Éventuels-arguments-à-passer-au-module)).

Notez qu'ici, contrairement aux argc/argv passées à la fonction `main()`, le premier élément d'argv n'est pas le nom du module mais directement un paramètre (par exemple `debug=1`).

---

#### pam_sm_setcred()

```c
PAM_EXTERN int pam_sm_setcred(
  pam_handle_t *pamh,
  int flags,
  int argc,
  const char **argv
);
```

> Quand une application appelle [`pam_setcred()`](#pam_setcred) (fonction exposée par l'API PAM), c'est cette fonction `pam_sm_setcred()` (fonction exposée par le module) qui sera appelée.

Quand l'identité de l'utilisateur est vérifiée, certains modules ont besoin d'effectuer des actions supplémentaires liées aux _credentials_ comme :

* associer l'utilisateur à des groupes en plus de ceux auxquels il appartient déjà dans `/etc/group`,

* définir un ticket Kerberos,

* limiter l'accès à certaines ressources,

* etc.

C'est dans cette fonction que l'on peut procéder à ces opérations.

Paramètres :

* `pamh`

* `flags`

  Champs de bits servant à activer le mode `PAM_SILENT`.

  Ce champs de bits sert aussi à activer un seul des modes suivants à la fois : `PAM_ESTABLISH_CRED`, `PAM_DELETE_CRED`, `PAM_REINITIALIZE_CRED` ou `PAM_REFRESH_CRED`. Si aucun ou plusieurs de ces 4 modes sont activés, la fonction doit renvoyer un code d'erreur (voir le man).

* `argc`

* `argv`

---

> Note concernant l'architecture de modules PAM :
> 
> Certaines modules PAM nécessitent les privilèges root pour pouvoir effectuer certaines actions comme associer un utilisateur à des groupes supplémentaires (`setgroups()`), lire le fichier `/etc/shadow` pour en extraire des hash de password, etc. C'est pour cela que certains programmes _PAM aware_ ne peuvent être lancées qu'avec `sudo` ou par un programme comme `init` qui a des privilèges suffisants, car le module doit hériter de ces privilèges pour fonctionner.
> 
> Toutefois, des modules peuvent avoir besoin d'effectuer une opération qui nécessite les droits root sans que le programme _PAM aware_ qui les appelle n'ai ces privilèges. C'est le cas de `pam_unix.so` qui a besoin d'accéder à `/etc/shadow` pour vérifier le password que l'utilisateur lui donne. Pour cela, ce module a fait un choix très simple : fournir un programme setuid qui fait la vérification du password à la place du module. Le module peut donc indirectement accéder à /etc/shadow sans privilèges particuliers.

### [Service account](#service-account)

TODO

### [Service password](#service-password)

TODO

### [Service session](#service-session)

TODO

## [Exercice](#exercice)

TODO

## [Ressources](#ressources)

* http://wpollock.com/AUnix2/PAM-Help.htm

* http://www.linuxdevcenter.com/pub/a/linux/2002/05/02/pam_modules.html

* http://artisan.karma-lab.net/petite-introduction-a-pam

* https://www.freebsd.org/doc/en_US.ISO8859-1/articles/pam/pam-essentials.html

* https://www.netbsd.org/docs/guide/en/chap-pam.html

* Concernant l'utilisation de `syslog` : http://www.linux-pam.org/Linux-PAM-html/mwg-see-programming-syslog.html

## [Auteur](#auteur)

Ce document a été rédigé initialement par [Younes SERRAJ](https://github.com/yoones).

Contributions appréciées via l'[issues tracker](https://github.com/yoones/yoonix/issues) de github ou via une pull request.

## [Licence](#licence-) [![Licence Creative Commons](https://i.creativecommons.org/l/by/4.0/80x15.png)](http://creativecommons.org/licenses/by/4.0/)

Cette œuvre est mise à disposition selon les termes de la [Licence Creative Commons Attribution 4.0 International](http://creativecommons.org/licenses/by/4.0/)
