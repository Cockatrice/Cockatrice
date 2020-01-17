-- Servatrice db migration from version 26 to version 27

ALTER TABLE cockatrice_users ADD COLUMN passwordLastChangedDate datetime NOT NULL DEFAULT '0000-00-00 00:00:00';

UPDATE cockatrice_schema_version SET version=27 WHERE version=26;
