-- Servatrice db migration from version 29 to version 30

ALTER TABLE cockatrice_users ADD COLUMN adminnotes mediumtext NOT NULL;

UPDATE cockatrice_schema_version SET version=30 WHERE version=29;
