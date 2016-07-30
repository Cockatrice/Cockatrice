-- Servatrice db migration from version 16 to version 17

alter table cockatrice_rooms modify column `id_server` tinyint(3) not null default 1;
alter table cockatrice_rooms_gametypes modify column `id_server` tinyint(3) not null default 1;
alter table cockatrice_servermessages modify column `id_server` tinyint(3) not null default 1;

UPDATE cockatrice_schema_version SET version=17 WHERE version=16;
