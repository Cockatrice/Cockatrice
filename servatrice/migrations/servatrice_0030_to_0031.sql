-- Servatrice db migration from version 30 to version 31

ALTER TABLE cockatrice_log DROP INDEX `target_type`;

ALTER TABLE cockatrice_forgot_password ADD INDEX idx_emailed (`emailed`);
ALTER TABLE cockatrice_sessions ADD INDEX idx_start_time (`start_time`);
ALTER TABLE cockatrice_users ADD INDEX idx_admin (`admin`);
ALTER TABLE cockatrice_users ADD INDEX idx_active (`active`);
ALTER TABLE cockatrice_users ADD INDEX idx_privlevel (`privlevel`);

UPDATE cockatrice_schema_version SET version=31 WHERE version=30;
