function s4() {
  const s4 = Math.floor((1 + Math.random()) * 0x10000);
  return s4.toString(16).substring(1);
}

export function guid() {
  return s4() + s4() + "-" + s4() + "-" + s4() + "-" + s4() + "-" + s4() + s4() + s4();
}