# @cockatrice/protocol

Network protocol artifacts for [Cockatrice](https://github.com/Cockatrice/Cockatrice): the `.proto`
definitions used by the desktop client, Servatrice, and the webclient, plus the
authoritative protocol version constant they all share.

## Install

```sh
npm install @cockatrice/protocol
```

The package is published to GitHub Packages under the `@cockatrice` scope; consumers
need an `.npmrc` entry pointing the scope at `https://npm.pkg.github.com` and a
`GITHUB_TOKEN` with `read:packages`.

## Contents

- `pb/*.proto` — every protobuf schema file from `libcockatrice_protocol`.
- `protocol_version.json` — `{ "protocolVersion": <int> }`. Identical to the file
  the C++ build reads via `configure_file()`.

## Usage (TypeScript)

```ts
import protocolVersionInfo from "@cockatrice/protocol/protocol_version.json" with { type: "json" };
export const PROTOCOL_VERSION = protocolVersionInfo.protocolVersion;
```

Point your protobuf code-generator (e.g. buf) at `node_modules/@cockatrice/protocol/pb`.
