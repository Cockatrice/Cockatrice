-- Servatrice db migration from version 37 to version 38

-- The column must hold "$scrypt$<n>$<r>$<p>$<salt>$<verifier>" (up to ~255 chars) and arbitrary
-- legacy base64 hashes, so it grows beyond the old 120-char size. varchar(255) is used because
-- widening a CHAR requires a table rebuild, which ALGORITHM=INSTANT cannot perform — dropping the
-- clause lets the server pick a suitable algorithm (and any row-format change is avoided anyway).
ALTER TABLE `cockatrice_users` MODIFY `password_sha512` varchar(255) NOT NULL;

-- Encrypted downgrade backup: lets pre-challenge clients keep logging in after the
-- account migrates to scrypt, without storing the fast-crackable legacy hash in the
-- clear. Sealed with security/legacy_backup_key and purged after
-- security/legacy_backup_ttl_days without use (see password_legacy_backup_used_at).
ALTER TABLE `cockatrice_users`
  ADD COLUMN `password_legacy_backup` varchar(255) NOT NULL DEFAULT '',
  ADD COLUMN `password_legacy_backup_used_at` datetime DEFAULT NULL;

UPDATE cockatrice_schema_version SET version=38 WHERE version=37;
