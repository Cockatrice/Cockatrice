#!/bin/bash

set -e

version_line="$(grep 'INSERT INTO cockatrice_schema_version' servatrice/servatrice.sql)"
version_line="${version_line#*VALUES(}"
declare -i schema_ver="${version_line%%)*}"

latest_migration="$(ls -1 servatrice/migrations/ | tail -n1)"
xtoysql="${latest_migration#servatrice_}"
xtoy="${xtoysql%.sql}"
declare -i old_ver="10#${xtoy%_to_*}" #declare as integer with base 10, numbers with a leading 0 are normally interpreted as base 16
declare -i new_ver="10#${xtoy#*_to_}"

if ((old_ver >= new_ver)); then
    echo "New version $new_ver is not newer than $old_ver"
    exit 1
fi

if ((schema_ver != new_ver)); then
    echo "Schema version $schema_ver does not equal new version $new_ver"
    exit 1
fi

expected_sql="^UPDATE cockatrice_schema_version SET version=${new_ver} WHERE version=${old_ver};$"
if ! grep -q "$expected_sql" servatrice/migrations/$latest_migration; then
    echo "$latest_migration does not contain expected sql: $expected_sql"
    exit 1
fi

expected_define="^#define DATABASE_SCHEMA_VERSION $new_ver$"
if ! grep -q "$expected_define" servatrice/src/servatrice_database_interface.h; then
    echo "servatrice_database_interface.h does not contain expected #define: $expected_define"
    exit 1
fi
