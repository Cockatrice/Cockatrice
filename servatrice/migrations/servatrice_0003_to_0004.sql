-- Servatrice db migration from version 3 to version 4

alter table cockatrice_sessions add clientid varchar(15) not null;

UPDATE cockatrice_schema_version SET version=4 WHERE version=3;
