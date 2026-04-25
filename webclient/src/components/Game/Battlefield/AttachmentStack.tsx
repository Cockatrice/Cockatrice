import { App, Data } from '@app/types';

import CardSlot from '../CardSlot/CardSlot';
import { makeCardKey } from '../CardRegistry/CardRegistryContext';

import './AttachmentStack.css';

/**
 * Fraction of a card's width that each attached child peeks past its
 * predecessor. 0.3 matches the desktop Cockatrice aura fan — roughly 70%
 * overlap between adjacent cards.
 */
const OFFSET_FRACTION = 0.3;

export interface AttachmentStackProps {
  parent: Data.ServerInfo_Card;
  attachments: Data.ServerInfo_Card[];
  draggable: boolean;
  ownerPlayerId: number;
  arrowSourceKey: string | null;
  onCardHover?: (card: Data.ServerInfo_Card) => void;
  onCardClick?: (playerId: number, zone: string, card: Data.ServerInfo_Card) => void;
  onCardContextMenu?: (card: Data.ServerInfo_Card, event: React.MouseEvent) => void;
  onCardDoubleClick?: (card: Data.ServerInfo_Card) => void;
}

function AttachmentStack({
  parent,
  attachments,
  draggable,
  ownerPlayerId,
  arrowSourceKey,
  onCardHover,
  onCardClick,
  onCardContextMenu,
  onCardDoubleClick,
}: AttachmentStackProps) {
  const parentKey = makeCardKey(ownerPlayerId, App.ZoneName.TABLE, parent.id);

  // Stack footprint in units of one card width. N attachments → 1 + 0.3·N
  // slots wide; every card shares width = 1 / stackFactor of the stack.
  // The parent slot (BattlefieldStackColumn__slot) already sizes itself to
  // the full stack footprint as a percentage of the stack column, so here we
  // just fill the slot and lay the parent + attachments out as percentages.
  const stackFactor = 1 + attachments.length * OFFSET_FRACTION;
  // Round to hundredths — otherwise 1/1.6 yields 0.625 and computed offsets
  // carry float artifacts into sub-pixel rendering and test assertions.
  const round = (n: number) => Math.round(n * 100) / 100;
  const cardWidthPct = round(100 / stackFactor);

  return (
    <div className="attachment-stack">
      <div
        className="attachment-stack__parent"
        style={{ width: `${cardWidthPct}%` }}
      >
        <CardSlot
          card={parent}
          draggable={draggable}
          ownerPlayerId={ownerPlayerId}
          zone={App.ZoneName.TABLE}
          isArrowSource={arrowSourceKey === parentKey}
          onMouseEnter={onCardHover}
          onClick={(c) => onCardClick?.(ownerPlayerId, App.ZoneName.TABLE, c)}
          onContextMenu={onCardContextMenu}
          onDoubleClick={onCardDoubleClick}
        />
      </div>
      {attachments.map((child, i) => {
        const childKey = makeCardKey(ownerPlayerId, App.ZoneName.TABLE, child.id);
        const leftPct = round(((i + 1) * OFFSET_FRACTION * 100) / stackFactor);
        return (
          <div
            key={child.id}
            className="attachment-stack__child"
            style={{
              left: `${leftPct}%`,
              width: `${cardWidthPct}%`,
              zIndex: i + 1,
            }}
          >
            <CardSlot
              card={child}
              draggable={draggable}
              ownerPlayerId={ownerPlayerId}
              zone={App.ZoneName.TABLE}
              isArrowSource={arrowSourceKey === childKey}
              onMouseEnter={onCardHover}
              onClick={(c) => onCardClick?.(ownerPlayerId, App.ZoneName.TABLE, c)}
              onContextMenu={onCardContextMenu}
              onDoubleClick={onCardDoubleClick}
            />
          </div>
        );
      })}
    </div>
  );
}

export default AttachmentStack;
