-- Servatrice db migration from version 11 to version 12

alter table cockatrice_users modify token binary(16) NULL;

UPDATE cockatrice_schema_version SET version=12 WHERE version=11;
