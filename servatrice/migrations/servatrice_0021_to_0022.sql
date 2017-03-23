
CREATE TABLE IF NOT EXISTS `cockatrice_audit` (
  `id` int(7) unsigned zerofill NOT NULL auto_increment,
  `id_server` tinyint(3) NOT NULL,
  `name` varchar(35) NOT NULL,
  `ip_address` varchar(255) NOT NULL,
  `clientid` varchar(15) NOT NULL,
  `incidentDate` datetime NOT NULL default '0000-00-00 00:00:00',
  `action` varchar(35) NOT NULL,
  `results` ENUM('fail', 'success') NOT NULL DEFAULT 'fail',
  `details` varchar(255) NOT NULL,
  PRIMARY KEY  (`id`),
  KEY `user_name` (`name`)
) ENGINE=INNODB  DEFAULT CHARSET=utf8;

UPDATE cockatrice_schema_version SET version=22 WHERE version=21;