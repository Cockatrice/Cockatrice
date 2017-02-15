

CREATE TABLE IF NOT EXISTS `cockatrice_forgot_password` (
  `id` int(7) unsigned zerofill NOT NULL auto_increment,
  `name` varchar(35) NOT NULL,
  `requestDate` datetime NOT NULL default '0000-00-00 00:00:00',
  `emailed` tinyint(1) NOT NULL default 0,
  PRIMARY KEY  (`id`),
  KEY `user_name` (`name`)
) ENGINE=INNODB  DEFAULT CHARSET=utf8;

UPDATE cockatrice_schema_version SET version=21 WHERE version=20;