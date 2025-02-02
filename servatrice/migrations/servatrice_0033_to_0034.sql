-- Servatrice db migration from version 33 to version 34

ALTER TABLE cockatrice_users ADD COLUMN leftPawnColorOverride varchar(255);
ALTER TABLE cockatrice_users ADD COLUMN rightPawnColorOverride varchar(255);

ALTER TABLE cockatrice_users ADD INDEX `idx_pawnColorOverrides` (`leftPawnColorOverride`, `rightPawnColorOverride`);

UPDATE cockatrice_schema_version SET version=34 WHERE version=33;
