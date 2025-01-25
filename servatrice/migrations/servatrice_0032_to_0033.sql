-- Servatrice db migration from version 32 to version 33

ALTER TABLE cockatrice_user_analytics ADD INDEX `idx_last_login` (`last_login`);

UPDATE cockatrice_schema_version SET version=33 WHERE version=32;
