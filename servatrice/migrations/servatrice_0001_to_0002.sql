-- Servatrice db migration from version 1 to version 2

-- FIX #1281
CREATE TABLE IF NOT EXISTS `cockatrice_activation_emails` (
  `name` varchar(35) NOT NULL
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

UPDATE cockatrice_schema_version SET version=2 WHERE version=1;
