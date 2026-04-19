function s4(buf: Uint16Array, idx: number): string {
  // Mask to 16 bits then OR 0x10000 so the leading nibble is always present
  // (guarantees 4 hex digits without padding logic).
  const v = (buf[idx] & 0xffff) | 0x10000;
  return v.toString(16).substring(1);
}

export function guid(): string {
  const buf = new Uint16Array(8);
  crypto.getRandomValues(buf);
  return (
    s4(buf, 0) + s4(buf, 1) + '-' + s4(buf, 2) + '-' + s4(buf, 3) + '-' +
    s4(buf, 4) + '-' + s4(buf, 5) + s4(buf, 6) + s4(buf, 7)
  );
}
