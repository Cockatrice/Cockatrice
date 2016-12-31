-- Servatrice db migration from version 19 to version 20

alter table cockatrice_users add column privlevelStartDate datetime NOT NULL;
alter table cockatrice_users add column privlevelEndDate datetime NOT NULL;
update cockatrice_users set privlevelStartDate = NOW() where privlevel != 'NONE';
update cockatrice_users set privlevelEndDate = DATE_ADD(NOW() , INTERVAL 30 DAY) where privlevel != 'NONE';

CREATE TABLE IF NOT EXISTS `cockatrice_donations` (
  `id` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `username` varchar(255) DEFAULT NULL,
  `email` varchar(255) DEFAULT NULL,
  `payment_pre_fee` double DEFAULT NULL,
  `payment_post_fee` double DEFAULT NULL,
  `term_length` int(11) DEFAULT NULL,
  `date` varchar(255) DEFAULT NULL,
  `pp_type` varchar(255) DEFAULT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

UPDATE cockatrice_schema_version SET version=20 WHERE version=19;
