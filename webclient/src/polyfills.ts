// Runtime polyfills that must execute before any other application module.
// Import this file first from `src/index.tsx`.

// ── BigInt.prototype.toJSON ───────────────────────────────────────────────────
// Protobuf-ES maps proto `int64`/`uint64` fields to native `BigInt`. Those
// land in Redux state (e.g. `ServerInfo_User.accountageSecs`,
// `Response_Register.deniedEndTime`, the outbound `cmdId`), and any consumer
// that JSON-stringifies state — notably the Redux DevTools browser
// extension, but also logging and error-boundary dumps — throws with
// "Do not know how to serialize a BigInt" because `BigInt.prototype` has no
// `toJSON`. Installing one globally makes `JSON.stringify` coerce
// `BigInt → string` instead of throwing. Coercion is lossy but only affects
// serialized representations; the in-memory Redux state still holds real
// `BigInt`s and every consumer reads them via the generated proto accessors.
(BigInt.prototype as unknown as { toJSON: () => string }).toJSON = function bigIntToJSON() {
  return this.toString();
};

export {};
