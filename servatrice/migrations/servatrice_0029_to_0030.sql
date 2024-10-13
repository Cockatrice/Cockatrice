-- Servatrice db migration from version 29 to version 30

ALTER TABLE cockatrice_rooms ADD card_limit int(10) unsigned NOT NULL AFTER chat_history_size;

UPDATE cockatrice_schema_version SET version=30 WHERE version=29;
