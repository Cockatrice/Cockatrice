-- Servatrice db migration from version 4 to version 5

alter table cockatrice_bans add clientid varchar(15) not null;

UPDATE cockatrice_schema_version SET version=5 WHERE version=4;
