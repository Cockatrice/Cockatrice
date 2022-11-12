-- Servatrice db migration from version 28 to version 29

ALTER TABLE cockatrice_users MODIFY COLUMN avatar_bmp mediumblob NOT NULL;

UPDATE cockatrice_schema_version SET version=29 WHERE version=28;
