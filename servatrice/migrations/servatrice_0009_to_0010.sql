-- Servatrice db migration from version 9 to version 10

CREATE TABLE IF NOT EXISTS `cockatrice_warnings` (
  `id` int(7) unsigned NOT NULL,
  `user_name` varchar(255) NOT NULL,
  `mod_name` varchar(255) NOT NULL,
  `reason` text NOT NULL,
  `time_of` datetime NOT NULL,
  `clientid` varchar(15) NOT NULL,
  PRIMARY KEY (`user_name`,`time_of`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

UPDATE cockatrice_schema_version SET version=10 WHERE version=9;
