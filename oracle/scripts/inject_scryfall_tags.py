#!/usr/bin/env python3
"""Inject Scryfall Tagger tags into an MTGJson AllPrintings file.

MTGJson does not ship Scryfall Tagger tags yet. Until it does, this script
fakes the future `tags` card property so Cockatrice's tag search and tag
filter can be exercised end to end:

  1. It downloads (or reuses) Scryfall's official "Oracle Tags" bulk data.
  2. It builds an `oracle_id -> [tag slug, ...]` map from the taggings.
  3. It walks every card of an AllPrintings file, matches it by
     `identifiers.scryfallOracleId`, and adds a `tags` array of slugs.

The output is a regular MTGJson AllPrintings file that Oracle can import
through its "local file" option. The `tags` array it writes is exactly what
`oracleimporter.cpp` reads once MTGJson starts emitting it, so nothing about
the importer has to change later.

Usage:
    # Patch a downloaded AllPrintings.json.xz into AllPrintings.tagged.json
    ./inject_scryfall_tags.py AllPrintings.json.xz AllPrintings.tagged.json

    # Reuse an already downloaded tags file and write compressed output
    ./inject_scryfall_tags.py AllPrintings.json --oracle-tags oracle-tags.jsonl.gz \
        -o AllPrintings.tagged.json.gz

This is a development stop-gap: it holds the whole input in memory, so expect
a few GB of RAM for a full AllPrintings file.
"""

import argparse
import bz2
import gzip
import json
import lzma
import sys
import urllib.request
import zipfile
from pathlib import Path

USER_AGENT = "CockatriceTagMock/0.1 (+https://github.com/Cockatrice/Cockatrice)"
DEFAULT_API_URL = "https://api.scryfall.com/bulk-data/oracle-tags"
CACHE_FILE = "oracle-tags.jsonl.gz"


def fetch_json(url):
    request = urllib.request.Request(url, headers={"User-Agent": USER_AGENT, "Accept": "application/json"})
    with urllib.request.urlopen(request) as response:
        return json.load(response)


def download(url, destination):
    request = urllib.request.Request(url, headers={"User-Agent": USER_AGENT})
    with urllib.request.urlopen(request) as response, open(destination, "wb") as handle:
        while chunk := response.read(1 << 20):
            handle.write(chunk)


def open_compressed(path, mode):
    """Open a JSON/JSONL file, transparently decompressing by suffix."""
    path = Path(path)
    suffix = path.suffix.lower()
    if suffix == ".gz":
        return gzip.open(path, mode)
    if suffix == ".bz2":
        return bz2.open(path, mode)
    if suffix == ".xz":
        return lzma.open(path, mode)
    if suffix == ".zip":
        archive = zipfile.ZipFile(path)
        name = archive.namelist()[0]
        return archive.open(name, "r" if "r" in mode else "w")
    return open(path, mode)


def load_or_download_tags(args):
    if args.oracle_tags:
        return Path(args.oracle_tags)

    cache = Path(args.cache_dir) / CACHE_FILE
    if cache.exists() and not args.force_download:
        print(f"Using cached Scryfall Oracle Tags: {cache}", file=sys.stderr)
        return cache

    print(f"Fetching {args.api_url}", file=sys.stderr)
    metadata = fetch_json(args.api_url)
    download_uri = metadata.get("jsonl_download_uri") or metadata.get("download_uri")
    if not download_uri:
        raise SystemExit("Scryfall bulk data did not provide a download URI.")

    print(f"Downloading {download_uri}", file=sys.stderr)
    cache.parent.mkdir(parents=True, exist_ok=True)
    download(download_uri, cache)
    return cache


def build_oracle_id_to_tags(tags_path):
    """Return {oracle_id: sorted list of tag slugs} from Scryfall tag objects.

    Scryfall serves the tags as gzipped JSONL (one tag object per line); plain
    JSON arrays are accepted too so a hand-saved file keeps working.
    """
    mapping = {}
    with open_compressed(tags_path, "rt") as handle:
        head = handle.read(1)
        handle.seek(0)
        if head == "[":
            for tag in json.load(handle):
                add_tag_object(mapping, tag)
        else:
            for line in handle:
                line = line.strip()
                if line:
                    add_tag_object(mapping, json.loads(line))

    return {oracle_id: sorted(slugs) for oracle_id, slugs in mapping.items()}


def add_tag_object(mapping, tag):
    slug = tag.get("slug")
    if not slug:
        return
    for tagging in tag.get("taggings") or []:
        oracle_id = tagging.get("oracle_id")
        if oracle_id:
            mapping.setdefault(oracle_id, set()).add(slug)


def inject_tags(all_printings, oracle_id_to_tags):
    sets = all_printings.get("data", {})
    tagged_cards = 0
    total_cards = 0

    for card_set in sets.values():
        for card in card_set.get("cards", []):
            total_cards += 1
            oracle_id = (card.get("identifiers") or {}).get("scryfallOracleId")
            slugs = oracle_id_to_tags.get(oracle_id) if oracle_id else None
            if not slugs:
                continue

            existing = set(card.get("tags") or [])
            existing.update(slugs)
            card["tags"] = sorted(existing)
            tagged_cards += 1

    return total_cards, tagged_cards


def write_json(path, payload):
    with open_compressed(path, "wt") as handle:
        json.dump(payload, handle, separators=(",", ":"))


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("input", help="MTGJson AllPrintings file (.json/.gz/.xz/.zip/.bz2)")
    parser.add_argument("-o", "--output", help="Output file; defaults to <input>.tagged.json")
    parser.add_argument("--oracle-tags", help="Pre-downloaded Scryfall Oracle Tags file (JSONL/JSON, optionally gz)")
    parser.add_argument("--cache-dir", default=".", help="Where to cache the downloaded tags file")
    parser.add_argument("--api-url", default=DEFAULT_API_URL, help="Scryfall bulk-data endpoint for oracle tags")
    parser.add_argument("--force-download", action="store_true", help="Ignore a cached tags file and re-download")
    args = parser.parse_args()

    output = args.output or f"{args.input}.tagged.json"

    tags_path = load_or_download_tags(args)
    oracle_id_to_tags = build_oracle_id_to_tags(tags_path)
    print(f"Loaded tags for {len(oracle_id_to_tags)} oracle ids", file=sys.stderr)

    print(f"Reading {args.input}", file=sys.stderr)
    with open_compressed(args.input, "rt") as handle:
        all_printings = json.load(handle)

    total_cards, tagged_cards = inject_tags(all_printings, oracle_id_to_tags)
    print(f"Tagged {tagged_cards}/{total_cards} cards", file=sys.stderr)

    print(f"Writing {output}", file=sys.stderr)
    write_json(output, all_printings)
    print("Done.", file=sys.stderr)


if __name__ == "__main__":
    main()
