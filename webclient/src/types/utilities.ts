/**
 * Init shape for constructing protobuf messages via create().
 * Strips $typeName and $unknown branding, making all fields optional.
 * Use for function parameters that feed into create().
 */
export type ProtoInit<T> = {
  [K in keyof T as K extends '$typeName' | '$unknown' ? never : K]?: T[K];
};
