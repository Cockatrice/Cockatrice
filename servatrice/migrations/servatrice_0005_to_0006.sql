-- Servatrice db migration from version 5 to version 6

alter table cockatrice_users add last_login datetime not null;

UPDATE cockatrice_schema_version SET version=6 WHERE version=5;
