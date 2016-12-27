-- Servatrice db migration from version 19 to version 20

alter table cockatrice_users add column privlevelStartDate datetime NOT NULL;
alter table cockatrice_users add column privlevelEndDate datetime NOT NULL;
update cockatrice_users set privlevelStartDate = NOW() where privlevel != 'NONE';
update cockatrice_users set privlevelEndDate = DATE_ADD(NOW() , INTERVAL 30 DAY) where privlevel != 'NONE';

UPDATE cockatrice_schema_version SET version=20 WHERE version=19;
