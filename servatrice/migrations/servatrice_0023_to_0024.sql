-- Servatrice db migration from version 23 to version 24

SET FOREIGN_KEY_CHECKS=0;

-- short the "ip address" columns to 45 chars (max length of an ipv6 address)
-- to ensure the field can be used as a key on mysql < 5.7
-- (not all fields are actually keys, but better keep them uniform)
ALTER TABLE `cockatrice_sessions` MODIFY COLUMN `ip_address` varchar(45) NOT NULL;
ALTER TABLE `cockatrice_bans` MODIFY COLUMN `ip_address` varchar(45) NOT NULL;
ALTER TABLE `cockatrice_log` MODIFY COLUMN `sender_ip` varchar(45) NOT NULL;
ALTER TABLE `cockatrice_audit` MODIFY COLUMN `ip_address` varchar(45) NOT NULL;

-- short the "user name" columns to 35 chars (current max length in servatrice)
-- to ensure the field can be used as a key on mysql < 5.7
-- (not all fields are actually keys, but better keep them uniform)
ALTER TABLE `cockatrice_bans` MODIFY COLUMN `user_name` varchar(35) NOT NULL;
ALTER TABLE `cockatrice_warnings` MODIFY COLUMN `user_name` varchar(35) NOT NULL;
ALTER TABLE `cockatrice_warnings` MODIFY COLUMN `mod_name` varchar(35) NOT NULL;
ALTER TABLE `cockatrice_games` MODIFY COLUMN `creator_name` varchar(35) NOT NULL;
ALTER TABLE `cockatrice_games_players` MODIFY COLUMN `player_name` varchar(35) NOT NULL;
ALTER TABLE `cockatrice_donations` MODIFY COLUMN `username` varchar(35) NOT NULL;

-- remove the FK from cockatrice_activation_emails (it will be created again later)
-- the key name should end with _1, but multiple run of the 0012_to_003 migration
-- can lead to a different name or even multiple keys. In this case you must remove
-- all of them before continue.
-- Use "show create table cockatrice_activation_emails" to see the key names.
ALTER TABLE cockatrice_activation_emails DROP FOREIGN KEY `cockatrice_activation_emails_ibfk_1`;

-- unify tables and columns collation to utf8mb4_unicode_ci
ALTER TABLE `cockatrice_schema_version` CONVERT TO CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci;
ALTER TABLE `cockatrice_users` CONVERT TO CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci;
ALTER TABLE `cockatrice_decklist_files` CONVERT TO CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci;
ALTER TABLE `cockatrice_decklist_folders` CONVERT TO CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci;
ALTER TABLE `cockatrice_ignorelist` CONVERT TO CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci;
ALTER TABLE `cockatrice_buddylist` CONVERT TO CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci;
ALTER TABLE `cockatrice_rooms` CONVERT TO CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci;
ALTER TABLE `cockatrice_rooms_gametypes` CONVERT TO CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci;
ALTER TABLE `cockatrice_games` CONVERT TO CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci;
ALTER TABLE `cockatrice_games_players` CONVERT TO CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci;
ALTER TABLE `cockatrice_replays` CONVERT TO CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci;
ALTER TABLE `cockatrice_replays_access` CONVERT TO CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci;
ALTER TABLE `cockatrice_servers` CONVERT TO CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci;
ALTER TABLE `cockatrice_uptime` CONVERT TO CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci;
ALTER TABLE `cockatrice_servermessages` CONVERT TO CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci;
ALTER TABLE `cockatrice_sessions` CONVERT TO CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci;
ALTER TABLE `cockatrice_bans` CONVERT TO CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci;
ALTER TABLE `cockatrice_warnings` CONVERT TO CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci;
ALTER TABLE `cockatrice_log` CONVERT TO CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci;
ALTER TABLE `cockatrice_activation_emails` CONVERT TO CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci;
ALTER TABLE `cockatrice_user_analytics` CONVERT TO CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci;
ALTER TABLE `cockatrice_donations` CONVERT TO CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci;
ALTER TABLE `cockatrice_forgot_password` CONVERT TO CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci;
ALTER TABLE `cockatrice_audit` CONVERT TO CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci;

-- re-add the FK constraint on cockatrice_activation_emails
ALTER TABLE `cockatrice_activation_emails` ADD FOREIGN KEY(`name`) REFERENCES `cockatrice_users`(`name`) ON DELETE CASCADE ON UPDATE CASCADE;

SET FOREIGN_KEY_CHECKS=1;

-- update schema version
UPDATE cockatrice_schema_version SET version=24 WHERE version=23;
