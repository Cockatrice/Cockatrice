ALTER TABLE `cockatrice_users` ADD COLUMN `card_art_params` TEXT DEFAULT NULL, ALGORITHM=INSTANT;

CREATE TABLE IF NOT EXISTS `cockatrice_card_art_name_rules` (
    `id` int(11) unsigned NOT NULL AUTO_INCREMENT,
    `card_name` varchar(255) NOT NULL,
    `mode` enum('ALLOW','DENY') NOT NULL,
    `reason` varchar(255) DEFAULT NULL,
    `created_by` int(7) unsigned DEFAULT NULL,
    `created_at` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP,
    PRIMARY KEY (`id`),
    UNIQUE KEY `uniq_card_name` (`card_name`),
    KEY `idx_mode` (`mode`),
    FOREIGN KEY (`created_by`) REFERENCES `cockatrice_users`(`id`)
    ON DELETE SET NULL
    ON UPDATE CASCADE
    ) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE utf8mb4_unicode_ci;

UPDATE cockatrice_schema_version SET version=35 WHERE version=34;
