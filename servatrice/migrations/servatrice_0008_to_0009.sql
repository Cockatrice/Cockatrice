-- Servatrice db migration from version 8 to version 9

alter table cockatrice_rooms add chat_history_size int(4) not null after join_message;
update cockatrice_rooms set chat_history_size = 100;

UPDATE cockatrice_schema_version SET version=9 WHERE version=8;
