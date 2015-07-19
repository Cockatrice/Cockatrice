-- Servatrice db migration from version 1 to version 2

-- FIX #153
CREATE TABLE IF NOT EXISTS `cockatrice_schema_version` (
  `version` int(7) unsigned NOT NULL,
  PRIMARY KEY  (`version`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

INSERT INTO cockatrice_schema_version VALUES(2);

alter table cockatrice_users add clientid varchar(15) not null;
