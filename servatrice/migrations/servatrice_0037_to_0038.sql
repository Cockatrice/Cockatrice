-- Servatrice db migration from version 37 to version 38

-- Room chat channels.
--
-- A room now offers a fixed catalog of named message streams in addition to the
-- implicit Main channel. Public channels are broadcast to every room member;
-- restricted channels are only delivered to (and sendable by) members whose
-- privilege level meets the channel's access level.

-- 1. Catalog table, mirroring the config-driven channels of no-DB servers.
CREATE TABLE IF NOT EXISTS `cockatrice_rooms_channels` (
  `id_room` int(7) unsigned NOT NULL,
  `id_server` tinyint(3) NOT NULL DEFAULT 1,
  `channel_id` varchar(50) NOT NULL,
  `display_name` varchar(50) NOT NULL,
  `access_level` ENUM('public','registered','judge','moderator','admin','developer') NOT NULL DEFAULT 'public',
  PRIMARY KEY (`id_room`, `id_server`, `channel_id`),
  FOREIGN KEY(`id_room`) REFERENCES `cockatrice_rooms`(`id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 DEFAULT COLLATE utf8mb4_unicode_ci;

-- 2. Audit log: room_channel target type plus a channel column so moderators
-- can tell which stream a logged message belongs to.
ALTER TABLE `cockatrice_log`
  MODIFY `target_type` ENUM('room', 'game', 'chat', 'room_channel') NOT NULL;

ALTER TABLE `cockatrice_log`
  ADD COLUMN `channel` varchar(50) NULL AFTER `target_name`;

UPDATE cockatrice_schema_version SET version=38 WHERE version=37;