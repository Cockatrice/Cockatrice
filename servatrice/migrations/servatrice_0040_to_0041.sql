-- Ban records have an append identity so multiple actions in one second
-- remain ordered, including an unban immediately after an account rename.
-- Registered-account bans follow their immutable account ID. Preserve the
-- recorded name and all IP/client-ID fields. Unregistered names remain NULL.
ALTER TABLE cockatrice_bans
  ADD COLUMN id bigint unsigned NOT NULL AUTO_INCREMENT FIRST,
  ADD COLUMN user_id int(7) unsigned NULL AFTER id,
  DROP PRIMARY KEY,
  ADD PRIMARY KEY (id),
  ADD KEY user_name_time_from (user_name, time_from),
  ADD KEY user_id_time_from (user_id, time_from);

-- Do not link bans older than the current account: that name may have been reused.
UPDATE cockatrice_bans b
INNER JOIN cockatrice_users u ON u.name = b.user_name
SET b.user_id = u.id
WHERE b.user_name <> '' AND b.time_from >= u.registrationDate;

UPDATE cockatrice_schema_version SET version=41 WHERE version=40;
