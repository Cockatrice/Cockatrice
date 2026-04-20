import { isFieldSet } from '@bufbuild/protobuf';
import type { DescMessage, MessageShape } from '@bufbuild/protobuf';

/**
 * Merges fields from `source` onto `target` in-place, copying only the fields
 * that are set on `source`. Use for reducers that receive partial proto
 * messages (e.g. desktop-server events where only changed fields are set on
 * the wire) — a naive assignment overwrites the target with protobuf defaults
 * for unset fields, wiping existing state.
 */
export function mergeSetFields<Desc extends DescMessage>(
  schema: Desc,
  target: MessageShape<Desc>,
  source: MessageShape<Desc>,
): void {
  for (const field of schema.fields) {
    if (isFieldSet(source, field)) {
      const key = field.localName as keyof MessageShape<Desc>;
      target[key] = source[key];
    }
  }
}
