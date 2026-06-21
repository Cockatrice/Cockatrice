# Account registration monitor

Posts a Discord message whenever a new account is registered in Servatrice
(`cockatrice_users`). Each message includes the username, real name (if set),
email, and registration time.

It runs as a periodic read-only query against the production database. It does
not modify the database and does not touch the running Servatrice process.

## How it decides what is "new"

Accounts get an auto-increment `id`, so "new since last time" is just
`id > last_seen_id`. The monitor stores that single high-water-mark id in its
state file. Each run it posts every account above the mark, oldest first, then
advances the mark to the highest id it posted.

Because the mark only moves forward and an id is posted exactly once, there are
no duplicate messages and nothing is missed, even if the monitor is down for a
while. The state file holds a single number, so it never grows.

The first run (when no state file exists yet) records the current maximum id as
the baseline and posts nothing. This prevents the entire existing user base from
being dumped into the channel. Only accounts registered after that baseline are
posted.

If a post to Discord fails, the monitor stops there without advancing the mark
past it, so that account and everything after it are retried on the next run.

## Privacy note

Messages contain personal data (real name and email). Discord stores message
content on their servers, so post only to a private channel that the right
people can see, and treat the webhook URL as a secret. It lives in the config
ini alongside the database password, so keep that file readable only by the user
that runs the monitor.

## Setup

The monitor reads its database credentials and the webhook from a
servatrice-style ini file passed with `--config` (or the `CONFIG_FILE` env var).
You can point it at your existing `servatrice.ini`, or keep a small separate ini
just for the monitor.

### 1. Create a read-only database user

Run as a DB admin. Adjust the host (`'%'` allows any host; restrict it to the
machine running the monitor if you can) and the table prefix if yours is not the
default `cockatrice`.

```sql
CREATE USER 'account_monitor'@'%' IDENTIFIED BY 'a-strong-password';
GRANT SELECT (id, name, realname, email, registrationDate)
  ON servatrice.cockatrice_users TO 'account_monitor'@'%';
FLUSH PRIVILEGES;
```

Using a read-only user is recommended over pointing `--config` at the real
`servatrice.ini`, because Servatrice's own DB account usually has write access
the monitor does not need.

### 2. Create the Discord webhook and add it to the config

In Discord: open the target channel, then Edit Channel -> Integrations ->
Webhooks -> New Webhook. Name it, pick the channel, and copy the webhook URL.

Add a `[discord]` section with the URL to the ini you will pass to `--config`.
If you want the read-only user above, set the `[database]` section to use it. A
small dedicated `monitor.ini` looks like this:

```ini
[database]
hostname=127.0.0.1
database=servatrice
user=account_monitor
password=a-strong-password
prefix=cockatrice

[discord]
new_user_activation_webhook=https://discord.com/api/webhooks/XXXX/YYYY
```

If you would rather use one file, add the `[discord]` section to the real
`servatrice.ini` instead. Servatrice ignores sections it does not use. Note that
Servatrice (a Qt app) rewrites ini values it touches in quoted, backslash-escaped
form, for example `"https\://..."`. The monitor strips that encoding from the
webhook automatically, so either the plain or the escaped form works.

### 3. Install

```bash
cd servatrice/scripts/account_monitor
python3 -m venv venv
./venv/bin/pip install -r requirements.txt
```

### 4. Verify before scheduling

```bash
# Confirm the webhook works (sends one test message to the channel)
./venv/bin/python ./account_monitor.py --config /path/to/monitor.ini --test-webhook

# Confirm DB access and see what it would do, without posting or writing state
./venv/bin/python ./account_monitor.py --config /path/to/monitor.ini --dry-run --verbose
```

The first real run seeds the baseline and posts nothing:

```bash
./venv/bin/python ./account_monitor.py --config /path/to/monitor.ini
```

After that, test it end to end by registering a throwaway account and confirming
a message appears on the next run.

## Run it every 2 minutes with cron

Edit the crontab of the user that owns the script directory (`crontab -e`) and
add one line. This runs the monitor every 2 minutes, using the venv's Python and
your config ini, and appends output to a log:

```cron
*/2 * * * * cd /opt/cockatrice/servatrice/scripts/account_monitor && ./venv/bin/python ./account_monitor.py --config /etc/servatrice/servatrice.ini >> /var/log/account_monitor.log 2>&1
```

Adjust the three paths to your install: the script directory after `cd`, and the
`--config` and log paths. The `*/2` field is what makes it run every 2 minutes;
change it to `*/5` for every 5, and so on.

By default the high-water-mark is stored in `state.json` next to the script, so
the directory must be writable by the cron user. To put it elsewhere, set
`STATE_FILE`:

```cron
*/2 * * * * STATE_FILE=/var/lib/account_monitor/state.json cd /opt/cockatrice/servatrice/scripts/account_monitor && ./venv/bin/python ./account_monitor.py --config /etc/servatrice/servatrice.ini >> /var/log/account_monitor.log 2>&1
```

The interval only controls how often it checks; it is not a lookback window, so
a longer interval never causes missed accounts. The query is cheap: an indexed
range scan on the primary key for `id > last_seen`.

## Options

- `--config PATH` / `-c PATH` — read DB settings from `[database]` and the webhook from `[discord] new_user_activation_webhook` of a servatrice-style ini (falls back to the `CONFIG_FILE` env var).
- `--dry-run` — query and log what would be posted; no Discord posts, no state write.
- `--test-webhook` — send one test message to the webhook and exit (does not need DB credentials).
- `--verbose` — debug logging.

## Configuration reference

Settings come from the `--config` ini, with environment variables available as
overrides if you need them (env takes precedence over the ini).

| Setting | ini (`--config`) | Environment override |
| --- | --- | --- |
| DB host | `[database] hostname` | `DB_HOST` |
| DB port | `[database] port` (optional) | `DB_PORT` |
| DB name | `[database] database` | `DB_NAME` |
| DB user | `[database] user` | `DB_USER` |
| DB password | `[database] password` | `DB_PASSWORD` |
| Table prefix | `[database] prefix` | `DB_TABLE_PREFIX` |
| Webhook URL | `[discord] new_user_activation_webhook` | `DISCORD_WEBHOOK_URL` |
| DB TLS | — | `DB_SSL` / `DB_SSL_CA` |
| State file path | — | `STATE_FILE` (default: `state.json` next to the script) |
