-- Servatrice db migration from version 12 to version 13

-- WARNING: this is quite a big change, so you really, REALLY should
-- backup your database before attempting to execute this migration.

-- First move all the tables to the InnoDB engine
ALTER TABLE `cockatrice_schema_version` ENGINE=InnoDB;
ALTER TABLE `cockatrice_decklist_files` ENGINE=InnoDB;
ALTER TABLE `cockatrice_decklist_folders` ENGINE=InnoDB;
ALTER TABLE `cockatrice_games` ENGINE=InnoDB;
ALTER TABLE `cockatrice_games_players` ENGINE=InnoDB;
ALTER TABLE `cockatrice_news` ENGINE=InnoDB;
ALTER TABLE `cockatrice_users` ENGINE=InnoDB;
ALTER TABLE `cockatrice_uptime` ENGINE=InnoDB;
ALTER TABLE `cockatrice_servermessages` ENGINE=InnoDB;
ALTER TABLE `cockatrice_ignorelist` ENGINE=InnoDB;
ALTER TABLE `cockatrice_buddylist` ENGINE=InnoDB;
ALTER TABLE `cockatrice_bans` ENGINE=InnoDB;
ALTER TABLE `cockatrice_warnings` ENGINE=InnoDB;
ALTER TABLE `cockatrice_sessions` ENGINE=InnoDB;
ALTER TABLE `cockatrice_servers` ENGINE=InnoDB;
ALTER TABLE `cockatrice_replays` ENGINE=InnoDB;
ALTER TABLE `cockatrice_replays_access` ENGINE=InnoDB;
ALTER TABLE `cockatrice_rooms` ENGINE=InnoDB;
ALTER TABLE `cockatrice_rooms_gametypes` ENGINE=InnoDB;
ALTER TABLE `cockatrice_log` ENGINE=InnoDB;
ALTER TABLE `cockatrice_activation_emails` ENGINE=InnoDB;
ALTER TABLE `cockatrice_user_analytics` ENGINE=InnoDB;

-- Fix the replays tables not using unsigned values for id_game and id_player
ALTER TABLE `cockatrice_replays` MODIFY COLUMN `id_game` int(7) unsigned NULL;
ALTER TABLE `cockatrice_replays_access` MODIFY COLUMN `id_game` int(7) unsigned NOT NULL;
ALTER TABLE `cockatrice_replays_access` MODIFY COLUMN `id_player` int(7) unsigned NOT NULL;

-- Now add some foreign keys between tables. Since there was no constaint before,
-- we need to ensure no leftover record (eg. a user deck without an user) exists
-- before adding the FK, or the query will fail.

DELETE FROM `cockatrice_decklist_files` WHERE `id_user` NOT IN (SELECT `id` FROM `cockatrice_users`);
ALTER TABLE `cockatrice_decklist_files` ADD FOREIGN KEY(`id_user`) REFERENCES `cockatrice_users`(`id`) ON DELETE CASCADE ON UPDATE CASCADE;

DELETE FROM `cockatrice_decklist_folders` WHERE `id_user` NOT IN (SELECT `id` FROM `cockatrice_users`);
ALTER TABLE `cockatrice_decklist_folders` ADD FOREIGN KEY(`id_user`) REFERENCES `cockatrice_users`(`id`) ON DELETE CASCADE ON UPDATE CASCADE;

DELETE FROM `cockatrice_news` WHERE `id_user` NOT IN (SELECT `id` FROM `cockatrice_users`);
ALTER TABLE `cockatrice_news` ADD FOREIGN KEY(`id_user`) REFERENCES `cockatrice_users`(`id`) ON DELETE CASCADE ON UPDATE CASCADE;

DELETE FROM `cockatrice_ignorelist` WHERE `id_user1` NOT IN (SELECT `id` FROM `cockatrice_users`);
DELETE FROM `cockatrice_ignorelist` WHERE `id_user2` NOT IN (SELECT `id` FROM `cockatrice_users`);
ALTER TABLE `cockatrice_ignorelist` ADD FOREIGN KEY(`id_user1`) REFERENCES `cockatrice_users`(`id`)  ON DELETE CASCADE ON UPDATE CASCADE;
ALTER TABLE `cockatrice_ignorelist` ADD FOREIGN KEY(`id_user2`) REFERENCES `cockatrice_users`(`id`)  ON DELETE CASCADE ON UPDATE CASCADE;

DELETE FROM `cockatrice_buddylist` WHERE `id_user1` NOT IN (SELECT `id` FROM `cockatrice_users`);
DELETE FROM `cockatrice_buddylist` WHERE `id_user2` NOT IN (SELECT `id` FROM `cockatrice_users`);
ALTER TABLE `cockatrice_buddylist` ADD FOREIGN KEY(`id_user1`) REFERENCES `cockatrice_users`(`id`)  ON DELETE CASCADE ON UPDATE CASCADE;
ALTER TABLE `cockatrice_buddylist` ADD FOREIGN KEY(`id_user2`) REFERENCES `cockatrice_users`(`id`)  ON DELETE CASCADE ON UPDATE CASCADE;

DELETE FROM `cockatrice_user_analytics` WHERE `id` NOT IN (SELECT `id` FROM `cockatrice_users`);
ALTER TABLE `cockatrice_user_analytics` ADD FOREIGN KEY(`id`) REFERENCES `cockatrice_users`(`id`) ON DELETE CASCADE ON UPDATE CASCADE;

DELETE FROM `cockatrice_log` WHERE `sender_id` NOT IN (SELECT `id` FROM `cockatrice_users`);
ALTER TABLE `cockatrice_log` ADD FOREIGN KEY(`sender_id`) REFERENCES `cockatrice_users`(`id`) ON DELETE CASCADE ON UPDATE CASCADE;

DELETE FROM `cockatrice_activation_emails` WHERE `name` NOT IN (SELECT `name` FROM `cockatrice_users`);
ALTER TABLE `cockatrice_activation_emails` ADD FOREIGN KEY(`name`) REFERENCES `cockatrice_users`(`name`) ON DELETE CASCADE ON UPDATE CASCADE;

DELETE FROM `cockatrice_rooms_gametypes` WHERE `id_room` NOT IN (SELECT `id` FROM `cockatrice_rooms`);
ALTER TABLE `cockatrice_rooms_gametypes` ADD FOREIGN KEY(`id_room`) REFERENCES `cockatrice_rooms`(`id`) ON DELETE CASCADE ON UPDATE CASCADE;

DELETE FROM `cockatrice_games_players` WHERE `id_game` NOT IN (SELECT `id` FROM `cockatrice_games`);
ALTER TABLE `cockatrice_games_players` ADD FOREIGN KEY(`id_game`) REFERENCES `cockatrice_games`(`id`) ON DELETE CASCADE ON UPDATE CASCADE;

DELETE FROM `cockatrice_replays` WHERE `id_game` NOT IN (SELECT `id` FROM `cockatrice_games`);
ALTER TABLE `cockatrice_replays` ADD FOREIGN KEY(`id_game`) REFERENCES `cockatrice_games`(`id`) ON DELETE CASCADE ON UPDATE CASCADE;

DELETE FROM `cockatrice_replays_access` WHERE `id_game` NOT IN (SELECT `id` FROM `cockatrice_games`);
ALTER TABLE `cockatrice_replays_access` ADD FOREIGN KEY(`id_game`) REFERENCES `cockatrice_games`(`id`) ON DELETE CASCADE ON UPDATE CASCADE;

DELETE FROM `cockatrice_replays_access` WHERE `id_player` NOT IN (SELECT `id` FROM `cockatrice_users`);
ALTER TABLE `cockatrice_replays_access` ADD FOREIGN KEY(`id_player`) REFERENCES `cockatrice_users`(`id`) ON DELETE CASCADE ON UPDATE CASCADE;

DELETE FROM `cockatrice_bans` WHERE `id_admin` NOT IN (SELECT `id` FROM `cockatrice_users`);
ALTER TABLE `cockatrice_bans` ADD FOREIGN KEY(`id_admin`) REFERENCES `cockatrice_users`(`id`) ON DELETE CASCADE ON UPDATE CASCADE;

-- Last: update schema version
UPDATE cockatrice_schema_version SET version=13 WHERE version=12;
