-- Servatrice db migration from version 20 to version 21

alter table cockatrice_activation_emails add column type enum("REG","FORGOTPASS") NOT NULL;

CREATE TABLE IF NOT EXISTS `cockatrice_audit` (
  `id` int(7) unsigned zerofill NOT NULL auto_increment,
  `type` varchar(255) NOT NULL,
  `user_name` varchar(35) NOT NULL,
  `user_email` varchar(255) NOT NULL,
  `ip_address` varchar(255) NOT NULL,
  `incidentDate` datetime NOT NULL default '0000-00-00 00:00:00',
  `result` tinyint(1) NOT NULL,
  `details` varchar(255) NOT NULL,
  PRIMARY KEY  (`id`),
  KEY `user_name` (`user_name`),
  KEY `user_email` (`user_email`)
) ENGINE=INNODB  DEFAULT CHARSET=utf8;

UPDATE cockatrice_schema_version SET version=21 WHERE version=20;
