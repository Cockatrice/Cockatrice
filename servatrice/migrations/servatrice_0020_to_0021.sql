

CREATE TABLE IF NOT EXISTS `cockatrice_password_reset` (
  `id` int(7) unsigned zerofill NOT NULL auto_increment,
  `user_name` varchar(35) NOT NULL,
  `requestDate` datetime NOT NULL default '0000-00-00 00:00:00',
  PRIMARY KEY  (`id`),
  KEY `user_name` (`user_name`)
) ENGINE=INNODB  DEFAULT CHARSET=utf8;

UPDATE cockatrice_schema_version SET version=21 WHERE version=20;