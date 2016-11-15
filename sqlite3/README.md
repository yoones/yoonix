# SQLite3

## Overview

SQLite3 est un moteur de base de données SQL :

* relationnel

* sans serveur (contrairement à MySQL, PostgreSQL, etc.)

* sans fichiers de configuration

* sans autre dépendance que la `libc` (donc particulièrement facile à déployer)

Cette bibliothèque de fonctions permet donc à une application de structurer et gérer ses données persistentes en toute simplicité.

Nous allons voir dans ce document comment :

1. Préparer une base de données sqlite3 (création de la base de données et de ses tables)

2. Manipuler des données (insert, select, update, delete)

__Prérequis__ : connaitre la syntaxe SQL.

Ce document sera accompagné d'un Live Coding (vidéo) et le code est disponible sur github.

![Comme toujours, je vous invite fortement à lire la documentation officielle](http://messages.hellobits.com/warning.svg?message=Comme%20toujours%2C%20je%20vous%20invite%20fortement%20%C3%A0%20lire%20les%20man%20de%20chaque%20fonction%20avant%20de%20les%20utiliser)

Sommaire
---

- [Installer sqlite3](#installer-sqlite3)
- [Utiliser sqlite3](#utiliser-sqlite3)
	- [En ligne de commande](#en-ligne-de-commande)
	- [En C](#en-c)
		- [Linker options](#linker-options)
		- [Headers](#headers)
		- [Fonctions](#fonctions)
			- [sqlite3_open()](#sqlite3_open)
			- [sqlite3_errmsg()](#sqlite3_errmsg)
			- [sqlite3_free()](#sqlite3_free)
			- [sqlite3_exec()](#sqlite3_exec)
			- [sqlite3_exec() callback](#sqlite3_exec-callback)
			- [sqlite3_close()](#sqlite3_close)
		- [Exemple](#exemple)
- [Exercice](#exercice)
- [Ressources](#ressources)
- [Auteur](#auteur)
- [Licence](#licence-)

## Installer sqlite3

Nous avons deux paquets à installer. Je donne ici les noms des paquets sur debian, je vous laisse trouver les équivalents si vous êtes sur une autre distribution.

Pour utiliser l'outil en ligne de commande :

```console
$ sudo apt-get install sqlite3
```

Pour installer la bibliothèque (pour coder) :

```console
$ sudo apt-get install libsqlite3-dev
```

## Utiliser sqlite3

### En ligne de commande

TODO

### En C

#### [Linker options](#2-linker-options)

Pour compiler un programme qui utilise sqlite3, il faut ajouter à la ligne de compilation l'option suivante : `-lsqlite3`.

```console
$ gcc *.c -lsqlite3
```

#### [Header](#3-headers)

On retrouvera au début de nos fichiers .c le header suivant :

```c
#include <sqlite3.h>
```

#### [Fonctions](#4-fonctions)

Si vous souhaitez avoir une liste complète des fonctions disponibles :

https://www.sqlite.org/c3ref/funclist.html

---

##### sqlite3_open()

```c
int sqlite3_open(
  const char *filename,
  sqlite3 **ppDb
);
```

`sqlite3_open()` ouvre un fichier considéré comme une base de données SQL.

Paramètres :

* `filename`

  Adresse du fichier à ouvrir.

* `ppDb`

  Handle qui sera créé par sqlite3. C'est l'équivalent d'un file descriptor.
  
  Notez que même en cas d'erreur à l'ouverture d'une base de données, `ppDb` sera définit et il faudra appeler `sqlite3_close()` pour le libérer.
  
Si tout se passe bien, cette fonction renvoie `SQLITE_OK`. Dans le cas contraire, un code d'erreur est renvoyé et `sqlite3_errmsg()` peut être utilisé pour récupérer une description en anglais du problème (à la façon de `strerror()`).

> Si votre application est multi-tâches, regardez du côté de `sqlite3_open_v2()`.

---

##### sqlite3_errmsg()

```c
const char *sqlite3_errmsg(
  sqlite3 *pDb
);
```

Cette fonction renvoie une chaine de caractères (constante) qui décrit (en anglais) la dernière erreur rencontrée par sqlite3.

---

##### sqlite3_free

```c
void sqlite3_free(
  void *data
);
```

Cette fonction va libérer une ressource initalement allouée à l'aide de `sqlite3_*alloc*()`.

Paramètres :

* `data`

  Adresse de la donnée à libérer

---

##### sqlite3_exec()

```c
int sqlite3_exec(
  sqlite3 *pDb,
  const char *sql,
  int (*callback)(void *, int, char **, char **),
  void *callback_data,
  char **errmsg
);
```

C'est le nerf de la guerre. `sqlite3_exec()` est la fonction qui vous permettra d'exécuter des requêtes SQL.

Paramètres :

* `pDb`

  Handle d'une base de données ouverte.

* `sql`

  Requête SQL à exécuter (n'oubliez pas le `;` à la fin des requêtes).

* `callback`

  Adresse de la fonction à appeler si la requête produit un résultat (ex: requêtes SQL de type `select`).
  
  Si vous n'attendez pas de résultat, vous pouvez passer `NULL` en paramètre.

* `callback_data`

  1er argument qui sera passé à `callback`.

* `errmsg`

  Eventuel message d'erreur. Cette valeur devra être libérée grâce à `sqlite3_free`.

Cette fonction renvoie `SQLITE_OK` si tout s'est bien passé.

Si cette fonction renvoie `SQLITE_ABORT`, c'est qu'un appel à la callback a renvoyé une valeur différente de zéro.

__Important :__

`sqlite3_exec()` est une fonction synchrone. Elle fera autant d'appels à `callback` que nécessaire __avant__ de renvoyer `SQLITE_OK` si tout s'est bien passé ou un code d'erreur dans le cas contraire.

---

##### sqlite3_exec() callback

```c
int callback(
  void *data,
  int argc,
  char **argv,
  char **colums_names
);
```

Quand une requête SQL doit renvoyer des résultats, c'est une fonction callback qui sera appelée. Cette fonction doit respecter le prototype ci-dessus.

Si, par exemple, une requête `select` produit 3 résultats, la callback sera appelée 3 fois.

Paramètres :

* `data`

  4ème paramètre passé à `sqlite3_exec()`.

* `argc`

  Nombre de colonnes

* `argv`

  Contenu de chacune des colonnes

* `colums_names`

  Nom de chacune des colonnes

Si la callback souhaite faire échouer l'appel à `sqlite3_exec()` (peu importe la raison), il lui suffit de renvoyer une valeur différente de 0. Dans ce cas, la requête SQL actuelle sera avortée et aucune requête SQL ultérieur ne sera exécutée.

__Exemple :__

Pour la requête suivante :

```sql
select firstname,lastname from users WHERE role="member";
```

`callback` sera appelé autant de fois que de tuples correspondant à ma requête seront trouvés. Chaque appel à `callback` me permettra de récupérer les informations suivantes :

* `argc` = 2
* `argv[0]` = "... un prénom ..."
* `argv[1]` = "... un nom ..."
* `colums_names[0]` = "firstname"
* `colums_names[1]` = "lastname"

---

##### sqlite3_close

```c
int sqlite3_close(
  sqlite3 *pDb
);
```

Ferme une base de données ouverte à l'aide de `sqlite3_open()`.

---

#### [Exemple](#exemple-de-module)

TODO

[Voir le code source sur github](https://github.com/yoones/yoonix/tree/master/sqlite3/exemple)

## [Exercice](#exercice)

TODO

## [Ressources](#ressources)

* https://www.sqlite.org/c3ref/funclist.html

## [Auteur](#auteur)

Ce document a été rédigé initialement par [Younes SERRAJ](https://github.com/yoones).

Contributions appréciées via l'[issues tracker](https://github.com/yoones/yoonix/issues) de github ou via une pull request.

## [Licence](#licence-) [![Licence Creative Commons](https://i.creativecommons.org/l/by/4.0/80x15.png)](http://creativecommons.org/licenses/by/4.0/)

Cette œuvre est mise à disposition selon les termes de la [Licence Creative Commons Attribution 4.0 International](http://creativecommons.org/licenses/by/4.0/)
