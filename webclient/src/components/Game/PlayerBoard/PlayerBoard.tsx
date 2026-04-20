import type { Data } from '@app/types';
import { cx } from '@app/utils';

import Battlefield from '../Battlefield/Battlefield';
import PlayerInfoPanel from '../PlayerInfoPanel/PlayerInfoPanel';
import ZoneRail from '../ZoneRail/ZoneRail';

import './PlayerBoard.css';

export interface PlayerBoardProps {
  gameId: number;
  playerId: number;
  mirrored?: boolean;
  canAct?: boolean;
  canEditCounters?: boolean;
  arrowSourceKey?: string | null;
  onCardHover?: (card: Data.ServerInfo_Card) => void;
  onCardClick?: (playerId: number, zone: string, card: Data.ServerInfo_Card) => void;
  onCardContextMenu?: (card: Data.ServerInfo_Card, event: React.MouseEvent) => void;
  onCardDoubleClick?: (card: Data.ServerInfo_Card) => void;
  onZoneClick?: (playerId: number, zoneName: string) => void;
  onZoneContextMenu?: (playerId: number, zoneName: string, event: React.MouseEvent) => void;
  onRequestCreateCounter?: () => void;
  onPlayerContextMenu?: (event: React.MouseEvent) => void;
}

function PlayerBoard({
  gameId,
  playerId,
  mirrored = false,
  canAct = false,
  canEditCounters = false,
  arrowSourceKey = null,
  onCardHover,
  onCardClick,
  onCardContextMenu,
  onCardDoubleClick,
  onZoneClick,
  onZoneContextMenu,
  onRequestCreateCounter,
  onPlayerContextMenu,
}: PlayerBoardProps) {
  return (
    <div
      className={cx('player-board', { 'player-board--mirrored': mirrored })}
      data-testid={`player-board-${playerId}`}
    >
      <PlayerInfoPanel
        gameId={gameId}
        playerId={playerId}
        canEdit={canEditCounters}
        onRequestCreateCounter={onRequestCreateCounter}
        onContextMenu={onPlayerContextMenu}
      />
      <Battlefield
        gameId={gameId}
        playerId={playerId}
        mirrored={mirrored}
        canAct={canAct}
        arrowSourceKey={arrowSourceKey}
        onCardHover={onCardHover}
        onCardClick={onCardClick}
        onCardContextMenu={onCardContextMenu}
        onCardDoubleClick={onCardDoubleClick}
      />
      <ZoneRail
        gameId={gameId}
        playerId={playerId}
        onCardHover={onCardHover}
        onZoneClick={onZoneClick}
        onZoneContextMenu={onZoneContextMenu}
      />
    </div>
  );
}

export default PlayerBoard;
