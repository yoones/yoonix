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

![Comme toujours, je vous invite fortement à lire la documentation officielle](http://messages.hellobits.com/warning.svg?message=Comme%20toujours%2C%20je%20vous%20invite%20fortement%20%C3%A0%20lire%20la%20documentation%20officielle)

Sommaire
---

- [sqlite3 en ligne de commande](#sqlite3-en-ligne-de-commande)
	- [Installation](#installation)
	- [Démo](#démo)
- [libsqlite3](#libsqlite3)
	- [1. Dépendance](#1-dépendance)
	- [2. Linker options](#2-linker-options)
	- [3. Headers](#3-headers)
	- [4. Fonctions](#4-fonctions)
		- [sqlite3_open()](#sqlite3_open)
		- [sqlite3_errmsg()](#sqlite3_errmsg)
		- [sqlite3_free()](#sqlite3_free)
		- [sqlite3_exec()](#sqlite3_exec)
		- [sqlite3_exec() callback](#sqlite3_exec-callback)
		- [sqlite3_close()](#sqlite3_close)
	- [5. Exemple](#5-exemple)
- [Exercice](#exercice)
- [Ressources](#ressources)
- [Auteur](#auteur)
- [Licence](#licence-)

## sqlite3 en ligne de commande

### [Installation](#installation)

Pour utiliser l'outil en ligne de commande :

```console
$ sudo apt-get install sqlite3
```

### [Démo](#démo)

On va commencer par lancer l'outil en ligne de commande et afficher l'aide grâce à la commande `.help` :

```console
$ sqlite3
SQLite version 3.8.7.1 2014-10-29 13:59:56
Enter ".help" for usage hints.
Connected to a transient in-memory database.
Use ".open FILENAME" to reopen on a persistent database.
sqlite> .help
.backup ?DB? FILE      Backup DB (default "main") to FILE
.bail on|off           Stop after hitting an error.  Default OFF
.clone NEWDB           Clone data into NEWDB from the existing database
.databases             List names and files of attached databases
.dump ?TABLE? ...      Dump the database in an SQL text format
                         If TABLE specified, only dump tables matching
                         LIKE pattern TABLE.
.echo on|off           Turn command echo on or off
.eqp on|off            Enable or disable automatic EXPLAIN QUERY PLAN
.exit                  Exit this program
.explain ?on|off?      Turn output mode suitable for EXPLAIN on or off.
                         With no args, it turns EXPLAIN on.
.fullschema            Show schema and the content of sqlite_stat tables
.headers on|off        Turn display of headers on or off
.help                  Show this message
.import FILE TABLE     Import data from FILE into TABLE
.indices ?TABLE?       Show names of all indices
                         If TABLE specified, only show indices for tables
                         matching LIKE pattern TABLE.
.load FILE ?ENTRY?     Load an extension library
.log FILE|off          Turn logging on or off.  FILE can be stderr/stdout
.mode MODE ?TABLE?     Set output mode where MODE is one of:
                         csv      Comma-separated values
                         column   Left-aligned columns.  (See .width)
                         html     HTML <table> code
                         insert   SQL insert statements for TABLE
                         line     One value per line
                         list     Values delimited by .separator string
                         tabs     Tab-separated values
                         tcl      TCL list elements
.nullvalue STRING      Use STRING in place of NULL values
.once FILENAME         Output for the next SQL command only to FILENAME
.open ?FILENAME?       Close existing database and reopen FILENAME
.output ?FILENAME?     Send output to FILENAME or stdout
.print STRING...       Print literal STRING
.prompt MAIN CONTINUE  Replace the standard prompts
.quit                  Exit this program
.read FILENAME         Execute SQL in FILENAME
.restore ?DB? FILE     Restore content of DB (default "main") from FILE
.save FILE             Write in-memory database into FILE
.schema ?TABLE?        Show the CREATE statements
                         If TABLE specified, only show tables matching
                         LIKE pattern TABLE.
.separator STRING ?NL? Change separator used by output mode and .import
                         NL is the end-of-line mark for CSV
.shell CMD ARGS...     Run CMD ARGS... in a system shell
.show                  Show the current values for various settings
.stats on|off          Turn stats on or off
.system CMD ARGS...    Run CMD ARGS... in a system shell
.tables ?TABLE?        List names of tables
                         If TABLE specified, only list tables matching
                         LIKE pattern TABLE.
.timeout MS            Try opening locked tables for MS milliseconds
.timer on|off          Turn SQL timer on or off
.trace FILE|off        Output each SQL statement as it is run
.vfsname ?AUX?         Print the name of the VFS stack
.width NUM1 NUM2 ...   Set column widths for "column" mode
                         Negative values right-justify
```

Première chose à distinguer : les requêtes SQL des commandes propres à sqlite3.

* Quand on veut __exécuter une requête SQL__, il suffit de l'écrire et de terminer la requête par un `;`.

* Quand on veut __exécuter une commande sqlite3__ (comme `.tables`), on n'oublie pas le `.` en début de commande et on ne mets pas de `;` en fin de commande.

Ouvrons maintenant une base de données `db.sqlite3` et créons une table nommée `tasks` :

```console
sqlite> .open db.sqlite3
sqlite> .tables
sqlite> create table tasks (id integer primary key autoincrement, description text);
sqlite> .tables
tasks
sqlite> .schema tasks
CREATE TABLE tasks (id integer primary key autoincrement, description text);
```

C'est tout simple vu que c'est une requête SQL `create table ...`.

Je me suis servi également des commandes suivantes :

* `.open` : ouvre la base de données passée en paramètre

* `.tables` : liste les tables existantes

* `.schema` : un peu comme le `DESCRIBE` de MySQL, cette commande décrit une table (liste les champs, leurs types, ...)

Maintenant qu'une table existe, on peut lui ajouter quelques tuples :

```console
sqlite> insert into tasks (description) values ("My very first task");
sqlite> select * from tasks;
1|My very first task
sqlite> insert into tasks (description) values ("Another tasks");
sqlite> select * from tasks;
1|My very first task
2|Another task
sqlite> select * from tasks order by id desc
   ...> ;
2|Another task
1|My very first task
sqlite> .quit
```

Si on oublie le `;` à la fin de la ligne, sqlite3 considère que la requête n'est pas terminée et nous permet de continuer à l'écrire sur la ligne suivante.

Pour conclure, on ouvre à nouveau la base de données que l'on vient de créer et de peupler pour s'assurer qu'elle contient toujours nos données :

```console
$ sqlite3 db.sqlite3
SQLite version 3.8.7.1 2014-10-29 13:59:56
Enter ".help" for usage hints.
sqlite> .tables
tasks
sqlite> select * from tasks;
1|My very first task
2|Another task
sqlite> .quit
```

## libsqlite3

### [1. Dépendance](#1-dépendance)

Pour installer la bibliothèque sur debian :

```console
$ sudo apt-get install libsqlite3-dev
```

_Je vous laisse trouver l'équivalent si vous êtes sur une autre distribution._

### [2. Linker options](#2-linker-options)

Pour compiler un programme qui utilise sqlite3, il faut ajouter à la ligne de compilation l'option suivante : `-lsqlite3`.

```console
$ gcc *.c -lsqlite3
```

### [3. Header](#3-headers)

On retrouvera au début de nos fichiers .c le header suivant :

```c
#include <sqlite3.h>
```

### [4. Fonctions](#4-fonctions)

Si vous souhaitez avoir une liste complète des fonctions disponibles :

https://www.sqlite.org/c3ref/funclist.html

---

#### sqlite3_open()

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

#### sqlite3_errmsg()

```c
const char *sqlite3_errmsg(
  sqlite3 *pDb
);
```

Cette fonction renvoie une chaine de caractères (constante) qui décrit (en anglais) la dernière erreur rencontrée par sqlite3.

---

#### sqlite3_free

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

#### sqlite3_exec()

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

  Requête SQL à exécuter.

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

#### sqlite3_exec() callback

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

#### sqlite3_close

```c
int sqlite3_close(
  sqlite3 *pDb
);
```

Ferme une base de données ouverte à l'aide de `sqlite3_open()`.

---

### [5. Exemple](#5-exemple)

Afin d'illustrer tout ça, vous pouvez suivre le lien ci-dessous pour voir un code très basique qui ouvre une base de données et affiche le contenu trouvé dans la table `tasks`.

[Voir le code source sur github](https://github.com/yoones/yoonix/tree/master/sqlite3/exemple)

## [Exercice](#exercice)

Voici quelques petits exercices à faire pour vous aider à prendre cette bibliothèque en main :

* Ecrire un gestionnaire de tâches

  Difficulté : facile

* Ecrire un gestionnaire de logs qui :
  
  - lit sur l'entrée standard des lignes de texte
  
  - les enregistre dans une table `random_logs`.
  
  Ce programme doit pouvoir être lancé plusieurs fois en simultané sur la même database sans la corrompre.

  Difficulté : moyenne

Et comme toujours, faites bien attention à l'SQL Injection ;)

## [Ressources](#ressources)

* https://www.sqlite.org/c3ref/funclist.html

* Concernant l'accès concurenciel : http://www.sqlite.org/lockingv3.html

## [Auteur](#auteur)

Ce document a été rédigé initialement par [Younes SERRAJ](https://github.com/yoones).

Contributions appréciées via l'[issues tracker](https://github.com/yoones/yoonix/issues) de github ou via une pull request.

## [Licence](#licence-) [![Licence Creative Commons](https://i.creativecommons.org/l/by/4.0/80x15.png)](http://creativecommons.org/licenses/by/4.0/)

Cette œuvre est mise à disposition selon les termes de la [Licence Creative Commons Attribution 4.0 International](http://creativecommons.org/licenses/by/4.0/)
