#!/usr/bin/env python3
"""Generate an include-what-you-use mapping file for Qt 6 headers.

Qt 6 ships a capital forwarding header (e.g. <QWidget>) for each lowercase
private header (e.g. qwidget.h). include-what-you-use resolves symbols to the
lowercase header by default, so this script emits a mapping that makes IWYU
prefer the canonical capital includes, mirroring the qt5_11.imp bundled with
IWYU but adapted to the Qt 6 module layout.

For every capital header that forwards to exactly one lowercase header:

* a "symbol" mapping is emitted so IWYU suggests <QClass> when the definition
  of a class is needed, and
* an "include" mapping is emitted so a literal `#include <qclass.h>` in source
  is rewritten to its canonical capital header.

A lowercase header can be forwarded to by several capital headers (e.g.
qevent.h provides QCloseEvent, QMouseEvent, ...). In that case the include
mapping targets the primary class, i.e. the capital header whose lowercased
name equals the private header (QWidget for qwidget.h); headers with no such
primary (aggregates like qevent.h) get no include mapping and are left alone.

Usage:
    tools/generate_qt_mappings.py [QT_INSTALL_HEADERS]

QT_INSTALL_HEADERS defaults to `qmake6 -query QT_INSTALL_HEADERS` (falling back
to common locations). The mapping is written to stdout.
"""

import argparse
import os
import re
import subprocess
import sys

INCLUDE_RE = re.compile(r'^\s*#include\s*<\s*Qt[A-Za-z0-9_]+/+\s*([^>\s]+)>')
CAPITAL_NAME_RE = re.compile(r'^Q[A-Za-z0-9_]+$')
PRIVATE_NAME_RE = re.compile(r'^q[a-z0-9_]+\.h$')

# Private Qt headers whose public spelling has to be named explicitly. Keep an
# entry here only if the target header exists in every Qt we support; IWYU would
# otherwise keep suggesting the private lowercase include, which is correct only
# for headers that have shipped publicly since Qt 5.
PORTABLE_WRAPPERS = {
    # Qt 6.5 split the declarations that used to live in qglobal.h out into
    # separate headers, under both a QtFoo wrapper and a qfoo.h private name.
    # Neither spelling exists on the oldest Qt we support (6.4.2), and qtypes.h
    # itself only appeared in Qt 6.9, so all of them are hard compile errors
    # there. QtGlobal has been public since Qt 5, defines these declarations
    # itself up to Qt 6.4, and includes the split headers from Qt 6.5 on, so it
    # is the one spelling that resolves across the whole supported range.
    'qtypes.h': 'QtGlobal',
    'qtpreprocessorsupport.h': 'QtGlobal',
    'qtversionchecks.h': 'QtGlobal',
    'qassert.h': 'QtGlobal',
    'qminmax.h': 'QtGlobal',
    'qtclasshelpermacros.h': 'QtGlobal',
    'qtenvironmentvariables.h': 'QtGlobal',
    'qttranslation.h': 'QtGlobal',
    'qoverload.h': 'QtGlobal',
}


def run_or_empty(cmd):
    try:
        return subprocess.run(cmd, capture_output=True, text=True,
                              check=True).stdout.strip()
    except (OSError, subprocess.CalledProcessError):
        return ''


def default_qt_headers_dir():
    for cmd in (['qmake6', '-query', 'QT_INSTALL_HEADERS'],
                ['qmake', '-query', 'QT_INSTALL_HEADERS']):
        path = run_or_empty(cmd)
        if path:
            return path
    for path in ('/usr/include/qt6', '/usr/lib64/qt6/include',
                 '/usr/local/include/qt6'):
        if os.path.isdir(path):
            return path
    return ''


def collect_mappings(qt_headers_dir):
    forwarders = {}  # (module_dir, private) -> [capital names]
    symbols = {}     # capital name -> (module_dir, private)
    for entry in sorted(os.listdir(qt_headers_dir)):
        module_dir = os.path.join(qt_headers_dir, entry)
        if not entry.startswith('Qt') or not os.path.isdir(module_dir):
            continue
        for name in sorted(os.listdir(module_dir)):
            path = os.path.join(module_dir, name)
            if not os.path.isfile(path) or not CAPITAL_NAME_RE.match(name):
                continue
            forwarded = []
            with open(path, 'r', encoding='utf-8', errors='replace') as f:
                for line in f:
                    match = INCLUDE_RE.match(line)
                    if match:
                        forwarded.append(match.group(1))
                        if len(forwarded) > 1:
                            break
            if len(forwarded) != 1:
                continue
            private = forwarded[0]
            if not PRIVATE_NAME_RE.match(private):
                continue
            forwarders.setdefault((module_dir, private), []).append(name)
            symbols[name] = (module_dir, private)
    return forwarders, symbols


def primary_include_mappings(forwarders):
    mappings = {}
    for (module_dir, private), names in sorted(forwarders.items()):
        # Curated overrides win over the name rule below. They exist to redirect
        # an include to a header that is valid across our whole supported Qt
        # range, which the name rule cannot know about because it is generated
        # from whatever Qt happens to be installed.
        curated = PORTABLE_WRAPPERS.get(private)
        if curated is not None:
            mappings[(module_dir, private)] = curated
            continue
        # Prefer the forwarder whose name mirrors the private header
        # (QAbstractAnimation -> qabstractanimation.h).
        primary = next((n for n in names if n.lower() + '.h' == private), None)
        if primary is not None:
            mappings[(module_dir, private)] = primary
    return mappings


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('qt_headers_dir', nargs='?', default=None,
                        help='Qt 6 headers dir (default: qmake query)')
    args = parser.parse_args()

    qt_headers_dir = args.qt_headers_dir or default_qt_headers_dir()
    if not qt_headers_dir or not os.path.isdir(qt_headers_dir):
        sys.stderr.write('error: could not locate Qt headers, pass the include '
                         'dir\n')
        return 1

    forwarders, symbols = collect_mappings(qt_headers_dir)
    includes = primary_include_mappings(forwarders)

    # A curated key that matches nothing is almost always a typo, and it fails
    # silently: the entry just falls back to the name rule and emits the
    # version-incompatible wrapper the override was meant to prevent.
    known_private = {private for (_module_dir, private) in forwarders}
    unknown = sorted(set(PORTABLE_WRAPPERS) - known_private)
    if unknown:
        sys.stderr.write('error: PORTABLE_WRAPPERS keys not found in the Qt '
                         'headers: %s\n' % ', '.join(unknown))
        return 1

    entries = []
    # Headers we redirect in PORTABLE_WRAPPERS do not exist on the oldest
    # supported Qt, so do not advertise them as the home of a symbol either.
    redirected = {name
                  for (module_dir, private), names in forwarders.items()
                  if private in PORTABLE_WRAPPERS
                  for name in names}
    for name in sorted(symbols):
        if name in redirected:
            continue
        entries.append(
            f'  {{ "symbol": [ "{name}", "private", "<{name}>", "public"] }},')

    # Free operators of QSharedPointer/QWeakPointer live in qsharedpointer.h.
    # IWYU does not apply "include" mappings when *adding* an include for an
    # operator or other non-class symbol, so map them to the canonical capital
    # header explicitly. A use is only reported when the operator is not
    # already covered by an included header, so this does not mis-attribute
    # comparison operators of other types (which are provided by their own
    # type headers).
    for name in ('operator==', 'operator!=', 'operator<', 'operator<=',
                 'operator>', 'operator>=', 'comparesEqual'):
        entries.append(
            f'  {{ "symbol": [ "{name}", "private", "<QSharedPointer>", "public"] }},')
    for (module_dir, private), public in sorted(includes.items()):
        module = os.path.basename(module_dir)
        # IWYU's .imp tokenizer treats the value as a JSON-ish string:
        # '\"' becomes a quote and '\\' becomes a backslash for the regex.
        quoted = '\\"'  # \" -- a JSON-escaped double quote in the .imp file
        dotted = re.escape(private).replace('\\', '\\\\')
        pattern = ('@[' + quoted + '<](' + re.escape(module) + r'/)?' +
                   dotted + '[' + quoted + '>]')
        entries.append(
            f'  {{ "include": [ "{pattern}", "private", "<{public}>", "public"] }},')

    if not entries:
        sys.stderr.write('error: no Qt header mappings found\n')
        return 1

    print(f'# Do not edit! Generated by {os.path.basename(__file__)} from')
    print(f'# {qt_headers_dir}')
    print('[')
    for entry in entries[:-1]:
        print(entry)
    print(entries[-1].rstrip(','))
    print(']')
    return 0


if __name__ == '__main__':
    sys.exit(main())
