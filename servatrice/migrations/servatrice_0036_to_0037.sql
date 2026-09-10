-- Servatrice db migration from version 36 to version 37

-- The column must hold "$scrypt$<n>$<r>$<p>$<salt>$<verifier>" (up to ~255 chars) and arbitrary
-- legacy base64 hashes, so it grows beyond the old 120-char size. varchar(255) is used because
-- widening a CHAR requires a table rebuild, which ALGORITHM=INSTANT cannot perform — dropping the
-- clause lets the server pick a suitable algorithm (and any row-format change is avoided anyway).
ALTER TABLE `cockatrice_users` MODIFY `password_sha512` varchar(255) NOT NULL;

UPDATE cockatrice_schema_version SET version=37 WHERE version=36;
