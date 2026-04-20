import { useCallback, useEffect, useLayoutEffect, useMemo, useState } from 'react';

import { useWebClient } from '@app/hooks';
import { GameSelectors, useAppSelector } from '@app/store';
import { App } from '@app/types';
import type { Data, Enriched } from '@app/types';

import { makeCardKey, useCardRegistry } from '../CardRegistry/CardRegistryContext';

import './GameArrowOverlay.css';

export interface GameArrowOverlayProps {
  gameId: number | undefined;
  boardRef: React.RefObject<HTMLElement | null>;
  dragPreview?: { x1: number; y1: number; x2: number; y2: number; color: string } | null;
}

interface ResolvedArrow {
  arrowId: number;
  ownerPlayerId: number;
  x1: number;
  y1: number;
  x2: number;
  y2: number;
  color: string;
}

const ARROW_FALLBACK_CSS = App.rgbaToCss(App.ArrowColor.RED);

function cssColor(c: { r: number; g: number; b: number; a: number } | undefined): string {
  if (!c) {
    return ARROW_FALLBACK_CSS;
  }
  return App.rgbaToCss({ r: c.r, g: c.g, b: c.b, a: c.a ?? 255 });
}

function GameArrowOverlay({ gameId, boardRef, dragPreview = null }: GameArrowOverlayProps) {
  const webClient = useWebClient();
  const registry = useCardRegistry();
  const players = useAppSelector((state) =>
    gameId != null ? GameSelectors.getPlayers(state, gameId) : undefined,
  );

  // Tick is bumped whenever we need to re-query DOM rects (card registry
  // mutation, board resize). Keeps the overlay declarative without an external
  // layout engine.
  const [tick, setTick] = useState(0);
  const bump = useCallback(() => {
    setTick((t) => t + 1);
  }, []);

  useEffect(() => {
    if (!registry) {
      return undefined;
    }
    return registry.subscribe(bump);
  }, [registry, bump]);

  // First-paint: the board ref is null during the initial render, so `boardRect`
  // is undefined and the arrows memo bails out. Bump once after mount so the
  // next render sees a populated ref.
  useLayoutEffect(() => {
    bump();
  }, [bump]);

  useLayoutEffect(() => {
    const el = boardRef.current;
    if (!el || typeof ResizeObserver === 'undefined') {
      return undefined;
    }
    const ro = new ResizeObserver(() => bump());
    ro.observe(el);
    return () => ro.disconnect();
  }, [boardRef, bump]);

  const boardRect = boardRef.current?.getBoundingClientRect();

  const arrows = useMemo<ResolvedArrow[]>(() => {
    if (!players || !registry || !boardRect) {
      return [];
    }
    const out: ResolvedArrow[] = [];
    for (const player of Object.values(players) as Enriched.PlayerEntry[]) {
      for (const a of Object.values(player.arrows) as Data.ServerInfo_Arrow[]) {
        const sourceEl = registry.get(
          makeCardKey(a.startPlayerId, a.startZone, a.startCardId),
        );
        const targetEl = registry.get(
          makeCardKey(a.targetPlayerId, a.targetZone, a.targetCardId),
        );
        if (!sourceEl || !targetEl) {
          continue;
        }
        const s = sourceEl.getBoundingClientRect();
        const t = targetEl.getBoundingClientRect();
        out.push({
          arrowId: a.id,
          ownerPlayerId: player.properties.playerId,
          x1: s.left + s.width / 2 - boardRect.left,
          y1: s.top + s.height / 2 - boardRect.top,
          x2: t.left + t.width / 2 - boardRect.left,
          y2: t.top + t.height / 2 - boardRect.top,
          color: cssColor(a.arrowColor),
        });
      }
    }
    // `tick` in deps intentionally re-runs the memo on DOM-layout changes.
    return out;
  }, [players, registry, boardRect, tick]);

  const handleArrowClick = (arrowId: number) => {
    if (gameId == null) {
      return;
    }
    webClient.request.game.deleteArrow(gameId, { arrowId });
  };

  const width = boardRect?.width ?? 0;
  const height = boardRect?.height ?? 0;

  return (
    <svg
      className="game-arrow-overlay"
      data-testid="game-arrow-overlay"
      width={width}
      height={height}
      viewBox={`0 0 ${width} ${height}`}
      preserveAspectRatio="none"
    >
      <defs>
        <marker
          id="game-arrow-overlay__head"
          viewBox="0 0 12 12"
          refX="10"
          refY="6"
          markerWidth="10"
          markerHeight="10"
          orient="auto-start-reverse"
        >
          <path d="M0,0 L12,6 L0,12 z" fill="currentColor" />
        </marker>
      </defs>
      {arrows.map((a) => (
        <line
          key={a.arrowId}
          className="game-arrow-overlay__line"
          data-testid={`arrow-${a.arrowId}`}
          x1={a.x1}
          y1={a.y1}
          x2={a.x2}
          y2={a.y2}
          stroke={a.color}
          style={{ color: a.color }}
          markerEnd="url(#game-arrow-overlay__head)"
          onClick={() => handleArrowClick(a.arrowId)}
        />
      ))}
      {dragPreview && (
        <line
          className="game-arrow-overlay__line game-arrow-overlay__line--preview"
          data-testid="arrow-preview"
          x1={dragPreview.x1}
          y1={dragPreview.y1}
          x2={dragPreview.x2}
          y2={dragPreview.y2}
          stroke={dragPreview.color}
          style={{ color: dragPreview.color }}
          markerEnd="url(#game-arrow-overlay__head)"
        />
      )}
    </svg>
  );
}

export default GameArrowOverlay;
