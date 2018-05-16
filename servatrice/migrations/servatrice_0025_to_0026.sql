-- Servatrice db migration from version 25 to version 26

-- Some previous migrations didn't care about column ordering,
-- meaning select could return info in a different order depending on the
-- age of the database.

-- This migration ensures a consistent column ordering across all databases,
-- regardless of age. Future migrations should take care to ensure this
-- ordering stays consistent.

ALTER TABLE cockatrice_users MODIFY `id` int(7) unsigned zerofill NOT NULL FIRST;
ALTER TABLE cockatrice_users MODIFY `admin` tinyint(1) NOT NULL AFTER `id`;
ALTER TABLE cockatrice_users MODIFY `name` varchar(35) NOT NULL AFTER `admin`;
ALTER TABLE cockatrice_users MODIFY `realname` varchar(255) NOT NULL AFTER `name`;
ALTER TABLE cockatrice_users MODIFY `gender` char(1) NOT NULL AFTER `realname`;
ALTER TABLE cockatrice_users MODIFY `password_sha512` char(120) NOT NULL AFTER `gender`;
ALTER TABLE cockatrice_users MODIFY `email` varchar(255) NOT NULL AFTER `password_sha512`;
ALTER TABLE cockatrice_users MODIFY `country` char(2) NOT NULL AFTER `email`;
ALTER TABLE cockatrice_users MODIFY `avatar_bmp` blob NOT NULL AFTER `country`;
ALTER TABLE cockatrice_users MODIFY `registrationDate` datetime NOT NULL AFTER `avatar_bmp`;
ALTER TABLE cockatrice_users MODIFY `active` tinyint(1) NOT NULL AFTER `registrationDate`;
ALTER TABLE cockatrice_users MODIFY `token` binary(16) AFTER `active`;
ALTER TABLE cockatrice_users MODIFY `clientid` varchar(15) NOT NULL AFTER `token`;
ALTER TABLE cockatrice_users MODIFY `privlevel` enum("NONE","VIP","DONATOR") NOT NULL AFTER `clientid`;
ALTER TABLE cockatrice_users MODIFY `privlevelStartDate` datetime NOT NULL AFTER `privlevel`;
ALTER TABLE cockatrice_users MODIFY `privlevelEndDate` datetime NOT NULL AFTER `privlevelStartDate`;

ALTER TABLE cockatrice_decklist_files MODIFY `id` int(7) unsigned zerofill NOT NULL auto_increment FIRST;
ALTER TABLE cockatrice_decklist_files MODIFY `id_folder` int(7) unsigned zerofill NOT NULL AFTER `id`;
ALTER TABLE cockatrice_decklist_files MODIFY `id_user` int(7) unsigned NULL AFTER `id_folder`;
ALTER TABLE cockatrice_decklist_files MODIFY `name` varchar(50) NOT NULL AFTER `id_user`;
ALTER TABLE cockatrice_decklist_files MODIFY `upload_time` datetime NOT NULL AFTER `name`;
ALTER TABLE cockatrice_decklist_files MODIFY `content` text NOT NULL AFTER `upload_time`;

ALTER TABLE cockatrice_decklist_folders MODIFY `id` int(7) unsigned zerofill NOT NULL auto_increment FIRST;
ALTER TABLE cockatrice_decklist_folders MODIFY `id_parent` int(7) unsigned zerofill NOT NULL AFTER `id`;
ALTER TABLE cockatrice_decklist_folders MODIFY `id_user` int(7) unsigned NULL AFTER `id_parent`;
ALTER TABLE cockatrice_decklist_folders MODIFY `name` varchar(30) NOT NULL AFTER `id_user`;

ALTER TABLE cockatrice_ignorelist MODIFY `id_user1` int(7) unsigned NOT NULL FIRST;
ALTER TABLE cockatrice_ignorelist MODIFY `id_user2` int(7) unsigned NOT NULL AFTER `id_user1`;

ALTER TABLE cockatrice_buddylist MODIFY `id_user1` int(7) unsigned NOT NULL FIRST;
ALTER TABLE cockatrice_buddylist MODIFY `id_user2` int(7) unsigned NOT NULL AFTER `id_user1`;

ALTER TABLE cockatrice_rooms MODIFY `id` int(7) unsigned NOT NULL auto_increment FISRT;
ALTER TABLE cockatrice_rooms MODIFY `name` varchar(50) NOT NULL AFTER `id`;
ALTER TABLE cockatrice_rooms MODIFY `descr` varchar(255) NOT NULL AFTER `name`;
ALTER TABLE cockatrice_rooms MODIFY `permissionlevel` enum('NONE','REGISTERED','MODERATOR','ADMINISTRATOR') NOT NULL AFTER `descr`;
ALTER TABLE cockatrice_rooms MODIFY `privlevel` enum('NONE','PRIVILEGED','VIP','DONATOR') NOT NULL AFTER `permissionlevel`;
ALTER TABLE cockatrice_rooms MODIFY `auto_join` tinyint(1) default 0 AFTER `privlevel`;
ALTER TABLE cockatrice_rooms MODIFY `join_message` varchar(255) NOT NULL AFTER `auto_join`;
ALTER TABLE cockatrice_rooms MODIFY `chat_history_size` int(4) NOT NULL AFTER `join_message`;
ALTER TABLE cockatrice_rooms MODIFY `id_server` tinyint(3) NOT NULL DEFAULT 1 AFTER `chat_history_size`;

ALTER TABLE cockatrice_rooms_gametypes MODIFY `id_room` int(7) unsigned NOT NULL FIRST;
ALTER TABLE cockatrice_rooms_gametypes MODIFY `name` varchar(50) NOT NULL AFTER `id_room`;
ALTER TABLE cockatrice_rooms_gametypes MODIFY `id_server` tinyint(3) NOT NULL DEFAULT 1 AFTER `name`;

ALTER TABLE cockatrice_games MODIFY `room_name` varchar(255) NOT NULL FIRST;
ALTER TABLE cockatrice_games MODIFY `id` int(7) unsigned NOT NULL auto_increment AFTER `room_name`;
ALTER TABLE cockatrice_games MODIFY `descr` varchar(50) default NULL AFTER `id`;
ALTER TABLE cockatrice_games MODIFY `creator_name` varchar(35) NOT NULL AFTER `descr`;
ALTER TABLE cockatrice_games MODIFY `password` tinyint(1) NOT NULL AFTER `creator_name`;
ALTER TABLE cockatrice_games MODIFY `game_types` varchar(255) NOT NULL AFTER `password`;
ALTER TABLE cockatrice_games MODIFY `player_count` tinyint(3) NOT NULL AFTER `game_types`;
ALTER TABLE cockatrice_games MODIFY `time_started` datetime default NULL AFTER `player_count`;
ALTER TABLE cockatrice_games MODIFY `time_finished` datetime default NULL AFTER `time_started`;

ALTER TABLE cockatrice_games_players MODIFY `id_game` int(7) unsigned zerofill NOT NULL FIRST;
ALTER TABLE cockatrice_games_players MODIFY `player_name` varchar(35) NOT NULL AFTER `id_game`;

ALTER TABLE cockatrice_replays MODIFY `id` int(7) NOT NULL AUTO_INCREMENT FIRST;
ALTER TABLE cockatrice_replays MODIFY `id_game` int(7) unsigned NULL AFTER `id`;
ALTER TABLE cockatrice_replays MODIFY `duration` int(7) NOT NULL AFTER `id_game`;
ALTER TABLE cockatrice_replays MODIFY `replay` mediumblob NOT NULL AFTER `duration`;

ALTER TABLE cockatrice_replays_access MODIFY `id_game` int(7) unsigned NOT NULL FIRST;
ALTER TABLE cockatrice_replays_access MODIFY `id_player` int(7) unsigned NOT NULL AFTER `id_game`;
ALTER TABLE cockatrice_replays_access MODIFY `replay_name` varchar(255) NOT NULL AFTER `id_player`;
ALTER TABLE cockatrice_replays_access MODIFY `do_not_hide` tinyint(1) NOT NULL AFTER `replay_name`;

ALTER TABLE cockatrice_servers MODIFY `id` mediumint(8) unsigned NOT NULL FIRST;
ALTER TABLE cockatrice_servers MODIFY `ssl_cert` text NOT NULL AFTER `id`;
ALTER TABLE cockatrice_servers MODIFY `hostname` varchar(255) NOT NULL AFTER `ssl_cert`;
ALTER TABLE cockatrice_servers MODIFY `address` varchar(255) NOT NULL AFTER `hostname`;
ALTER TABLE cockatrice_servers MODIFY `game_port` mediumint(8) unsigned NOT NULL AFTER `address`;
ALTER TABLE cockatrice_servers MODIFY `control_port` mediumint(9) NOT NULL AFTER `game_port`;

ALTER TABLE cockatrice_uptime MODIFY `id_server` tinyint(3) NOT NULL FIRST;
ALTER TABLE cockatrice_uptime MODIFY `timest` datetime NOT NULL DEFAULT '0000-00-00 00:00:00' AFTER `id_server`;
ALTER TABLE cockatrice_uptime MODIFY `uptime` int(11) NOT NULL AFTER `timest`;
ALTER TABLE cockatrice_uptime MODIFY `users_count` int(11) NOT NULL AFTER `uptime`;
ALTER TABLE cockatrice_uptime MODIFY `mods_count` int(11) NOT NULL DEFAULT 0 AFTER `users_count`;
ALTER TABLE cockatrice_uptime MODIFY `mods_list` TEXT AFTER `mods_count`;
ALTER TABLE cockatrice_uptime MODIFY `games_count` int(11) NOT NULL AFTER `mods_list`;
ALTER TABLE cockatrice_uptime MODIFY `rx_bytes` int(11) NOT NULL AFTER `games_count`;
ALTER TABLE cockatrice_uptime MODIFY `tx_bytes` int(11) NOT NULL AFTER `rx_bytes`;

ALTER TABLE cockatrice_servermessages MODIFY `id_server` tinyint(3) not null default 1 FIRST;
ALTER TABLE cockatrice_servermessages MODIFY `timest` datetime NOT NULL default '0000-00-00 00:00:00' AFTER `id_server`;
ALTER TABLE cockatrice_servermessages MODIFY `message` text AFTER `timest`;

ALTER TABLE cockatrice_sesions MODIFY `id` int(9) NOT NULL AUTO_INCREMENT FIRST;
ALTER TABLE cockatrice_sesions MODIFY `user_name` varchar(35) NOT NULL AFTER `id`;
ALTER TABLE cockatrice_sesions MODIFY `id_server` tinyint(3) NOT NULL AFTER `user_name`;
ALTER TABLE cockatrice_sesions MODIFY `ip_address` varchar(45) NOT NULL AFTER `id_server`;
ALTER TABLE cockatrice_sesions MODIFY `start_time` datetime NOT NULL AFTER `ip_address`;
ALTER TABLE cockatrice_sesions MODIFY `end_time` datetime DEFAULT NULL AFTER `start_time`;
ALTER TABLE cockatrice_sesions MODIFY `clientid` varchar(15) NOT NULL AFTER `end_time`;
ALTER TABLE cockatrice_sesions MODIFY `connection_type` ENUM('tcp', 'websocket') AFTER `clientid`;

ALTER TABLE cockatrice_bans MODIFY `user_name` varchar(35) NOT NULL FIRST;
ALTER TABLE cockatrice_bans MODIFY `ip_address` varchar(45) NOT NULL AFTER `user_name`;
ALTER TABLE cockatrice_bans MODIFY `id_admin` int(7) unsigned zerofill NOT NULL AFTER `ip_address`;
ALTER TABLE cockatrice_bans MODIFY `time_from` datetime NOT NULL AFTER `id_admin`;
ALTER TABLE cockatrice_bans MODIFY `minutes` int(6) NOT NULL AFTER `time_from`;
ALTER TABLE cockatrice_bans MODIFY `reason` text NOT NULL AFTER `minutes`;
ALTER TABLE cockatrice_bans MODIFY `visible_reason` text NOT NULL AFTER `reason`;
ALTER TABLE cockatrice_bans MODIFY `clientid` varchar(15) NOT NULL AFTER `visible_reason`;

ALTER TABLE cockatrice_warnings MODIFY `user_id` int(7) unsigned NOT NULL FIRST;
ALTER TABLE cockatrice_warnings MODIFY `user_name` varchar(35) NOT NULL AFTER `user_id`;
ALTER TABLE cockatrice_warnings MODIFY `mod_name` varchar(35) NOT NULL AFTER `user_name`;
ALTER TABLE cockatrice_warnings MODIFY `reason` text NOT NULL AFTER `mod_name`;
ALTER TABLE cockatrice_warnings MODIFY `time_of` datetime NOT NULL AFTER `reason`;
ALTER TABLE cockatrice_warnings MODIFY `clientid` varchar(15) NOT NULL AFTER `time_of`;

ALTER TABLE cockatrice_log MODIFY `log_time` datetime NOT NULL FIRST;
ALTER TABLE cockatrice_log MODIFY `sender_id` int(7) unsigned NULL AFTER `log_time`;
ALTER TABLE cockatrice_log MODIFY `sender_name` varchar(35) NOT NULL AFTER `sender_id`;
ALTER TABLE cockatrice_log MODIFY `sender_ip` varchar(45) NOT NULL AFTER `sender_name`;
ALTER TABLE cockatrice_log MODIFY `log_message` text NOT NULL AFTER `sender_ip`;
ALTER TABLE cockatrice_log MODIFY `target_type` ENUM('room', 'game', 'chat') AFTER `log_message`;
ALTER TABLE cockatrice_log MODIFY `target_id` int(7) NULL AFTER `target_type`;
ALTER TABLE cockatrice_log MODIFY `target_name` varchar(50) NOT NULL AFTER `target_id`;

-- cockatrice_activation_emails has only 1 column so we skip it

ALTER TABLE cockatrice_user_analytics MODIFY `id` int(7) unsigned zerofill NOT NULL FIRST;
ALTER TABLE cockatrice_user_analytics MODIFY `client_ver` varchar(35) NOT NULL AFTER `id`;
ALTER TABLE cockatrice_user_analytics MODIFY `last_login` datetime NOT NULL AFTER `client_ver`;
ALTER TABLE cockatrice_user_analytics MODIFY `notes` varchar(255) NOT NULL AFTER `last_login`;

ALTER TABLE cockatrice_donations MODIFY `id` int(11) unsigned NOT NULL AUTO_INCREMENT FIRST;
ALTER TABLE cockatrice_donations MODIFY `username` varchar(35) DEFAULT NULL AFTER `id`;
ALTER TABLE cockatrice_donations MODIFY `email` varchar(255) DEFAULT NULL AFTER `username`;
ALTER TABLE cockatrice_donations MODIFY `payment_pre_fee` double DEFAULT NULL AFTER `email`;
ALTER TABLE cockatrice_donations MODIFY `payment_post_fee` double DEFAULT NULL AFTER `payment_pre_fee`;
ALTER TABLE cockatrice_donations MODIFY `term_length` int(11) DEFAULT NULL AFTER `payment_post_fee`;
ALTER TABLE cockatrice_donations MODIFY `date` varchar(255) DEFAULT NULL AFTER `term_length`;
ALTER TABLE cockatrice_donations MODIFY `pp_type` varchar(255) DEFAULT NULL AFTER `date`;

ALTER TABLE cockatrice_forgot_password MODIFY `id` int(7) unsigned zerofill NOT NULL auto_increment FIRST;
ALTER TABLE cockatrice_forgot_password MODIFY `name` varchar(35) NOT NULL AFTER `id`;
ALTER TABLE cockatrice_forgot_password MODIFY `requestDate` datetime NOT NULL default '0000-00-00 00:00:00' AFTER `name`;
ALTER TABLE cockatrice_forgot_password MODIFY `emailed` tinyint(1) NOT NULL default 0 AFTER `requestDate`;

ALTER TABLE cockatrice_audit MODIFY `id` int(7) unsigned zerofill NOT NULL auto_increment FIRST;
ALTER TABLE cockatrice_audit MODIFY `id_server` tinyint(3) NOT NULL AFTER `id`;
ALTER TABLE cockatrice_audit MODIFY `name` varchar(35) NOT NULL AFTER `id_server`;
ALTER TABLE cockatrice_audit MODIFY `ip_address` varchar(45) NOT NULL AFTER `name`;
ALTER TABLE cockatrice_audit MODIFY `clientid` varchar(15) NOT NULL AFTER `ip_address`;
ALTER TABLE cockatrice_audit MODIFY `incidentDate` datetime NOT NULL default '0000-00-00 00:00:00' AFTER `clientid`;
ALTER TABLE cockatrice_audit MODIFY `action` varchar(35) NOT NULL AFTER `incidentDate`;
ALTER TABLE cockatrice_audit MODIFY `results` ENUM('fail', 'success') NOT NULL DEFAULT 'fail' AFTER `action`;
ALTER TABLE cockatrice_audit MODIFY `details` varchar(255) NOT NULL AFTER `results`;

UPDATE cockatrice_schema_version SET version=26 WHERE version=25;
