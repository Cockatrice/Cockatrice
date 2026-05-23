#!/usr/bin/env node
// Assembles a publish-ready @cockatrice/protocol package directory from
// libcockatrice_protocol/. Run by .github/workflows/protocol-publish.yml.
// stdlib only.

import { readdirSync, readFileSync, mkdirSync, copyFileSync, writeFileSync, rmSync, existsSync } from "node:fs";
import { join, resolve, dirname } from "node:path";
import { fileURLToPath } from "node:url";

const REQUIRED_CORE_PROTOS = ["commands.proto", "server_message.proto", "event_server_identification.proto"];

function parseArgs(argv) {
  const args = { version: null, outDir: null };
  for (let i = 0; i < argv.length; i++) {
    const a = argv[i];
    if (a === "--version") args.version = argv[++i];
    else if (a.startsWith("--version=")) args.version = a.slice("--version=".length);
    else if (a === "--out-dir") args.outDir = argv[++i];
    else if (a.startsWith("--out-dir=")) args.outDir = a.slice("--out-dir=".length);
    else die(`unknown argument: ${a}`);
  }
  if (!args.version) die("missing required --version <semver>");
  return args;
}

function die(msg) {
  console.error(`package-protocol: ${msg}`);
  process.exit(1);
}

function normalizeVersion(raw) {
  const stripped = raw.replace(/^v/, "");
  if (!/^\d+\.\d+\.\d+(-[0-9A-Za-z.-]+)?$/.test(stripped)) {
    die(`version "${raw}" is not a valid semver`);
  }
  return stripped;
}

const __filename = fileURLToPath(import.meta.url);
const repoRoot = resolve(dirname(__filename), "..");
const protoSrcDir = join(repoRoot, "libcockatrice_protocol", "libcockatrice", "protocol", "pb");
const versionJsonSrc = join(repoRoot, "libcockatrice_protocol", "protocol_version.json");
const licenseSrc = join(repoRoot, "LICENSE");

const { version: rawVersion, outDir: outDirArg } = parseArgs(process.argv.slice(2));
const pkgVersion = normalizeVersion(rawVersion);
const outDir = resolve(outDirArg ?? join(repoRoot, "build", "protocol-package"));

if (existsSync(outDir)) rmSync(outDir, { recursive: true, force: true });
mkdirSync(join(outDir, "pb"), { recursive: true });

const protoFiles = readdirSync(protoSrcDir).filter((f) => f.endsWith(".proto"));
if (protoFiles.length === 0) die(`no .proto files found in ${protoSrcDir}`);

const missing = REQUIRED_CORE_PROTOS.filter((f) => !protoFiles.includes(f));
if (missing.length > 0) {
  die(`required core proto files missing: ${missing.join(", ")} (layout of ${protoSrcDir} changed?)`);
}

for (const f of protoFiles) {
  copyFileSync(join(protoSrcDir, f), join(outDir, "pb", f));
}

copyFileSync(versionJsonSrc, join(outDir, "protocol_version.json"));
copyFileSync(licenseSrc, join(outDir, "LICENSE"));

const pkgJson = {
  name: "@cockatrice/protocol",
  version: pkgVersion,
  description: "Cockatrice network protocol: .proto definitions and protocol version constant.",
  license: "GPL-2.0-or-later",
  repository: { type: "git", url: "git+https://github.com/Cockatrice/Cockatrice.git" },
  homepage: "https://github.com/Cockatrice/Cockatrice",
  files: ["pb/", "protocol_version.json", "LICENSE", "README.md"],
  publishConfig: { registry: "https://npm.pkg.github.com", access: "restricted" },
  exports: {
    "./protocol_version.json": "./protocol_version.json",
    "./pb/*.proto": "./pb/*.proto",
  },
};
writeFileSync(join(outDir, "package.json"), JSON.stringify(pkgJson, null, 2) + "\n");

const readme = `# @cockatrice/protocol

Network protocol artifacts for [Cockatrice](https://github.com/Cockatrice/Cockatrice): the \`.proto\`
definitions used by the desktop client, Servatrice, and the webclient, plus the
authoritative protocol version constant they all share.

## Install

\`\`\`sh
npm install @cockatrice/protocol
\`\`\`

The package is published to GitHub Packages under the \`@cockatrice\` scope; consumers
need an \`.npmrc\` entry pointing the scope at \`https://npm.pkg.github.com\` and a
\`GITHUB_TOKEN\` with \`read:packages\`.

## Contents

- \`pb/*.proto\` — every protobuf schema file from \`libcockatrice_protocol\`.
- \`protocol_version.json\` — \`{ "protocolVersion": <int> }\`. Identical to the file the
  C++ build reads via \`configure_file()\`.

## Usage (TypeScript)

\`\`\`ts
import protocolVersionInfo from "@cockatrice/protocol/protocol_version.json" with { type: "json" };
export const PROTOCOL_VERSION = protocolVersionInfo.protocolVersion;
\`\`\`

Point your protobuf code-generator (e.g. buf) at \`node_modules/@cockatrice/protocol/pb\`.
`;
writeFileSync(join(outDir, "README.md"), readme);

console.log(`package-protocol: assembled ${outDir}`);
console.log(`  name:    ${pkgJson.name}`);
console.log(`  version: ${pkgJson.version}`);
console.log(`  proto:   ${protoFiles.length} files`);
