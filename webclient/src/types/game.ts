export interface Game {
  description: string;
  gameId: number;
  gameType: string;
  gameTypes: string[];
  roomId: number;
  started: boolean;
}

export enum GameSortField {
  START_TIME = 'startTime'
}

export interface GameConfig {
  description: string;
  password: string;
  maxPlayer: number;
  onlyBuddies: boolean;
  onlyRegistered: boolean;
  spectatorsAllowed: boolean;
  spectatorsNeedPassword: boolean;
  spectatorsCanTalk: boolean;
  spectatorsSeeEverything: boolean;
  gameTypeIds: number[];
  joinAsJudge: boolean;
  joinAsSpectator: boolean;
}

export interface JoinGameParams {
  gameId: number;
  password: string;
  spectator: boolean;
  overrideRestrictions: boolean;
  joinAsJudge: boolean;
}

export enum LeaveGameReason {
  OTHER = 1,
  USER_KICKED = 2,
  USER_LEFT = 3,
  USER_DISCONNECTED = 4
}

// ── Enums ────────────────────────────────────────────────────────────────────

export enum ZoneType {
  PrivateZone = 0,
  PublicZone = 1,
  HiddenZone = 2,
}

/** Matches CardAttribute enum in card_attributes.proto */
export enum CardAttribute {
  AttrTapped = 1,
  AttrAttacking = 2,
  AttrFaceDown = 3,
  AttrColor = 4,
  AttrPT = 5,
  AttrAnnotation = 6,
  AttrDoesntUntap = 7,
}

// ── Primitive data structures (mirrors ServerInfo_* protos) ──────────────────

export interface Color {
  r: number;
  g: number;
  b: number;
  a: number;
}

/** Mirrors ServerInfo_CardCounter */
export interface CardCounterInfo {
  id: number;
  value: number;
}

/** Mirrors ServerInfo_Card */
export interface CardInfo {
  id: number;
  name: string;
  x: number;
  y: number;
  faceDown: boolean;
  tapped: boolean;
  attacking: boolean;
  color: string;
  pt: string;
  annotation: string;
  destroyOnZoneChange: boolean;
  doesntUntap: boolean;
  counterList: CardCounterInfo[];
  attachPlayerId: number;
  attachZone: string;
  attachCardId: number;
  providerId: string;
}

/** Mirrors ServerInfo_Zone */
export interface ZoneInfo {
  name: string;
  type: ZoneType;
  withCoords: boolean;
  cardCount: number;
  cardList: CardInfo[];
  alwaysRevealTopCard: boolean;
  alwaysLookAtTopCard: boolean;
}

/** Mirrors ServerInfo_Counter */
export interface CounterInfo {
  id: number;
  name: string;
  counterColor: Color;
  radius: number;
  count: number;
}

/** Mirrors ServerInfo_Arrow */
export interface ArrowInfo {
  id: number;
  startPlayerId: number;
  startZone: string;
  startCardId: number;
  targetPlayerId: number;
  targetZone: string;
  targetCardId: number;
  arrowColor: Color;
}

/** Mirrors ServerInfo_PlayerProperties */
export interface PlayerProperties {
  playerId: number;
  userInfo: any;
  spectator: boolean;
  conceded: boolean;
  readyStart: boolean;
  deckHash: string;
  pingSeconds: number;
  sideboardLocked: boolean;
  judge: boolean;
}

/** Mirrors ServerInfo_Player */
export interface PlayerInfo {
  properties: PlayerProperties;
  deckList: string;
  zoneList: ZoneInfo[];
  counterList: CounterInfo[];
  arrowList: ArrowInfo[];
}

// ── Game event payload interfaces (data arriving from server events) ──────────

export interface GameStateChangedData {
  playerList: PlayerInfo[];
  gameStarted: boolean;
  activePlayerId: number;
  activePhase: number;
  secondsElapsed: number;
}

export interface GameSayData {
  message: string;
}

export interface MoveCardData {
  cardId: number;
  cardName: string;
  startPlayerId: number;
  startZone: string;
  position: number;
  targetPlayerId: number;
  targetZone: string;
  x: number;
  y: number;
  newCardId: number;
  faceDown: boolean;
  newCardProviderId: string;
}

export interface FlipCardData {
  zoneName: string;
  cardId: number;
  cardName: string;
  faceDown: boolean;
  cardProviderId: string;
}

export interface DestroyCardData {
  zoneName: string;
  cardId: number;
}

export interface AttachCardData {
  startZone: string;
  cardId: number;
  targetPlayerId: number;
  targetZone: string;
  targetCardId: number;
}

export interface CreateTokenData {
  zoneName: string;
  cardId: number;
  cardName: string;
  color: string;
  pt: string;
  annotation: string;
  destroyOnZoneChange: boolean;
  x: number;
  y: number;
  cardProviderId: string;
  faceDown: boolean;
}

export interface SetCardAttrData {
  zoneName: string;
  cardId: number;
  attribute: CardAttribute;
  attrValue: string;
}

export interface SetCardCounterData {
  zoneName: string;
  cardId: number;
  counterId: number;
  counterValue: number;
}

export interface CreateArrowData {
  arrowInfo: ArrowInfo;
}

export interface DeleteArrowData {
  arrowId: number;
}

export interface CreateCounterData {
  counterInfo: CounterInfo;
}

export interface SetCounterData {
  counterId: number;
  value: number;
}

export interface DelCounterData {
  counterId: number;
}

export interface DrawCardsData {
  number: number;
  cards: CardInfo[];
}

export interface RevealCardsData {
  zoneName: string;
  cardId: number[];
  otherPlayerId: number;
  cards: CardInfo[];
  grantWriteAccess: boolean;
  numberOfCards: number;
}

export interface ShuffleData {
  zoneName: string;
  start: number;
  end: number;
}

export interface RollDieData {
  sides: number;
  value: number;
  values: number[];
}

export interface DumpZoneData {
  zoneOwnerId: number;
  zoneName: string;
  numberCards: number;
  isReversed: boolean;
}

export interface ChangeZonePropertiesData {
  zoneName: string;
  alwaysRevealTopCard: boolean;
  alwaysLookAtTopCard: boolean;
}

export interface SetActivePlayerData {
  activePlayerId: number;
}

export interface SetActivePhaseData {
  phase: number;
}

export interface ReverseTurnData {
  reversed: boolean;
}

/**
 * Passed to every game event handler alongside the event payload.
 * Contains per-container metadata from GameEventContainer.
 * Not stored in Redux — transient routing metadata only.
 */
export interface GameEventMeta {
  gameId: number;
  playerId: number;
  /** Raw protobuf GameEventContext object. Not stored in Redux. */
  context: any;
  secondsElapsed: number;
  /** Proto type is uint32. Non-zero means the action was forced by a judge. */
  forcedByJudge: number;
}

// ── Command parameter interfaces ─────────────────────────────────────────────

export interface CardToMove {
  cardId: number;
  faceDown?: boolean;
  pt?: string;
  tapped?: boolean;
}

export interface MoveCardParams {
  startPlayerId: number;
  startZone: string;
  cardsToMove: { card: CardToMove[] };
  targetPlayerId: number;
  targetZone: string;
  x?: number;
  y?: number;
  isReversed?: boolean;
}

export interface DrawCardsParams {
  number: number;
}

export interface RollDieParams {
  sides: number;
  count?: number;
}

export interface ShuffleParams {
  zoneName: string;
  start?: number;
  end?: number;
}

export interface FlipCardParams {
  zone: string;
  cardId: number;
  faceDown: boolean;
  pt?: string;
}

export interface AttachCardParams {
  startZone: string;
  cardId: number;
  targetPlayerId?: number;
  targetZone?: string;
  targetCardId?: number;
}

export interface CreateTokenParams {
  zone: string;
  cardName: string;
  color?: string;
  pt?: string;
  annotation?: string;
  destroyOnZoneChange?: boolean;
  x?: number;
  y?: number;
  targetZone?: string;
  targetCardId?: number;
  targetMode?: number;
  cardProviderId?: string;
  faceDown?: boolean;
}

export interface SetCardAttrParams {
  zone: string;
  cardId: number;
  attribute: CardAttribute;
  attrValue: string;
}

export interface SetCardCounterParams {
  zone: string;
  cardId: number;
  counterId: number;
  counterValue: number;
}

export interface IncCardCounterParams {
  zone: string;
  cardId: number;
  counterId: number;
  counterDelta: number;
}

export interface RevealCardsParams {
  zoneName: string;
  cardId?: number[];
  playerId?: number;
  grantWriteAccess?: boolean;
  topCards?: number;
}

export interface DumpZoneParams {
  playerId: number;
  zoneName: string;
  numberCards: number;
  isReversed?: boolean;
}

export interface ChangeZonePropertiesParams {
  zoneName: string;
  alwaysRevealTopCard?: boolean;
  alwaysLookAtTopCard?: boolean;
}

export interface CreateArrowParams {
  startPlayerId: number;
  startZone: string;
  startCardId: number;
  targetPlayerId: number;
  targetZone?: string;
  targetCardId?: number;
  arrowColor: Color;
  deleteInPhase?: number;
}

export interface DeleteArrowParams {
  arrowId: number;
}

export interface CreateCounterParams {
  counterName: string;
  counterColor: Color;
  radius: number;
  value: number;
}

export interface SetCounterParams {
  counterId: number;
  value: number;
}

export interface IncCounterParams {
  counterId: number;
  delta: number;
}

export interface DelCounterParams {
  counterId: number;
}

export interface KickFromGameParams {
  playerId: number;
}

export interface ReadyStartParams {
  ready: boolean;
  forceStart?: boolean;
}

export interface MulliganParams {
  number: number;
}

export interface DeckSelectParams {
  deck?: string;
  deckId?: number;
}

export interface MoveCardToZone {
  cardName: string;
  startZone: string;
  targetZone: string;
}

export interface SetSideboardPlanParams {
  moveList: MoveCardToZone[];
}

export interface SetSideboardLockParams {
  locked: boolean;
}

export interface SetActivePhaseParams {
  phase: number;
}

export interface GameSayParams {
  message: string;
}
