// Byte-level verification that Command_AttachCard's proto2 optional target
// fields stay UNSET when we build an "Unattach" — i.e. the client omits
// them entirely rather than sending sentinel values. Desktop's server-side
// `has_target_player_id()` / `has_target_zone()` / `has_target_card_id()`
// drives the unattach branch; if any target field gets serialized, the
// server treats it as a re-attach instead of an unattach.
//
// See the M6 Unattach deferrable in webclient/plans/gameboard-deferrables.md.

import { create, isFieldSet, toBinary, fromBinary } from '@bufbuild/protobuf';
import { Command_AttachCardSchema } from '@app/generated';

describe('Command_AttachCard proto2 presence (Unattach invariant)', () => {
  it('omitting target fields leaves them unset on the in-memory message', () => {
    const msg = create(Command_AttachCardSchema, {
      startZone: 'table',
      cardId: 10,
    });

    expect(isFieldSet(msg, Command_AttachCardSchema.field.startZone)).toBe(true);
    expect(isFieldSet(msg, Command_AttachCardSchema.field.cardId)).toBe(true);
    expect(isFieldSet(msg, Command_AttachCardSchema.field.targetPlayerId)).toBe(false);
    expect(isFieldSet(msg, Command_AttachCardSchema.field.targetZone)).toBe(false);
    expect(isFieldSet(msg, Command_AttachCardSchema.field.targetCardId)).toBe(false);
  });

  it('omitting target fields omits them from the serialized bytes', () => {
    const msg = create(Command_AttachCardSchema, {
      startZone: 'table',
      cardId: 10,
    });

    const bytes = toBinary(Command_AttachCardSchema, msg);
    // Round-trip to confirm the wire format also reports the fields unset.
    const parsed = fromBinary(Command_AttachCardSchema, bytes);

    expect(isFieldSet(parsed, Command_AttachCardSchema.field.targetPlayerId)).toBe(false);
    expect(isFieldSet(parsed, Command_AttachCardSchema.field.targetZone)).toBe(false);
    expect(isFieldSet(parsed, Command_AttachCardSchema.field.targetCardId)).toBe(false);
  });

  it('passing an explicit targetPlayerId DOES set presence (so "attach" paths still work)', () => {
    const msg = create(Command_AttachCardSchema, {
      startZone: 'table',
      cardId: 10,
      targetPlayerId: 5,
      targetZone: 'table',
      targetCardId: 99,
    });

    const bytes = toBinary(Command_AttachCardSchema, msg);
    const parsed = fromBinary(Command_AttachCardSchema, bytes);

    expect(isFieldSet(parsed, Command_AttachCardSchema.field.targetPlayerId)).toBe(true);
    expect(isFieldSet(parsed, Command_AttachCardSchema.field.targetZone)).toBe(true);
    expect(isFieldSet(parsed, Command_AttachCardSchema.field.targetCardId)).toBe(true);
    expect(parsed.targetPlayerId).toBe(5);
  });

  it('setting a target field to its default -1 still records presence (presence != default)', () => {
    // This is the trap the deferrable warned about: proto2 `optional` with a
    // default of -1 treats an explicit `-1` as "field set with value -1",
    // which the server interprets as "attach to slot -1" rather than unattach.
    // Clients must OMIT the field entirely to unattach.
    const msg = create(Command_AttachCardSchema, {
      startZone: 'table',
      cardId: 10,
      targetPlayerId: -1,
    });

    expect(isFieldSet(msg, Command_AttachCardSchema.field.targetPlayerId)).toBe(true);
  });
});
