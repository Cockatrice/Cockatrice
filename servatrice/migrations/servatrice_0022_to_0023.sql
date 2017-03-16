-- Servatrice db migration from version 22 to version 23

alter table cockatrice_rooms modify column permissionlevel enum('NONE','REGISTERED','MODERATOR','ADMINISTRATOR');
alter table cockatrice_rooms add column privlevel enum('NONE','PRIVILEGED','VIP','DONATOR') NOT NULL;

UPDATE cockatrice_schema_version SET version=23 WHERE version=22;
