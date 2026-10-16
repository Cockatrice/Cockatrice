-- Servatrice db migration from version 37 to version 38

-- Deck collection sync + versioning.
--
-- 1. Stable per-deck identity and change detection for two-way sync:
--    guid links a local file to its server row across renames, hash detects
--    content changes without downloading, last_modified drives newest-wins
--    default and conflict highlighting, deleted implements tombstones so a
--    delete on one device does not resurrect from the other.
-- 2. Limited per-deck version history: every hash-changing upload inserts a
--    row; the server prunes beyond the configured retention (default 20).

ALTER TABLE `cockatrice_decklist_files`
  ADD COLUMN `guid` char(36) NULL AFTER `tags`,
  ADD COLUMN `hash` char(40) NULL AFTER `guid`,
  ADD COLUMN `last_modified` datetime NULL AFTER `hash`,
  ADD COLUMN `deleted` tinyint(1) NOT NULL DEFAULT 0 AFTER `last_modified`;

CREATE TABLE IF NOT EXISTS `cockatrice_deck_versions` (
  `id` int(7) unsigned zerofill NOT NULL auto_increment,
  `deck_id` int(7) unsigned zerofill NOT NULL,
  `version_no` int(7) unsigned NOT NULL,
  `content` text NOT NULL,
  `hash` char(40) NULL,
  `created_at` datetime NOT NULL,
  `created_by_clientid` varchar(15) NULL,
  PRIMARY KEY  (`id`),
  KEY `deck_id` (`deck_id`),
  FOREIGN KEY(`deck_id`) REFERENCES `cockatrice_decklist_files`(`id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 DEFAULT COLLATE utf8mb4_unicode_ci;

UPDATE cockatrice_schema_version SET version=38 WHERE version=37;
