-- Servatrice db migration from version 16 to version 18

alter table cockatrice_users add privlevel enum("NONE","VIP","DONATOR") NOT NULL;

UPDATE cockatrice_schema_version SET version=18 WHERE version=17;
