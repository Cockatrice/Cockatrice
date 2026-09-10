-- Servatrice db migration from version 36 to version 37

-- Deck sharing (temporary share links + permanent public decks).
--
-- This feature was developed behind several intermediate migrations that have
-- never shipped, so they are folded into this single 36 -> 37 migration:
-- temporary share links, permanent public-deck visibility, preview metadata,
-- and per-deck tags.

-- 1. Temporary deck shares: a named bundle of decks that can be fetched by
-- anyone who knows the (unguessable) token, until the share expires.
CREATE TABLE IF NOT EXISTS `cockatrice_deck_share` (
  `id` int(7) unsigned zerofill NOT NULL auto_increment,
  `token` varchar(64) NOT NULL,
  `name` varchar(64) NOT NULL,
  `created_by` int(7) unsigned NULL,
  `created_at` datetime NOT NULL,
  `expires_at` datetime NOT NULL,
  PRIMARY KEY  (`id`),
  UNIQUE KEY `token` (`token`),
  KEY `expires_at` (`expires_at`),
  FOREIGN KEY(`created_by`) REFERENCES `cockatrice_users`(`id`) ON DELETE SET NULL ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 DEFAULT COLLATE utf8mb4_unicode_ci;

-- Individual decks inside a share bundle. Content is materialized at share
-- time so expiring/deleting a share can cascade cleanly. The metadata columns
-- are nullable because Qt's MySQL driver binds empty QStrings as NULL when
-- using prepared statements.
CREATE TABLE IF NOT EXISTS `cockatrice_deck_share_item` (
  `id` int(7) unsigned zerofill NOT NULL auto_increment,
  `share_id` int(7) unsigned zerofill NOT NULL,
  `name` varchar(50) NOT NULL,
  `tags` text NULL,
  `banner_card` varchar(255) NULL,
  `game_format` varchar(50) NULL,
  `color_identity` varchar(5) NULL,
  `content` text NOT NULL,
  `position` int(7) NOT NULL,
  PRIMARY KEY  (`id`),
  KEY `share_id` (`share_id`),
  FOREIGN KEY(`share_id`) REFERENCES `cockatrice_deck_share`(`id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 DEFAULT COLLATE utf8mb4_unicode_ci;

-- 2. Permanent deck sharing: add public visibility flags to the deck storage
-- tables. A deck is visible to other users if it is marked public, or if any
-- ancestor folder is marked public (inherited). Existing decks default to
-- private, so the upgrade does not expose any data.
ALTER TABLE `cockatrice_decklist_files`
  ADD COLUMN `is_public` tinyint(1) NOT NULL DEFAULT 0 AFTER `content`;

ALTER TABLE `cockatrice_decklist_folders`
  ADD COLUMN `is_public` tinyint(1) NOT NULL DEFAULT 0 AFTER `name`;

-- 3. Per-deck preview metadata so clients can render another user's public
-- decks (e.g. in a visual deck storage grid) without downloading each deck
-- list. The metadata is computed by the uploading client; decks uploaded
-- before this migration have empty values until they are re-uploaded.
ALTER TABLE `cockatrice_decklist_files`
  ADD COLUMN `banner_card_name` varchar(255) NULL AFTER `content`,
  ADD COLUMN `banner_card_provider` varchar(32) NULL AFTER `banner_card_name`,
  ADD COLUMN `color_identity` varchar(5) NULL AFTER `banner_card_provider`;

-- 4. Per-deck tags for public decks. The uploading client sends a
-- comma-separated tag string (matching the deck's own tags), so another user's
-- public decks can render and filter by tag without downloading each deck list.
-- Decks uploaded before this migration have NULL tags until they are
-- re-uploaded.
ALTER TABLE `cockatrice_decklist_files`
  ADD COLUMN `tags` text NULL AFTER `color_identity`;

UPDATE cockatrice_schema_version SET version=37 WHERE version=36;
