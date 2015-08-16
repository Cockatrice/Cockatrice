-- Servatrice db migration from version 2 to version 3

alter table cockatrice_users add clientid varchar(15) not null;

UPDATE cockatrice_schema_version SET version=3 WHERE version=2;
