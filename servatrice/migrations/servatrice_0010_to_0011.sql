-- Servatrice db migration from version 10 to version 11

alter table cockatrice_warnings change id user_id int(7) unsigned NOT NULL;
alter table cockatrice_warnings drop primary key, add primary key(user_id,time_of);

UPDATE cockatrice_schema_version SET version=11 WHERE version=10;
