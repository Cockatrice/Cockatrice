-- Servatrice db migration from version 35 to version 36

CREATE TABLE IF NOT EXISTS `cockatrice_reports` (
 `id`                  int(11) unsigned NOT NULL AUTO_INCREMENT,
 `reporter_id`         int(7) unsigned NULL,
 `reporter_name`       varchar(35) NOT NULL,
 `reported_user_id`    int(7) unsigned NULL,
 `reported_user_name`  varchar(35) NOT NULL,
 `game_id`             int(7) unsigned NULL,
 `room_id`             int(7) unsigned NULL,
 `category`            varchar(255) NOT NULL,
 `description`         text NOT NULL,
 `chat_log`            mediumtext NULL,
 `created_at`          datetime NOT NULL,
 `resolution_time`     datetime NULL,
 `status`              enum('open','assigned','resolved','dismissed') NOT NULL DEFAULT 'open',
 `assigned_to`         int(7) unsigned NULL,
 `resolution_note`     text,
 `notified`            tinyint(1) NOT NULL DEFAULT 0,
 PRIMARY KEY (`id`),
 INDEX `idx_status`    (`status`),
 INDEX `idx_created`   (`created_at`),
 INDEX `idx_reporter_id_created` (`reporter_id`, `created_at`),
 INDEX `idx_reported_user_name` (`reported_user_name`),
 INDEX `idx_status_created` (`status`, `created_at`),
 FOREIGN KEY (`reporter_id`)      REFERENCES `cockatrice_users`(`id`) ON DELETE SET NULL ON UPDATE CASCADE,
 FOREIGN KEY (`reported_user_id`) REFERENCES `cockatrice_users`(`id`) ON DELETE SET NULL ON UPDATE CASCADE,
 FOREIGN KEY (`assigned_to`)      REFERENCES `cockatrice_users`(`id`) ON DELETE SET NULL ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 DEFAULT COLLATE utf8mb4_unicode_ci;

CREATE TABLE IF NOT EXISTS `cockatrice_report_comments` (
 `id`                  int(11) unsigned NOT NULL AUTO_INCREMENT,
 `report_id`           int(11) unsigned NOT NULL,
 `author_name`         varchar(35) NOT NULL,
 `author_id`           int(7) unsigned NULL,
 `comment_text`        text NOT NULL,
 `created_at`          datetime NOT NULL,
 `is_moderator`        tinyint(1) NOT NULL DEFAULT 0,
 `notified`            tinyint(1) NOT NULL DEFAULT 0,
 PRIMARY KEY (`id`),
 INDEX `idx_report_id` (`report_id`),
 INDEX `idx_notified`  (`notified`),
 FOREIGN KEY (`report_id`) REFERENCES `cockatrice_reports`(`id`) ON DELETE CASCADE ON UPDATE CASCADE,
 FOREIGN KEY (`author_id`) REFERENCES `cockatrice_users`(`id`) ON DELETE SET NULL ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 DEFAULT COLLATE utf8mb4_unicode_ci;

UPDATE cockatrice_schema_version SET version=36 WHERE version=35;
