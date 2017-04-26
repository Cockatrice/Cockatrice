-- Servatrice db migration from version 18 to version 19

alter table cockatrice_sessions modify column `user_name` varchar(35) NOT NULL;
alter table cockatrice_sessions modify column `ip_address` varchar(255) NOT NULL;

UPDATE cockatrice_schema_version SET version=19 WHERE version=18;
