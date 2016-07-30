-- Schema file for servatrice database.

-- This schema file is using the default table prefix "cockatrice",
-- to match the "prefix=cockatrice" default setting in servatrice.ini

SET SQL_MODE="NO_AUTO_VALUE_ON_ZERO";


/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8 */;

-- Every time the database schema changes, the schema version number
-- must be incremented. Also remember to update the corresponding
-- number in servatrice/src/servatrice_database_interface.h

CREATE TABLE IF NOT EXISTS `cockatrice_schema_version` (
  `version` int(7) unsigned NOT NULL,
  PRIMARY KEY  (`version`)
) ENGINE=INNODB DEFAULT CHARSET=utf8;

INSERT INTO cockatrice_schema_version VALUES(17);

-- users and user data tables
CREATE TABLE IF NOT EXISTS `cockatrice_users` (
  `id` int(7) unsigned zerofill NOT NULL auto_increment,
  `admin` tinyint(1) NOT NULL,
  `name` varchar(35) NOT NULL,
  `realname` varchar(255) NOT NULL,
  `gender` char(1) NOT NULL,
  `password_sha512` char(120) NOT NULL,
  `email` varchar(255) NOT NULL,
  `country` char(2) NOT NULL,
  `avatar_bmp` blob NOT NULL,
  `registrationDate` datetime NOT NULL,
  `active` tinyint(1) NOT NULL,
  `token` binary(16),
  `clientid` varchar(15) NOT NULL,
  PRIMARY KEY  (`id`),
  UNIQUE KEY `name` (`name`),
  KEY `token` (`token`),
  KEY `email` (`email`)
) ENGINE=INNODB  DEFAULT CHARSET=utf8;

CREATE TABLE IF NOT EXISTS `cockatrice_decklist_files` (
  `id` int(7) unsigned zerofill NOT NULL auto_increment,
  `id_folder` int(7) unsigned zerofill NOT NULL,
  `id_user` int(7) unsigned NULL,
  `name` varchar(50) NOT NULL,
  `upload_time` datetime NOT NULL,
  `content` text NOT NULL,
  PRIMARY KEY  (`id`),
  KEY `FolderPlusUser` (`id_folder`,`id_user`),
  FOREIGN KEY(`id_user`) REFERENCES `cockatrice_users`(`id`)  ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=INNODB  DEFAULT CHARSET=utf8;

CREATE TABLE IF NOT EXISTS `cockatrice_decklist_folders` (
  `id` int(7) unsigned zerofill NOT NULL auto_increment,
  `id_parent` int(7) unsigned zerofill NOT NULL,
  `id_user` int(7) unsigned NULL,
  `name` varchar(30) NOT NULL,
  PRIMARY KEY  (`id`),
  KEY `ParentPlusUser` (`id_parent`,`id_user`),
  FOREIGN KEY(`id_user`) REFERENCES `cockatrice_users`(`id`)  ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=INNODB  DEFAULT CHARSET=utf8;

CREATE TABLE IF NOT EXISTS `cockatrice_ignorelist` (
  `id_user1` int(7) unsigned NOT NULL,
  `id_user2` int(7) unsigned NOT NULL,
  UNIQUE KEY `key` (`id_user1`, `id_user2`),
  FOREIGN KEY(`id_user1`) REFERENCES `cockatrice_users`(`id`)  ON DELETE CASCADE ON UPDATE CASCADE,
  FOREIGN KEY(`id_user2`) REFERENCES `cockatrice_users`(`id`)  ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=INNODB DEFAULT CHARSET=utf8;

CREATE TABLE IF NOT EXISTS `cockatrice_buddylist` (
  `id_user1` int(7) unsigned NOT NULL,
  `id_user2` int(7) unsigned NOT NULL,
  UNIQUE KEY `key` (`id_user1`, `id_user2`),
  FOREIGN KEY(`id_user1`) REFERENCES `cockatrice_users`(`id`)  ON DELETE CASCADE ON UPDATE CASCADE,
  FOREIGN KEY(`id_user2`) REFERENCES `cockatrice_users`(`id`)  ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=INNODB DEFAULT CHARSET=utf8;

-- rooms
CREATE TABLE IF NOT EXISTS `cockatrice_rooms` (
  `id` int(7) unsigned NOT NULL auto_increment,
  `name` varchar(50) NOT NULL,
  `descr` varchar(255) NOT NULL,
  `permissionlevel` varchar(20) NOT NULL,
  `auto_join` tinyint(1) default 0,
  `join_message` varchar(255) NOT NULL,
  `chat_history_size` int(4) NOT NULL,
  `id_server` tinyint(3) NOT NULL DEFAULT 1,
  PRIMARY KEY (`id`)
) ENGINE=INNODB DEFAULT CHARSET=utf8;

CREATE TABLE IF NOT EXISTS `cockatrice_rooms_gametypes` (
  `id_room` int(7) unsigned NOT NULL,
  `name` varchar(50) NOT NULL,
  `id_server` tinyint(3) NOT NULL DEFAULT 1,
  FOREIGN KEY(`id_room`) REFERENCES `cockatrice_rooms`(`id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=INNODB DEFAULT CHARSET=utf8;

-- games
CREATE TABLE IF NOT EXISTS `cockatrice_games` (
  `room_name` varchar(255) NOT NULL,
  `id` int(7) unsigned NOT NULL auto_increment,
  `descr` varchar(50) default NULL,
  `creator_name` varchar(255) NOT NULL,
  `password` tinyint(1) NOT NULL,
  `game_types` varchar(255) NOT NULL,
  `player_count` tinyint(3) NOT NULL,
  `time_started` datetime default NULL,
  `time_finished` datetime default NULL,
  PRIMARY KEY  (`id`)
) ENGINE=INNODB DEFAULT CHARSET=utf8;

CREATE TABLE IF NOT EXISTS `cockatrice_games_players` (
  `id_game` int(7) unsigned zerofill NOT NULL,
  `player_name` varchar(255) NOT NULL,
  FOREIGN KEY(`id_game`) REFERENCES `cockatrice_games`(`id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=INNODB DEFAULT CHARSET=utf8;

-- Note: an empty row with id_game = NULL is created when the game is created,
-- and then updated when the game ends with the full replay data.
CREATE TABLE IF NOT EXISTS `cockatrice_replays` (
  `id` int(7) NOT NULL AUTO_INCREMENT,
  `id_game` int(7) unsigned NULL,
  `duration` int(7) NOT NULL,
  `replay` mediumblob NOT NULL,
  PRIMARY KEY (`id`),
  FOREIGN KEY(`id_game`) REFERENCES `cockatrice_games`(`id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=INNODB DEFAULT CHARSET=utf8;

CREATE TABLE IF NOT EXISTS `cockatrice_replays_access` (
  `id_game` int(7) unsigned NOT NULL,
  `id_player` int(7) unsigned NOT NULL,
  `replay_name` varchar(255) COLLATE utf8_unicode_ci NOT NULL,
  `do_not_hide` tinyint(1) NOT NULL,
  KEY `id_player` (`id_player`),
  FOREIGN KEY(`id_game`) REFERENCES `cockatrice_games`(`id`) ON DELETE CASCADE ON UPDATE CASCADE,
  FOREIGN KEY(`id_player`) REFERENCES `cockatrice_users`(`id`)  ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=INNODB DEFAULT CHARSET=utf8;

-- server administration

-- Note: unused table
CREATE TABLE IF NOT EXISTS `cockatrice_servers` (
  `id` mediumint(8) unsigned NOT NULL,
  `ssl_cert` text COLLATE utf8_unicode_ci NOT NULL,
  `hostname` varchar(255) COLLATE utf8_unicode_ci NOT NULL,
  `address` varchar(255) COLLATE utf8_unicode_ci NOT NULL,
  `game_port` mediumint(8) unsigned NOT NULL,
  `control_port` mediumint(9) NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=INNODB DEFAULT CHARSET=utf8;

CREATE TABLE IF NOT EXISTS `cockatrice_uptime` (
  `id_server` tinyint(3) NOT NULL,
  `timest` datetime NOT NULL DEFAULT '0000-00-00 00:00:00',
  `uptime` int(11) NOT NULL,
  `users_count` int(11) NOT NULL,
  `games_count` int(11) NOT NULL,
  `rx_bytes` int(11) NOT NULL,
  `tx_bytes` int(11) NOT NULL,
  PRIMARY KEY (`timest`)
) ENGINE=INNODB DEFAULT CHARSET=utf8;

CREATE TABLE IF NOT EXISTS `cockatrice_servermessages` (
  `id_server` tinyint(3) not null default 1,
  `timest` datetime NOT NULL default '0000-00-00 00:00:00',
  `message` text,
  PRIMARY KEY  (`timest`)
) ENGINE=INNODB DEFAULT CHARSET=utf8;

CREATE TABLE IF NOT EXISTS `cockatrice_sessions` (
  `id` int(9) NOT NULL AUTO_INCREMENT,
  `user_name` varchar(255) COLLATE utf8_unicode_ci NOT NULL,
  `id_server` tinyint(3) NOT NULL,
  `ip_address` char(15) COLLATE utf8_unicode_ci NOT NULL,
  `start_time` datetime NOT NULL,
  `end_time` datetime DEFAULT NULL,
  `clientid` varchar(15) NOT NULL,
  `connection_type` ENUM('tcp', 'websocket'),
  PRIMARY KEY (`id`),
  KEY `username` (`user_name`)
) ENGINE=INNODB  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

-- server moderation
CREATE TABLE IF NOT EXISTS `cockatrice_bans` (
 `user_name` varchar(255) NOT NULL,
 `ip_address` varchar(255) NOT NULL,
 `id_admin` int(7) unsigned zerofill NOT NULL,
 `time_from` datetime NOT NULL,
 `minutes` int(6) NOT NULL,
 `reason` text NOT NULL,
 `visible_reason` text NOT NULL,
 `clientid` varchar(15) NOT NULL,
  PRIMARY KEY (`user_name`,`time_from`),
  KEY `time_from` (`time_from`,`ip_address`),
  KEY `ip_address` (`ip_address`),
  FOREIGN KEY(`id_admin`) REFERENCES `cockatrice_users`(`id`)  ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=INNODB DEFAULT CHARSET=utf8;

CREATE TABLE IF NOT EXISTS `cockatrice_warnings` (
  `user_id` int(7) unsigned NOT NULL,
  `user_name` varchar(255) NOT NULL,
  `mod_name` varchar(255) NOT NULL,
  `reason` text NOT NULL,
  `time_of` datetime NOT NULL,
  `clientid` varchar(15) NOT NULL,
  PRIMARY KEY (`user_id`,`time_of`)
) ENGINE=INNODB DEFAULT CHARSET=utf8;

CREATE TABLE IF NOT EXISTS `cockatrice_log` (
  `log_time` datetime NOT NULL,
  `sender_id` int(7) unsigned NULL,
  `sender_name` varchar(35) NOT NULL,
  `sender_ip` varchar(255) NOT NULL,
  `log_message` text NOT NULL,
  `target_type` ENUM('room', 'game', 'chat'),
  `target_id` int(7) NULL,
  `target_name` varchar(50) NOT NULL,
  KEY `sender_name` (`sender_name`),
  KEY `sender_ip` (`sender_ip`),
  KEY `target_type` (`target_type`),
  KEY `target_id` (`target_id`),
  KEY `target_name` (`target_name`),
  FOREIGN KEY(`sender_id`) REFERENCES `cockatrice_users`(`id`)  ON DELETE CASCADE ON UPDATE CASCADE
  -- No FK on target_id, it can be zero
) ENGINE=INNODB DEFAULT CHARSET=utf8;

CREATE TABLE IF NOT EXISTS `cockatrice_activation_emails` (
  `name` varchar(35) NOT NULL,
  FOREIGN KEY(`name`) REFERENCES `cockatrice_users`(`name`)  ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=INNODB DEFAULT CHARSET=utf8;

CREATE TABLE IF NOT EXISTS `cockatrice_user_analytics` (
  `id` int(7) unsigned zerofill NOT NULL,
  `client_ver` varchar(35) NOT NULL,
  `last_login` datetime NOT NULL,
  `notes` varchar(255) NOT NULL,
  PRIMARY KEY  (`id`),
  FOREIGN KEY(`id`) REFERENCES `cockatrice_users`(`id`)  ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=INNODB DEFAULT CHARSET=utf8;
