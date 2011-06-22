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
  `user` varchar(35) NOT NULL,
  `name` varchar(50) NOT NULL,
  `upload_time` datetime NOT NULL,
  `content` text NOT NULL,
  PRIMARY KEY  (`id`),
  KEY `FolderPlusUser` (`id_folder`,`user`)
) ENGINE=MyISAM  DEFAULT CHARSET=latin1 AUTO_INCREMENT=550 ;

-- --------------------------------------------------------

--
-- Tabellenstruktur für Tabelle `decklist_folders`
--

CREATE TABLE IF NOT EXISTS `cockatrice_decklist_folders` (
  `id` int(7) unsigned zerofill NOT NULL auto_increment,
  `id_parent` int(7) unsigned zerofill NOT NULL,
  `user` varchar(35) NOT NULL,
  `name` varchar(30) NOT NULL,
  PRIMARY KEY  (`id`),
  KEY `ParentPlusUser` (`id_parent`,`user`)
) ENGINE=MyISAM  DEFAULT CHARSET=latin1 AUTO_INCREMENT=80 ;

-- --------------------------------------------------------

--
-- Tabellenstruktur für Tabelle `games`
--

CREATE TABLE IF NOT EXISTS `cockatrice_games` (
  `id` int(7) unsigned zerofill NOT NULL,
  `descr` varchar(50) default NULL,
  `password` tinyint(1) default NULL,
  `time_started` datetime default NULL,
  `time_finished` datetime default NULL,
  PRIMARY KEY  (`id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- --------------------------------------------------------

--
-- Tabellenstruktur für Tabelle `games_players`
--

CREATE TABLE IF NOT EXISTS `cockatrice_games_players` (
  `id_game` int(7) unsigned zerofill NOT NULL,
  `player` varchar(35) default NULL,
  KEY `id_game` (`id_game`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

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
) ENGINE=MyISAM  DEFAULT CHARSET=latin1 AUTO_INCREMENT=17 ;

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
  `password` varchar(255) NOT NULL,
  `email` varchar(255) NOT NULL,
  `country` char(2) NOT NULL,
  `avatar_bmp` blob NOT NULL,
  `registrationDate` datetime NOT NULL,
  `active` tinyint(1) NOT NULL,
  `banned` tinyint(1) NOT NULL,
  `token` char(32) NOT NULL,
  PRIMARY KEY  (`id`),
  UNIQUE KEY `name` (`name`)
) ENGINE=MyISAM  DEFAULT CHARSET=latin1 AUTO_INCREMENT=915 ;

CREATE TABLE `cockatrice_uptime` (
  `id_server` tinyint(3) NOT NULL,
  `timest` datetime NOT NULL DEFAULT '0000-00-00 00:00:00',
  `uptime` int(11) DEFAULT NULL,
  `users_count` int(11) DEFAULT NULL,
  `games_count` int(11) DEFAULT NULL,
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

