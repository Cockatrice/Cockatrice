export enum Phase {
  Untap = 0,
  Upkeep = 1,
  Draw = 2,
  FirstMain = 3,
  BeginCombat = 4,
  DeclareAttackers = 5,
  DeclareBlockers = 6,
  CombatDamage = 7,
  EndCombat = 8,
  SecondMain = 9,
  EndCleanup = 10,
}

export const ZoneName = {
  TABLE: 'table',
  GRAVE: 'grave',
  EXILE: 'rfg',
  HAND: 'hand',
  DECK: 'deck',
  SIDEBOARD: 'sb',
  STACK: 'stack',
} as const;

export type ZoneName = typeof ZoneName[keyof typeof ZoneName];

export enum ScryfallImageSize {
  Small = 'small',
  Normal = 'normal',
  Large = 'large',
  Png = 'png',
  ArtCrop = 'art_crop',
  BorderCrop = 'border_crop',
}
