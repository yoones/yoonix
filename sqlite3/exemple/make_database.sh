#!/bin/bash

set -e

database_path="./db.sqlite"

if [ -f "$database_path" ]; then
    echo "Error: '$database_path' already exists. Aborting to prevent data loss." > /dev/stderr
    exit 1
fi

cat <<EOF | sqlite3 db.sqlite
create table if not exists tasks (id integer primary key autoincrement not null, description text not null);
insert into tasks (description) values ("first task");
insert into tasks (description) values ("second task");
insert into tasks (description) values ("third and last task");
.quit
EOF
