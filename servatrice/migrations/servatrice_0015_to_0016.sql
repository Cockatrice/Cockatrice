-- Servatrice db migration from version 15 to version 16

drop table cockatrice_news;

UPDATE cockatrice_schema_version SET version=16 WHERE version=15;
