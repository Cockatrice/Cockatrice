-- Servatrice db migration from version 31 to version 32

ALTER TABLE cockatrice_users ADD INDEX `idx_clientid` (`clientid`);
ALTER TABLE cockatrice_sessions ADD INDEX `idx_clientid` (`clientid`);
ALTER TABLE cockatrice_sessions ADD INDEX `idx_ip_address` (`ip_address`);
ALTER TABLE cockatrice_bans ADD INDEX `idx_user_name` (`user_name`);
ALTER TABLE cockatrice_warnings ADD INDEX `idx_time_of` (`time_of`);
ALTER TABLE cockatrice_warnings ADD INDEX `idx_user_name` (`user_name`);
ALTER TABLE cockatrice_log ADD INDEX `idx_log_time` (`log_time`);

UPDATE cockatrice_schema_version SET version=32 WHERE version=31;
