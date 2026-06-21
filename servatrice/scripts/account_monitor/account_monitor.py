#!/usr/bin/env python3
"""Post a Discord message when a new Servatrice account is registered.

Accounts get an auto-increment `id`, so "what is new since last time" is simply
`id > last_seen_id`. The monitor stores that single high-water-mark id in a
small state file. Each run it posts every account above the mark (oldest
first), then advances the mark. This means no duplicate posts, nothing missed
across downtime, and a state file that never grows (it holds one number).

On the very first run (no state file yet) it records the current maximum id as
the baseline and posts nothing, so existing users are not dumped into the
channel. From then on only newly-registered accounts are posted.

Intended to be run on a schedule (cron). Pass a servatrice-style ini with
--config (or CONFIG_FILE) for the database credentials and webhook; see
README.md.
"""

import argparse
import configparser
import json
import logging
import os
import re
import sys
import time
import urllib.error
import urllib.request

import pymysql

log = logging.getLogger("account_monitor")

# Columns we use. `id` drives the high-water-mark; `name` is the login/username,
# `realname` is the optional display name, `registrationDate` is when the account
# row was created.
NEW_ACCOUNTS_QUERY = (
    "SELECT id, name, realname, email, registrationDate "
    "FROM `{prefix}_users` WHERE id > %s ORDER BY id ASC"
)

EMBED_COLOR = 0x5865F2  # discord blurple
DISCORD_MAX_EMBED_FIELD = 1024
POST_DELAY_SECONDS = 1.0  # gap between webhook posts to stay under rate limits
MAX_RATELIMIT_RETRIES = 5


def _clean_ini_value(value):
    """Undo Qt QSettings ini encoding of a value.

    Qt apps (including Servatrice) write ini values that contain special
    characters wrapped in double quotes and backslash-escaped, e.g. a webhook
    URL stored as "https\\://...". configparser returns that text literally, so
    strip the wrapping quotes and remove the backslash escapes. This is applied
    to the webhook URL only, where it is safe (URLs contain no quotes or
    backslashes); DB values are left untouched so passwords are never altered.
    """
    value = value.strip()
    if len(value) >= 2 and value[0] == value[-1] and value[0] in "\"'":
        value = value[1:-1]
    return re.sub(r"\\(.)", r"\1", value)


def load_config_file(path):
    """Read DB and Discord settings from a servatrice-style ini.

    Pulls the [database] section (hostname/database/user/password/prefix/port)
    and the Discord webhook from [discord] new_user_activation_webhook. Returns
    a dict using this module's internal config keys; only keys actually present
    (and non-empty) in the file are returned, so missing values fall back to
    defaults or the environment.
    """
    # interpolation=None so a '%' in a password is not treated as a token.
    parser = configparser.ConfigParser(interpolation=None)
    if not parser.read(path):
        log.error("Config file not found or unreadable: %s", path)
        sys.exit(2)

    result = {}
    if parser.has_section("database"):
        db = parser["database"]
        db_mapping = {
            "hostname": "db_host",
            "database": "db_name",
            "user": "db_user",
            "password": "db_password",
            "prefix": "db_prefix",
            "port": "db_port",  # not in stock servatrice.ini, but honored if present
        }
        result.update({cfg_key: db[ini_key] for ini_key, cfg_key in db_mapping.items() if db.get(ini_key)})

    if parser.has_section("discord") and parser["discord"].get("new_user_activation_webhook"):
        result["webhook_url"] = _clean_ini_value(parser["discord"]["new_user_activation_webhook"])

    return result


def get_config(config_path=None, require_db=True):
    """Build configuration from defaults, an optional ini file, then env vars.

    Precedence, highest first: environment variables, the ini file, built-in
    defaults. Database credentials and the Discord webhook may come from either
    the ini file or the environment; the state file is environment-only.
    """
    cfg = {
        "db_host": "localhost",
        "db_port": 3306,
        "db_name": "servatrice",
        "db_user": None,
        "db_password": None,
        "db_prefix": "cockatrice",
        "db_ssl": False,
        "db_ssl_ca": None,
        "webhook_url": None,
        "state_file": os.path.join(os.path.dirname(os.path.abspath(__file__)), "state.json"),
    }

    if config_path:
        cfg.update(load_config_file(config_path))

    env_map = {
        "DB_HOST": "db_host",
        "DB_PORT": "db_port",
        "DB_NAME": "db_name",
        "DB_USER": "db_user",
        "DB_PASSWORD": "db_password",
        "DB_TABLE_PREFIX": "db_prefix",
        "DB_SSL_CA": "db_ssl_ca",
        "DISCORD_WEBHOOK_URL": "webhook_url",
        "STATE_FILE": "state_file",
    }
    for env_key, cfg_key in env_map.items():
        if os.environ.get(env_key):
            cfg[cfg_key] = os.environ[env_key]
    if os.environ.get("DB_SSL"):
        cfg["db_ssl"] = os.environ["DB_SSL"].lower() in ("1", "true", "yes")

    cfg["db_port"] = int(cfg["db_port"])

    required = {"webhook_url": "DISCORD_WEBHOOK_URL or [discord] new_user_activation_webhook"}
    if require_db:
        required["db_user"] = "DB_USER or [database] user"
        required["db_password"] = "DB_PASSWORD or [database] password"
    missing = [label for key, label in required.items() if not cfg[key]]
    if missing:
        log.error("Missing required configuration: %s", "; ".join(missing))
        sys.exit(2)

    if cfg["webhook_url"] and not cfg["webhook_url"].lower().startswith(("http://", "https://")):
        log.error("Webhook URL does not look like an http(s) URL: %r", cfg["webhook_url"])
        sys.exit(2)
    return cfg


def connect(cfg):
    """Open a read-only connection to the Servatrice database."""
    ssl = None
    if cfg["db_ssl"]:
        ssl = {"ca": cfg["db_ssl_ca"]} if cfg["db_ssl_ca"] else {}
    return pymysql.connect(
        host=cfg["db_host"],
        port=cfg["db_port"],
        user=cfg["db_user"],
        password=cfg["db_password"],
        database=cfg["db_name"],
        charset="utf8mb4",
        cursorclass=pymysql.cursors.DictCursor,
        connect_timeout=15,
        read_timeout=30,
        ssl=ssl,
    )


def fetch_max_id(conn, prefix):
    """Return the highest account id currently in the table, or 0 if empty."""
    with conn.cursor() as cur:
        cur.execute("SELECT MAX(id) AS max_id FROM `{prefix}_users`".format(prefix=prefix))
        row = cur.fetchone()
    return int(row["max_id"]) if row and row["max_id"] is not None else 0


def fetch_new_accounts(conn, prefix, last_id):
    """Return detail rows for accounts with id > last_id, oldest first."""
    with conn.cursor() as cur:
        cur.execute(NEW_ACCOUNTS_QUERY.format(prefix=prefix), (last_id,))
        return cur.fetchall()


def load_state(path):
    """Load the high-water-mark id. Returns (last_id, is_first_run)."""
    if not os.path.exists(path):
        return 0, True
    with open(path, "r", encoding="utf-8") as fh:
        data = json.load(fh)
    return int(data.get("last_id", 0)), False


def save_state(path, last_id):
    """Atomically persist the high-water-mark id."""
    directory = os.path.dirname(path)
    if directory:
        os.makedirs(directory, exist_ok=True)
    tmp = path + ".tmp"
    with open(tmp, "w", encoding="utf-8") as fh:
        json.dump({"version": 2, "last_id": int(last_id)}, fh)
    os.replace(tmp, path)


def build_embed(row):
    """Build a Discord embed dict for one newly-registered account."""
    fields = [
        {"name": "Username", "value": str(row["name"]) or "(none)", "inline": False},
    ]
    realname = (row.get("realname") or "").strip()
    if realname:
        fields.append({"name": "Real name", "value": realname[:DISCORD_MAX_EMBED_FIELD], "inline": False})
    fields.append({"name": "Email", "value": str(row.get("email") or "(none)"), "inline": False})
    reg = row.get("registrationDate")
    fields.append({"name": "Reg time", "value": str(reg) if reg is not None else "(unknown)", "inline": False})
    return {
        "title": "New account registered",
        "color": EMBED_COLOR,
        "fields": fields,
    }


def post_embed(webhook_url, embed):
    """POST a single embed to the Discord webhook, honoring 429 rate limits."""
    payload = json.dumps({"embeds": [embed]}).encode("utf-8")
    for attempt in range(MAX_RATELIMIT_RETRIES):
        req = urllib.request.Request(
            webhook_url,
            data=payload,
            headers={"Content-Type": "application/json", "User-Agent": "servatrice-account-monitor/1.0"},
            method="POST",
        )
        try:
            with urllib.request.urlopen(req, timeout=30) as resp:
                if resp.status in (200, 204):
                    return True
                log.warning("Unexpected Discord status %s", resp.status)
                return False
        except urllib.error.HTTPError as err:
            if err.code == 429:
                retry_after = _retry_after_seconds(err)
                log.warning("Rate limited by Discord; sleeping %.2fs", retry_after)
                time.sleep(retry_after)
                continue
            log.error("Discord webhook HTTP %s: %s", err.code, err.read().decode("utf-8", "replace")[:500])
            return False
        except urllib.error.URLError as err:
            log.error("Discord webhook connection error: %s", err)
            return False
    log.error("Gave up posting after %d rate-limit retries", MAX_RATELIMIT_RETRIES)
    return False


def _retry_after_seconds(err):
    """Extract the retry delay (seconds) from a Discord 429 response."""
    header = err.headers.get("Retry-After")
    if header:
        try:
            return float(header)
        except ValueError:
            pass
    try:
        body = json.loads(err.read().decode("utf-8", "replace"))
        return float(body.get("retry_after", 1.0))
    except (ValueError, json.JSONDecodeError):
        return 1.0


def main():
    parser = argparse.ArgumentParser(description="Post new Servatrice account registrations to Discord.")
    parser.add_argument(
        "--config", "-c",
        help="Path to a servatrice-style ini; reads DB settings from its [database] "
             "section (hostname/database/user/password/prefix) and the webhook from "
             "[discord] new_user_activation_webhook. Defaults to the CONFIG_FILE env "
             "var if set.",
    )
    parser.add_argument("--dry-run", action="store_true", help="Log what would be posted; do not post or write state.")
    parser.add_argument("--test-webhook", action="store_true", help="Send a single test message to the webhook and exit.")
    parser.add_argument("--verbose", action="store_true", help="Enable debug logging.")
    args = parser.parse_args()

    logging.basicConfig(
        level=logging.DEBUG if args.verbose else logging.INFO,
        format="%(asctime)s %(levelname)s %(message)s",
    )

    config_path = args.config or os.environ.get("CONFIG_FILE")

    if args.test_webhook:
        cfg = get_config(config_path, require_db=False)
        ok = post_embed(
            cfg["webhook_url"],
            {"title": "Account monitor test", "color": EMBED_COLOR,
             "description": "If you can see this, the webhook is configured correctly."},
        )
        sys.exit(0 if ok else 1)

    cfg = get_config(config_path)

    try:
        conn = connect(cfg)
    except pymysql.MySQLError as err:
        log.error("Database connection failed: %s", err)
        sys.exit(1)

    try:
        last_id, first_run = load_state(cfg["state_file"])

        if first_run:
            baseline = fetch_max_id(conn, cfg["db_prefix"])
            log.info("First run: seeding high-water-mark at id=%d; posting nothing.", baseline)
            if not args.dry_run:
                save_state(cfg["state_file"], baseline)
            return

        rows = fetch_new_accounts(conn, cfg["db_prefix"], last_id)
        if not rows:
            log.info("No new accounts since id=%d.", last_id)
            return

        log.info("Found %d new account(s) since id=%d.", len(rows), last_id)

        # Post oldest first. Advance the mark only past accounts we successfully
        # posted; on the first failure, stop so nothing after it is posted out of
        # order or skipped. The failed account (and the rest) retry next run.
        for row in rows:
            if args.dry_run:
                log.info("[dry-run] would post: id=%s name=%s email=%s reg=%s",
                         row["id"], row["name"], row.get("email"), row.get("registrationDate"))
                continue
            if not post_embed(cfg["webhook_url"], build_embed(row)):
                log.error("Failed to post account id=%s; stopping. Will retry from here next run.", row["id"])
                break
            last_id = row["id"]
            log.info("Posted account id=%s (%s)", row["id"], row["name"])
            time.sleep(POST_DELAY_SECONDS)

        if not args.dry_run:
            save_state(cfg["state_file"], last_id)
    finally:
        conn.close()


if __name__ == "__main__":
    main()
