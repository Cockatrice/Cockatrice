-- Servatrice db migration from version 7 to version 8

alter table cockatrice_users add role tinyint(1) not null after admin;

UPDATE cockatrice_schema_version SET version=8 WHERE version=7;
