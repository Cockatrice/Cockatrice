-- Servatrice db migration from version 17 to version 18

alter table cockatrice_users add `donator` tinyint(1) not null default 0;

UPDATE cockatrice_schema_version SET version=18 WHERE version=17;
