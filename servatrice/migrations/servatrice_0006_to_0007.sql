-- Servatrice db migration from version 6 to version 7

alter table cockatrice_rooms add permissionlevel varchar(20) not null after descr;

UPDATE cockatrice_schema_version SET version=7 WHERE version=6;
