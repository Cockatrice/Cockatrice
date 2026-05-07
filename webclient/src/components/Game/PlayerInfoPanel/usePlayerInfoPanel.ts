import { useWebClient } from '@app/hooks';
import { GameSelectors, useAppSelector } from '@app/store';
import type { Data, Enriched } from '@app/types';

type ServerColor = { r: number; g: number; b: number; a: number } | undefined;

// Canonical MTG mana colors, mirroring the dead-code table in desktop's
// libcockatrice_utility/libcockatrice/utility/color.h colorHelper(). Used
// as a fallback when the server leaves counterColor unset or all-zero.
const NAME_COLOR_MAP: Record<string, [number, number, number]> = {
  W: [245, 245, 220],
  U: [80, 140, 255],
  B: [60, 60, 60],
  R: [220, 60, 50],
  G: [70, 160, 70],
  LIFE: [220, 140, 60],
};

function isBlankColor(c: ServerColor): boolean {
  if (!c) {
    return true;
  }
  if ((c.a ?? 255) === 0) {
    return true;
  }
  return c.r === 0 && c.g === 0 && c.b === 0;
}

// Stable 32-bit FNV-1a hash of a string. Matches the shape of desktop's
// qHash-based procedural fallback for unknown counter names.
function hashName(name: string): number {
  let h = 0x811c9dc5;
  for (let i = 0; i < name.length; i++) {
    h ^= name.charCodeAt(i);
    h = Math.imul(h, 0x01000193);
  }
  return h >>> 0;
}

function hashColor(name: string): [number, number, number] {
  const h = hashName(name);
  return [100 + (h % 120), 100 + ((h >> 8) % 120), 100 + ((h >> 16) % 120)];
}

export function counterCssColor(counter: { name: string; counterColor?: ServerColor }): string {
  const server = counter.counterColor;
  if (!isBlankColor(server)) {
    return `rgba(${server!.r}, ${server!.g}, ${server!.b}, ${(server!.a ?? 255) / 255})`;
  }
  const key = counter.name.trim().toUpperCase();
  const mapped = NAME_COLOR_MAP[key];
  if (mapped) {
    return `rgba(${mapped[0]}, ${mapped[1]}, ${mapped[2]}, 1)`;
  }
  const [r, g, b] = hashColor(counter.name);
  return `rgba(${r}, ${g}, ${b}, 1)`;
}

function isLifeCounter(c: { name: string }): boolean {
  return c.name.trim().toLowerCase() === 'life';
}

export interface PlayerInfoPanel {
  player: Enriched.PlayerEntry | undefined;
  isHost: boolean;
  // Life renders in the header alongside the name; all other counters render
  // as circles centered in the rail body.
  lifeCounter: Data.ServerInfo_Counter | undefined;
  otherCounters: Data.ServerInfo_Counter[];
  handleIncrement: (counterId: number, delta: number) => void;
}

export interface UsePlayerInfoPanelArgs {
  gameId: number;
  playerId: number;
}

export function usePlayerInfoPanel({
  gameId,
  playerId,
}: UsePlayerInfoPanelArgs): PlayerInfoPanel {
  const webClient = useWebClient();
  const player = useAppSelector((state) => GameSelectors.getPlayer(state, gameId, playerId));
  const countersMap = useAppSelector((state) => GameSelectors.getCounters(state, gameId, playerId));
  const hostId = useAppSelector((state) => GameSelectors.getHostId(state, gameId));

  const isHost = hostId != null && hostId === playerId;
  const allCounters = Object.values(countersMap);
  const lifeCounter = allCounters.find(isLifeCounter);
  const otherCounters = allCounters.filter((c) => !isLifeCounter(c));

  const handleIncrement = (counterId: number, delta: number) => {
    webClient.request.game.incCounter(gameId, { counterId, delta });
  };

  return {
    player,
    isHost,
    lifeCounter,
    otherCounters,
    handleIncrement,
  };
}
