-- Servatrice db migration from version 36 to version 37

-- The column must hold "$scrypt$<n>$<r>$<p>$<salt>$<verifier>" (up to ~255 chars) and arbitrary
-- legacy base64 hashes, so it grows beyond the old 120-char size. varchar(255) is used as the
-- column type is promotion-safe and avoids the row-format change ALGORITHM=INSTANT cannot do.
ALTER TABLE `cockatrice_users` MODIFY `password_sha512` varchar(255) NOT NULL;

UPDATE cockatrice_schema_version SET version=37 WHERE version=36;