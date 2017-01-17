-- Servatrice db migration from version 20 to version 21

alter table cockatrice_activation_emails add column type enum("REG","FORGOTPASS") NOT NULL;

UPDATE cockatrice_schema_version SET version=21 WHERE version=20;
