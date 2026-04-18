// @critical Must be imported before any module that can JSON-stringify Redux state
// (BigInt proto fields throw without toJSON). See .github/instructions/webclient.instructions.md#initialization-order.
(BigInt.prototype as unknown as { toJSON: () => string }).toJSON = function bigIntToJSON() {
  return this.toString();
};

export {};
