-- Servatrice db migration from version 38 to version 39

-- Per-channel read/write split and banner messages.
--
-- write_access_level gates sending: NULL (the default) means writing requires
-- the same level as reading (access_level), preserving the previous
-- single-level behavior. Set it higher than access_level for read-mostly
-- channels (e.g. a developer news feed readable by everyone but writable only
-- by developers).
--
-- banner_message is a static notice shown at the top of the channel,
-- analogous to the room join message on Main. Empty = none.

ALTER TABLE `cockatrice_rooms_channels`
  MODIFY `access_level` ENUM('public','registered','judge','moderator','admin','developer') NOT NULL DEFAULT 'public',
  ADD COLUMN `write_access_level` ENUM('public','registered','judge','moderator','admin','developer') NULL DEFAULT NULL,
  ADD COLUMN `banner_message` varchar(255) NOT NULL DEFAULT '';

UPDATE cockatrice_schema_version SET version=39 WHERE version=38;
