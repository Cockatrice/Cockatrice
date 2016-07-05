-- Servatrice db migration from version 13 to version 14

alter table cockatrice_sessions add `connection_type` ENUM('tcp', 'websocket');
UPDATE cockatrice_sessions SET connection_type = 'tcp';

UPDATE cockatrice_schema_version SET version=14 WHERE version=13;
