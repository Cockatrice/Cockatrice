-- Servatrice db migration from version 24 to version 25

ALTER TABLE cockatrice_uptime ADD COLUMN mods_count int(11) NOT NULL DEFAULT 0;
ALTER TABLE cockatrice_uptime ADD COLUMN mods_list TEXT;

UPDATE cockatrice_schema_version SET version=25 WHERE version=24;
