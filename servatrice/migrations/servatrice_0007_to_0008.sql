-- Servatrice db migration from version 7 to version 8

CREATE TABLE IF NOT EXISTS `cockatrice_user_analytics` (
  `id` int(7) unsigned zerofill NOT NULL,
  `client_ver` varchar(35) NOT NULL,
  `last_login` datetime NOT NULL,
  `notes` varchar(255) NOT NULL,
  PRIMARY KEY  (`id`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

INSERT INTO `cockatrice_user_analytics` (id, last_login) SELECT id, last_login FROM `cockatrice_users` WHERE last_login != '';

ALTER TABLE `cockatrice_users`
  DROP COLUMN last_login;

UPDATE cockatrice_schema_version SET version=8 WHERE version=7;
