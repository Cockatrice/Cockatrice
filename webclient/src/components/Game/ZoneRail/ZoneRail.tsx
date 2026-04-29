import { App, Data } from '@app/types';

import ZoneStack from '../ZoneStack/ZoneStack';

import './ZoneRail.css';

export interface ZoneRailProps {
  gameId: number;
  playerId: number;
  onCardHover?: (card: Data.ServerInfo_Card) => void;
  onZoneClick?: (playerId: number, zoneName: string) => void;
  onZoneContextMenu?: (playerId: number, zoneName: string, event: React.MouseEvent) => void;
}

const ZONES: Array<{ name: string; label: string }> = [
  { name: App.ZoneName.DECK, label: 'Deck' },
  { name: App.ZoneName.GRAVE, label: 'Graveyard' },
  { name: App.ZoneName.EXILE, label: 'Exile' },
];

function ZoneRail({
  gameId,
  playerId,
  onCardHover,
  onZoneClick,
  onZoneContextMenu,
}: ZoneRailProps) {
  return (
    <div className="zone-rail" data-testid="zone-rail">
      {ZONES.map((z) => (
        <ZoneStack
          key={z.name}
          gameId={gameId}
          playerId={playerId}
          zoneName={z.name}
          label={z.label}
          onCardHover={onCardHover}
          onClick={onZoneClick ? (name) => onZoneClick(playerId, name) : undefined}
          onContextMenu={
            onZoneContextMenu
              ? (name, e) => onZoneContextMenu(playerId, name, e)
              : undefined
          }
        />
      ))}
    </div>
  );
}

export default ZoneRail;
