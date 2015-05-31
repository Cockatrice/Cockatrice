-- Servatrice db migration from version 0 to version 1

-- FIX #153
CREATE TABLE IF NOT EXISTS `cockatrice_schema_version` (
  `version` int(7) unsigned NOT NULL,
  PRIMARY KEY  (`version`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

INSERT INTO cockatrice_schema_version VALUES(1);

-- FIX #1119
ALTER TABLE `cockatrice_rooms_gametypes` DROP PRIMARY KEY;
ALTER TABLE `cockatrice_rooms_gametypes` ADD KEY (`id_room`);
