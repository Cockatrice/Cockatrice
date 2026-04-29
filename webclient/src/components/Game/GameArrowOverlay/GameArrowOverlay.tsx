import { useGameArrowOverlay } from './useGameArrowOverlay';

import './GameArrowOverlay.css';

export interface GameArrowOverlayProps {
  gameId: number | undefined;
  boardRef: React.RefObject<HTMLElement | null>;
  dragPreview?: { x1: number; y1: number; x2: number; y2: number; color: string } | null;
}

function GameArrowOverlay({ gameId, boardRef, dragPreview = null }: GameArrowOverlayProps) {
  const { arrows, width, height, handleArrowClick } = useGameArrowOverlay({ gameId, boardRef });

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
