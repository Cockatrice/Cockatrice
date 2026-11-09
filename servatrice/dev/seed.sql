-- Development fixtures for a local Servatrice database.
--
-- Accounts, rooms, channels, moderation data, and a deck are all keyed by
-- stable names or fixed ids, so this file can be applied repeatedly while
-- iterating. Passwords are the scrypt verifier for the literal password
-- "password"; regenerate them with PasswordHasher if that ever changes.
--
-- This file is loaded automatically by docker-compose.dev.yml on a fresh
-- volume, and can be re-applied with scripts/dev-db.sh seed.

-- Accounts -----------------------------------------------------------------
-- admin bitfield: 1 = admin (implies moderator), 2 = moderator,
-- 4 = judge, 8 = developer.
INSERT INTO cockatrice_users
    (admin, name, realname, password_sha512, email, country, avatar_bmp,
     registrationDate, active, clientid, adminnotes, privlevel,
     privlevelStartDate, privlevelEndDate, passwordLastChangedDate)
VALUES
    (15, 'devadmin', 'Dev Admin', '$scrypt$32768$8$1$Y29ja2F0cmljZS1kZXYhIQ==$L1m5vF9LgraerY880GvTPNdNpuYzmE4/My4wa7q6wTa3+QuoYeTpYERPhnH9I2WXBNLUrrvdYaCrOZ6EuI8ofQ==', 'devadmin@example.invalid', 'US', '', '2026-01-01 00:00:00', 1, '', 'Seeded development administrator.', 'NONE', '2026-01-01 00:00:00', '2026-01-01 00:00:00', '2026-01-01 00:00:00'),
    (2, 'devmod', 'Dev Moderator', '$scrypt$32768$8$1$Y29ja2F0cmljZS1kZXYhIQ==$L1m5vF9LgraerY880GvTPNdNpuYzmE4/My4wa7q6wTa3+QuoYeTpYERPhnH9I2WXBNLUrrvdYaCrOZ6EuI8ofQ==', 'devmod@example.invalid', 'US', '', '2026-01-01 00:00:00', 1, '', 'Seeded development moderator.', 'NONE', '2026-01-01 00:00:00', '2026-01-01 00:00:00', '2026-01-01 00:00:00'),
    (4, 'devjudge', 'Dev Judge', '$scrypt$32768$8$1$Y29ja2F0cmljZS1kZXYhIQ==$L1m5vF9LgraerY880GvTPNdNpuYzmE4/My4wa7q6wTa3+QuoYeTpYERPhnH9I2WXBNLUrrvdYaCrOZ6EuI8ofQ==', 'devjudge@example.invalid', 'US', '', '2026-01-01 00:00:00', 1, '', 'Seeded development judge.', 'NONE', '2026-01-01 00:00:00', '2026-01-01 00:00:00', '2026-01-01 00:00:00'),
    (8, 'devdev', 'Dev Developer', '$scrypt$32768$8$1$Y29ja2F0cmljZS1kZXYhIQ==$L1m5vF9LgraerY880GvTPNdNpuYzmE4/My4wa7q6wTa3+QuoYeTpYERPhnH9I2WXBNLUrrvdYaCrOZ6EuI8ofQ==', 'devdev@example.invalid', 'US', '', '2026-01-01 00:00:00', 1, '', 'Seeded development developer.', 'NONE', '2026-01-01 00:00:00', '2026-01-01 00:00:00', '2026-01-01 00:00:00'),
    (0, 'devplayer', 'Dev Player', '$scrypt$32768$8$1$Y29ja2F0cmljZS1kZXYhIQ==$L1m5vF9LgraerY880GvTPNdNpuYzmE4/My4wa7q6wTa3+QuoYeTpYERPhnH9I2WXBNLUrrvdYaCrOZ6EuI8ofQ==', 'devplayer@example.invalid', 'US', '', '2026-01-01 00:00:00', 1, '', 'Seeded development player.', 'NONE', '2026-01-01 00:00:00', '2026-01-01 00:00:00', '2026-01-01 00:00:00'),
    (0, 'devplayer2', 'Dev Player Two', '$scrypt$32768$8$1$Y29ja2F0cmljZS1kZXYhIQ==$L1m5vF9LgraerY880GvTPNdNpuYzmE4/My4wa7q6wTa3+QuoYeTpYERPhnH9I2WXBNLUrrvdYaCrOZ6EuI8ofQ==', 'devplayer2@example.invalid', 'US', '', '2026-01-01 00:00:00', 1, '', 'Seeded development player.', 'NONE', '2026-01-01 00:00:00', '2026-01-01 00:00:00', '2026-01-01 00:00:00')
ON DUPLICATE KEY UPDATE
    admin = VALUES(admin),
    realname = VALUES(realname),
    password_sha512 = VALUES(password_sha512),
    email = VALUES(email),
    active = VALUES(active),
    adminnotes = VALUES(adminnotes);

SET @devadmin_id = (SELECT id FROM cockatrice_users WHERE name = 'devadmin');
SET @devmod_id = (SELECT id FROM cockatrice_users WHERE name = 'devmod');
SET @devjudge_id = (SELECT id FROM cockatrice_users WHERE name = 'devjudge');
SET @devdev_id = (SELECT id FROM cockatrice_users WHERE name = 'devdev');
SET @devplayer_id = (SELECT id FROM cockatrice_users WHERE name = 'devplayer');
SET @devplayer2_id = (SELECT id FROM cockatrice_users WHERE name = 'devplayer2');

-- Rooms and channels -------------------------------------------------------
INSERT INTO cockatrice_rooms
    (id, name, descr, permissionlevel, privlevel, auto_join, join_message, chat_history_size, id_server)
VALUES
    (1, 'General', 'Play anything here.', 'NONE', 'NONE', 1, '', 100, 1),
    (2, 'Testing', 'Scratch room for server and client work.', 'NONE', 'NONE', 1, '', 100, 1)
ON DUPLICATE KEY UPDATE
    name = VALUES(name),
    descr = VALUES(descr),
    permissionlevel = VALUES(permissionlevel),
    privlevel = VALUES(privlevel),
    auto_join = VALUES(auto_join),
    join_message = VALUES(join_message),
    chat_history_size = VALUES(chat_history_size),
    id_server = VALUES(id_server);

DELETE FROM cockatrice_rooms_gametypes WHERE id_room IN (1, 2) AND id_server = 1;
INSERT INTO cockatrice_rooms_gametypes (id_room, name, id_server) VALUES
    (1, 'Standard', 1),
    (2, 'Standard', 1),
    (2, 'Commander', 1);

DELETE FROM cockatrice_rooms_channels WHERE id_room IN (1, 2) AND id_server = 1;
INSERT INTO cockatrice_rooms_channels
    (id_room, id_server, channel_id, display_name, access_level, write_access_level, banner_message)
VALUES
    (1, 1, 'general', 'General', 'public', NULL, ''),
    (1, 1, 'moderators', 'Moderators', 'moderator', NULL, ''),
    (1, 1, 'developer-news', 'Developer news', 'public', 'developer', 'Official announcements from the developers.'),
    (2, 1, 'testing', 'Testing', 'public', NULL, ''),
    (2, 1, 'moderators', 'Moderators', 'moderator', NULL, '');

-- Social lists -------------------------------------------------------------
INSERT IGNORE INTO cockatrice_buddylist (id_user1, id_user2) VALUES (@devplayer_id, @devmod_id);
INSERT IGNORE INTO cockatrice_ignorelist (id_user1, id_user2) VALUES (@devplayer_id, @devplayer2_id);

-- Moderation fixtures ------------------------------------------------------
INSERT INTO cockatrice_reports
    (id, reporter_id, reporter_name, reported_user_id, reported_user_name, game_id, room_id,
     category, description, chat_log, created_at, resolution_time, status, assigned_to,
     resolved_by, resolution_note, notified)
VALUES
    (9001, @devplayer_id, 'devplayer', @devplayer2_id, 'devplayer2', NULL, 1,
     'cheating', 'Seeded open report: suspected cheat in a test game.', NULL,
     '2026-01-02 12:00:00', NULL, 'open', NULL, NULL, NULL, 0),
    (9002, @devplayer2_id, 'devplayer2', @devplayer_id, 'devplayer', NULL, 1,
     'spam', 'Seeded resolved report: repeated messages in General.', NULL,
     '2026-01-02 13:00:00', '2026-01-02 15:00:00', 'resolved', @devmod_id,
     @devmod_id, 'Resolved during development seeding.', 0)
ON DUPLICATE KEY UPDATE
    reporter_id = VALUES(reporter_id),
    reporter_name = VALUES(reporter_name),
    reported_user_id = VALUES(reported_user_id),
    reported_user_name = VALUES(reported_user_name),
    room_id = VALUES(room_id),
    category = VALUES(category),
    description = VALUES(description),
    created_at = VALUES(created_at),
    resolution_time = VALUES(resolution_time),
    status = VALUES(status),
    assigned_to = VALUES(assigned_to),
    resolved_by = VALUES(resolved_by),
    resolution_note = VALUES(resolution_note);

INSERT INTO cockatrice_report_comments
    (id, report_id, author_name, author_id, comment_text, created_at, is_moderator, notified)
VALUES
    (9001, 9001, 'devmod', @devmod_id, 'Looking into this now.', '2026-01-02 12:30:00', 1, 0)
ON DUPLICATE KEY UPDATE
    comment_text = VALUES(comment_text),
    created_at = VALUES(created_at),
    is_moderator = VALUES(is_moderator);

INSERT INTO cockatrice_warnings (user_id, user_name, mod_name, reason, time_of, clientid)
VALUES (@devplayer2_id, 'devplayer2', 'devmod', 'Seeded warning: spamming the General channel.', '2026-01-03 12:00:00', '')
ON DUPLICATE KEY UPDATE
    mod_name = VALUES(mod_name),
    reason = VALUES(reason);

-- A permanent ban on a documentation-only address: it exercises the ban list
-- without locking out 127.0.0.1 or any seeded account.
INSERT INTO cockatrice_bans
    (user_name, ip_address, id_admin, time_from, minutes, reason, visible_reason, clientid)
VALUES
    ('griefer', '192.0.2.10', @devmod_id, '2026-01-04 12:00:00', 0,
     'Seeded ban: abusive behaviour.', 'Abusive behaviour.', '')
ON DUPLICATE KEY UPDATE
    id_admin = VALUES(id_admin),
    minutes = VALUES(minutes),
    reason = VALUES(reason),
    visible_reason = VALUES(visible_reason);

DELETE FROM cockatrice_log WHERE log_time = '2026-01-05 12:00:00';
INSERT INTO cockatrice_log
    (log_time, sender_id, sender_name, sender_ip, log_message, target_type, target_id, target_name, channel)
VALUES
    ('2026-01-05 12:00:00', @devplayer_id, 'devplayer', '127.0.0.1', 'Anyone up for a test game?', 'room_channel', 1, 'General', 'general'),
    ('2026-01-05 12:00:00', @devplayer2_id, 'devplayer2', '127.0.0.1', 'Sure, create it in Testing.', 'room_channel', 1, 'General', 'general');

-- Deck storage -------------------------------------------------------------
INSERT INTO cockatrice_decklist_folders (id, id_parent, id_user, name, is_public)
VALUES (9001, 0, @devplayer_id, 'Dev decks', 0)
ON DUPLICATE KEY UPDATE
    id_user = VALUES(id_user),
    name = VALUES(name),
    is_public = VALUES(is_public);

INSERT INTO cockatrice_decklist_files
    (id, id_folder, id_user, name, upload_time, content, is_public)
VALUES
    (9001, 9001, @devplayer_id, 'Dev burn deck', '2026-01-06 12:00:00',
     '<?xml version="1.0" encoding="UTF-8"?>\n<cockatrice_deck version="1">\n    <deckname>Dev burn deck</deckname>\n    <comments>Seeded development deck.</comments>\n    <zone name="main">\n        <card number="4" name="Lightning Bolt"/>\n        <card number="4" name="Mountain"/>\n    </zone>\n</cockatrice_deck>\n',
     0)
ON DUPLICATE KEY UPDATE
    id_folder = VALUES(id_folder),
    id_user = VALUES(id_user),
    name = VALUES(name),
    upload_time = VALUES(upload_time),
    content = VALUES(content),
    is_public = VALUES(is_public);
