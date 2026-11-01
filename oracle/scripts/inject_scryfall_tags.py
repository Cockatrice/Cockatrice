#!/usr/bin/env python3
"""Inject direct Oracle Tags into MTGJson AllPrintings for Oracle's local-file import.

This is the original development-only bridge until MTGJson provides tags itself.
It joins taggings by scryfallOracleId, preserves existing tags, and does not infer
ancestors or import illustration tags. AllPrintings is held in memory.

Usage: inject_scryfall_tags.py AllPrintings.json.xz -o AllPrintings.tagged.json.gz
Use --oracle-tags FILE to reuse downloaded JSON/JSONL without network access.
Existing cache/output files are replaced only after complete successful processing.
"""

import argparse
import bz2
from contextlib import contextmanager, ExitStack
import gzip
import io
import http.client
import math
import json
import lzma
import os
from pathlib import Path
import sys
import tempfile
import urllib.request
import zipfile

USER_AGENT = "CockatriceTagMock/0.1 (+https://github.com/Cockatrice/Cockatrice)"
DEFAULT_API_URL = "https://api.scryfall.com/bulk-data/oracle-tags"
CACHE_FILE = "oracle-tags.jsonl.gz"


def request(url, timeout):
    req = urllib.request.Request(url, headers={
        "User-Agent": USER_AGENT, "Accept": "application/json;q=0.9,*/*;q=0.8"})
    return urllib.request.urlopen(req, timeout=timeout)


def fetch_json(url, timeout=30):
    with request(url, timeout) as response:
        return json.load(response)


def download(url, destination, timeout=30):
    with request(url, timeout) as response, open(destination, "wb") as handle:
        expected = response.headers.get("Content-Length")
        received = 0
        while chunk := response.read(1 << 20):
            received += handle.write(chunk)
        if expected is not None and received != int(expected):
            raise ValueError("Incomplete Oracle Tags download")
        handle.flush()
        os.fsync(handle.fileno())


@contextmanager
def open_compressed(path, mode):
    """Read by file signature; write the format selected by the output suffix.

    Bulk metadata may offer plain JSON instead of gzip JSONL. A cache filename
    must not make that valid fallback unreadable. ZIPs must contain one data file.
    """
    path = Path(path)
    with ExitStack() as stack:
        if mode == "rt":
            raw = stack.enter_context(path.open("rb"))
            magic = raw.read(6)
            raw.seek(0)
            if magic.startswith(b"\x1f\x8b"):
                stream = stack.enter_context(gzip.GzipFile(fileobj=raw))
            elif magic.startswith(b"BZh"):
                stream = stack.enter_context(bz2.BZ2File(raw))
            elif magic.startswith(b"\xfd7zXZ\x00"):
                stream = stack.enter_context(lzma.LZMAFile(raw))
            elif magic.startswith(b"PK"):
                archive = stack.enter_context(zipfile.ZipFile(raw))
                members = [m for m in archive.infolist() if not m.is_dir() and
                           Path(m.filename).suffix.lower() in (".json", ".jsonl")]
                if len(members) != 1:
                    raise ValueError("ZIP input must contain exactly one JSON or JSONL file")
                stream = stack.enter_context(archive.open(members[0]))
            else:
                stream = raw
            yield stack.enter_context(io.TextIOWrapper(stream, encoding="utf-8-sig"))
        elif mode == "wt":
            suffix = path.suffix.lower()
            if suffix == ".zip":
                archive = stack.enter_context(zipfile.ZipFile(path, "w", compression=zipfile.ZIP_DEFLATED))
                stream = stack.enter_context(archive.open("AllPrintings.json", "w"))
                yield stack.enter_context(io.TextIOWrapper(stream, encoding="utf-8"))
            else:
                opener = {".gz": gzip.open, ".bz2": bz2.open, ".xz": lzma.open}.get(suffix, open)
                yield stack.enter_context(opener(path, "wt", encoding="utf-8"))
        else:
            raise ValueError("Only text read/write modes are supported")


@contextmanager
def staged_file(destination):
    """Use the destination directory so final replacement stays atomic."""
    destination = Path(destination)
    fd, name = tempfile.mkstemp(prefix="." + destination.name + ".", suffix=destination.suffix,
                                dir=destination.parent)
    os.close(fd)
    staged = Path(name)
    try:
        yield staged
        os.replace(staged, destination)
    finally:
        staged.unlink(missing_ok=True)


def normalize_slug(slug):
    if not isinstance(slug, str):
        raise ValueError("Tag slugs must be strings")
    slug = slug.strip().lower()
    if not slug or any(c.isspace() for c in slug):
        raise ValueError("Tag slugs must be nonempty single tokens")
    return slug


def add_tag_object(mapping, tag):
    if not isinstance(tag, dict):
        raise ValueError("Oracle Tags records must be objects")
    slug = normalize_slug(tag.get("slug"))
    taggings = tag.get("taggings", [])
    if taggings is None:
        taggings = []
    if not isinstance(taggings, list):
        raise ValueError("Taggings must be an array")
    for tagging in taggings:
        if not isinstance(tagging, dict):
            raise ValueError("Taggings must contain objects")
        oracle_id = tagging.get("oracle_id")
        if oracle_id is not None:
            if not isinstance(oracle_id, str) or not oracle_id:
                raise ValueError("oracle_id must be a nonempty string")
            mapping.setdefault(oracle_id, set()).add(slug)


def build_oracle_id_to_tags(tags_path):
    mapping = {}
    with open_compressed(tags_path, "rt") as handle:
        head = handle.read(1)
        while head and head.isspace():
            head = handle.read(1)
        handle.seek(0)
        if head == "[":
            for tag in json.load(handle):
                add_tag_object(mapping, tag)
        else:
            for line in handle:
                if line.strip():
                    add_tag_object(mapping, json.loads(line))
    return {oracle_id: sorted(slugs) for oracle_id, slugs in mapping.items()}


def load_or_download_tags(args):
    if args.oracle_tags:
        path = Path(args.oracle_tags)
        return path, build_oracle_id_to_tags(path)
    cache = Path(args.cache_dir) / CACHE_FILE
    if cache.exists() and not args.force_download:
        print(f"Using cached Scryfall Oracle Tags: {cache}", file=sys.stderr)
        return cache, build_oracle_id_to_tags(cache)
    print(f"Fetching {args.api_url}", file=sys.stderr)
    metadata = fetch_json(args.api_url, args.timeout)
    if not isinstance(metadata, dict):
        raise ValueError("Scryfall bulk metadata must be an object")
    uri = metadata.get("jsonl_download_uri") or metadata.get("download_uri")
    if not isinstance(uri, str) or not uri:
        raise ValueError("Scryfall bulk data did not provide a download URI")
    print(f"Downloading {uri}", file=sys.stderr)
    cache.parent.mkdir(parents=True, exist_ok=True)
    with staged_file(cache) as staged:
        download(uri, staged, args.timeout)
        mapping = build_oracle_id_to_tags(staged)
    return cache, mapping


def inject_tags(all_printings, oracle_id_to_tags):
    if not isinstance(all_printings, dict) or not isinstance(all_printings.get("data"), dict):
        raise ValueError("AllPrintings must contain a data object")
    tagged_cards = total_cards = 0
    for card_set in all_printings["data"].values():
        if not isinstance(card_set, dict) or not isinstance(card_set.get("cards"), list):
            raise ValueError("Every AllPrintings set must contain a cards array")
        for card in card_set["cards"]:
            if not isinstance(card, dict):
                raise ValueError("AllPrintings cards must be objects")
            total_cards += 1
            identifiers = card.get("identifiers", {})
            if identifiers is None:
                identifiers = {}
            if not isinstance(identifiers, dict):
                raise ValueError("Card identifiers must be an object")
            existing = card.get("tags", [])
            if not isinstance(existing, list):
                raise ValueError("Card tags must be an array")
            slugs = {normalize_slug(tag) for tag in existing}
            oracle_id = identifiers.get("scryfallOracleId")
            if oracle_id is not None and not isinstance(oracle_id, str):
                raise ValueError("scryfallOracleId must be a string")
            matched = oracle_id_to_tags.get(oracle_id, [])
            slugs.update(matched)
            if matched:
                tagged_cards += 1
            if slugs or "tags" in card:
                card["tags"] = sorted(slugs)
    return total_cards, tagged_cards


def write_json(path, payload):
    with staged_file(path) as staged:
        with open_compressed(staged, "wt") as handle:
            json.dump(payload, handle, ensure_ascii=False, separators=(",", ":"))
        with staged.open("rb") as handle:
            os.fsync(handle.fileno())


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("input", help="MTGJson AllPrintings file (.json/.gz/.xz/.zip/.bz2)")
    parser.add_argument("-o", "--output", help="Output file; defaults to <input>.tagged.json")
    parser.add_argument("--oracle-tags", help="Pre-downloaded Oracle Tags JSONL/JSON file")
    parser.add_argument("--cache-dir", default=".", help="Where to cache the downloaded tags file")
    parser.add_argument("--api-url", default=DEFAULT_API_URL, help="Scryfall bulk-data endpoint for Oracle Tags")
    parser.add_argument("--force-download", action="store_true", help="Download and validate a fresh tags file")
    parser.add_argument("--timeout", type=float, default=30, help="Network operation timeout in seconds (default: 30)")
    args = parser.parse_args()
    if not math.isfinite(args.timeout) or args.timeout <= 0:
        parser.error("--timeout must be positive")
    output = args.output or f"{args.input}.tagged.json"
    try:
        _, mapping = load_or_download_tags(args)
        print(f"Loaded tags for {len(mapping)} oracle ids", file=sys.stderr)
        with open_compressed(args.input, "rt") as handle:
            all_printings = json.load(handle)
        total, tagged = inject_tags(all_printings, mapping)
        write_json(output, all_printings)
        print(f"Tagged {tagged}/{total} cards; wrote {output}", file=sys.stderr)
    except (OSError, ValueError, EOFError, zipfile.BadZipFile, http.client.HTTPException) as exc:
        parser.exit(1, f"Error: {exc}\n")


if __name__ == "__main__":
    main()
