ALTER TABLE `cockatrice_users` MODIFY `password_sha512` char(255) NOT NULL, ALGORITHM=INSTANT;

UPDATE cockatrice_schema_version SET version=36 WHERE version=35;
