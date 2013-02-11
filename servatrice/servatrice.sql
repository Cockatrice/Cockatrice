-- phpMyAdmin SQL Dump
-- version 2.11.8.1deb1ubuntu0.2
-- http://www.phpmyadmin.net
--
-- Host: localhost
-- Erstellungszeit: 11. Oktober 2010 um 23:57
-- Server Version: 5.0.67
-- PHP-Version: 5.2.6-2ubuntu4.6

SET SQL_MODE="NO_AUTO_VALUE_ON_ZERO";


/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8 */;

--
-- Datenbank: `servatrice`
--

-- --------------------------------------------------------

--
-- Tabellenstruktur für Tabelle `decklist_files`
--

CREATE TABLE IF NOT EXISTS `cockatrice_decklist_files` (
  `id` int(7) unsigned zerofill NOT NULL auto_increment,
  `id_folder` int(7) unsigned zerofill NOT NULL,
  `id_user` int(7) unsigned NULL,
  `name` varchar(50) NOT NULL,
  `upload_time` datetime NOT NULL,
  `content` text NOT NULL,
  PRIMARY KEY  (`id`),
  KEY `FolderPlusUser` (`id_folder`,`id_user`)
) ENGINE=MyISAM  DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Tabellenstruktur für Tabelle `decklist_folders`
--

CREATE TABLE IF NOT EXISTS `cockatrice_decklist_folders` (
  `id` int(7) unsigned zerofill NOT NULL auto_increment,
  `id_parent` int(7) unsigned zerofill NOT NULL,
  `id_user` int(7) unsigned NULL,
  `name` varchar(30) NOT NULL,
  PRIMARY KEY  (`id`),
  KEY `ParentPlusUser` (`id_parent`,`id_user`)
) ENGINE=MyISAM  DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Tabellenstruktur für Tabelle `games`
--

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
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Tabellenstruktur für Tabelle `games_players`
--

CREATE TABLE IF NOT EXISTS `cockatrice_games_players` (
  `id_game` int(7) unsigned zerofill NOT NULL,
  `player_name` varchar(255) NOT NULL,
  KEY `id_game` (`id_game`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Tabellenstruktur für Tabelle `news`
--

CREATE TABLE IF NOT EXISTS `cockatrice_news` (
  `id` int(7) unsigned zerofill NOT NULL auto_increment,
  `id_user` int(7) unsigned zerofill NOT NULL,
  `news_date` datetime NOT NULL,
  `subject` varchar(255) NOT NULL,
  `content` text NOT NULL,
  PRIMARY KEY  (`id`)
) ENGINE=MyISAM  DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Tabellenstruktur für Tabelle `users`
--

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
  `token` binary(16) NOT NULL,
  PRIMARY KEY  (`id`),
  UNIQUE KEY `name` (`name`),
  KEY `token` (`token`),
  KEY `email` (`email`)
) ENGINE=MyISAM  DEFAULT CHARSET=utf8;

CREATE TABLE `cockatrice_uptime` (
  `id_server` tinyint(3) NOT NULL,
  `timest` datetime NOT NULL DEFAULT '0000-00-00 00:00:00',
  `uptime` int(11) NOT NULL,
  `users_count` int(11) NOT NULL,
  `games_count` int(11) NOT NULL,
  `rx_bytes` int(11) NOT NULL,
  `tx_bytes` int(11) NOT NULL,
  PRIMARY KEY (`timest`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

CREATE TABLE `cockatrice_servermessages` (
  `id_server` tinyint(3) not null default 0,
  `timest` datetime NOT NULL default '0000-00-00 00:00:00',
  `message` text,
  PRIMARY KEY  (`timest`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

CREATE TABLE `cockatrice_ignorelist` (
  `id_user1` int(7) unsigned NOT NULL,
  `id_user2` int(7) unsigned NOT NULL,
  UNIQUE KEY `key` (`id_user1`, `id_user2`),
  KEY `id_user1` (`id_user1`),
  KEY `id_user2` (`id_user2`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

CREATE TABLE `cockatrice_buddylist` (
  `id_user1` int(7) unsigned NOT NULL,
  `id_user2` int(7) unsigned NOT NULL,
  UNIQUE KEY `key` (`id_user1`, `id_user2`),
  KEY `id_user1` (`id_user1`),
  KEY `id_user2` (`id_user2`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

CREATE TABLE `cockatrice_bans` (
 `user_name` varchar(255) NOT NULL,
 `ip_address` varchar(255) NOT NULL,
 `id_admin` int(7) unsigned zerofill NOT NULL,
 `time_from` datetime NOT NULL,
 `minutes` int(6) NOT NULL,
 `reason` text NOT NULL,
 `visible_reason` text NOT NULL,
  PRIMARY KEY (`user_name`,`time_from`),
  KEY `time_from` (`time_from`,`ip_address`),
  KEY `ip_address` (`ip_address`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

CREATE TABLE `cockatrice_sessions` (
  `id` int(9) NOT NULL AUTO_INCREMENT,
  `user_name` varchar(255) COLLATE utf8_unicode_ci NOT NULL,
  `id_server` tinyint(3) NOT NULL,
  `ip_address` char(15) COLLATE utf8_unicode_ci NOT NULL,
  `start_time` datetime NOT NULL,
  `end_time` datetime DEFAULT NULL,
  PRIMARY KEY (`id`),
  KEY `username` (`user_name`)
) ENGINE=MyISAM  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

CREATE TABLE `cockatrice_servers` (
  `id` mediumint(8) unsigned NOT NULL,
  `ssl_cert` text COLLATE utf8_unicode_ci NOT NULL,
  `hostname` varchar(255) COLLATE utf8_unicode_ci NOT NULL,
  `address` varchar(255) COLLATE utf8_unicode_ci NOT NULL,
  `game_port` mediumint(8) unsigned NOT NULL,
  `control_port` mediumint(9) NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

CREATE TABLE `cockatrice_replays` (
  `id` int(7) NOT NULL AUTO_INCREMENT,
  `id_game` int(7) NOT NULL,
  `duration` int(7) NOT NULL,
  `replay` mediumblob NOT NULL,
  PRIMARY KEY (`id`),
  KEY `id_game` (`id_game`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

CREATE TABLE `cockatrice_replays_access` (
  `id_game` int(7) NOT NULL,
  `id_player` int(7) NOT NULL,
  `replay_name` varchar(255) COLLATE utf8_unicode_ci NOT NULL,
  `do_not_hide` tinyint(1) NOT NULL,
  KEY `id_player` (`id_player`),
  KEY `id_game` (`id_game`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

