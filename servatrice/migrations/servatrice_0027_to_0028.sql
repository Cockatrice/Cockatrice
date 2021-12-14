-- Servatrice db migration from version 27 to version 28

ALTER TABLE cockatrice_users DROP COLUMN gender;

UPDATE cockatrice_schema_version SET version=28 WHERE version=27;
