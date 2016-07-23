-- Servatrice db migration from version 14 to version 15

alter table cockatrice_rooms add `id_server` tinyint(3) not null default 0;
alter table cockatrice_rooms_gametypes add `id_server` tinyint(3) not null default 0;

UPDATE cockatrice_schema_version SET version=15 WHERE version=14;
