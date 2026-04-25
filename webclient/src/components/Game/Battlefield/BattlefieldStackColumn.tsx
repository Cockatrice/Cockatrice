import { Data } from '@app/types';

import AttachmentStack from './AttachmentStack';
import {
  CARD_HEIGHT_PX,
  CARD_WIDTH_PX,
  STACKED_CARD_OFFSET_X_PX,
} from './gridMath';

import './BattlefieldStackColumn.css';

// Keep in sync with AttachmentStack.OFFSET_FRACTION. Attachments peek 30% of a
// card width past their predecessor; a parent with N attachments occupies
// CARD_WIDTH_PX × (1 + N × 0.3) horizontally.
const ATTACH_OFFSET_FRACTION = 0.3;

const EMPTY_ATTACHMENTS: Data.ServerInfo_Card[] = [];

export interface BattlefieldStackColumnProps {
  cards: Data.ServerInfo_Card[]; // 1..MAX_SUBPOS cards, sorted by sub-position
  attachmentsByParent: ReadonlyMap<number, Data.ServerInfo_Card[]>;
  draggable: boolean;
  ownerPlayerId: number;
  arrowSourceKey: string | null;
  onCardHover?: (card: Data.ServerInfo_Card) => void;
  onCardClick?: (playerId: number, zone: string, card: Data.ServerInfo_Card) => void;
  onCardContextMenu?: (card: Data.ServerInfo_Card, event: React.MouseEvent) => void;
  onCardDoubleClick?: (card: Data.ServerInfo_Card) => void;
}

// Width of a stack column = right-edge extent of its widest-positioned card.
// Each card sits at left = subPos × OFFSET; a card with K attachments extends
// rightward to (subPos × OFFSET) + CARD_WIDTH × (1 + K × 0.3). We take the max
// across all cards so attachments on an early sub-position don't overlap the
// neighbor stack.
function computeStackWidth(
  cards: Data.ServerInfo_Card[],
  attachmentsByParent: ReadonlyMap<number, Data.ServerInfo_Card[]>,
): number {
  let maxRight = CARD_WIDTH_PX;
  cards.forEach((card, subPos) => {
    const attachCount = attachmentsByParent.get(card.id)?.length ?? 0;
    const cardWidth = CARD_WIDTH_PX * (1 + attachCount * ATTACH_OFFSET_FRACTION);
    const leftOffset = subPos * STACKED_CARD_OFFSET_X_PX;
    maxRight = Math.max(maxRight, leftOffset + cardWidth);
  });
  return Math.round(maxRight * 100) / 100;
}

function slotWidthFor(card: Data.ServerInfo_Card, attachmentsByParent: ReadonlyMap<number, Data.ServerInfo_Card[]>): number {
  const attachCount = attachmentsByParent.get(card.id)?.length ?? 0;
  return CARD_WIDTH_PX * (1 + attachCount * ATTACH_OFFSET_FRACTION);
}

function BattlefieldStackColumn({
  cards,
  attachmentsByParent,
  draggable,
  ownerPlayerId,
  arrowSourceKey,
  onCardHover,
  onCardClick,
  onCardContextMenu,
  onCardDoubleClick,
}: BattlefieldStackColumnProps) {
  const widthPx = computeStackWidth(cards, attachmentsByParent);

  // Stack column scales with lane height via aspect-ratio. Its rendered
  // width = laneHeight × widthPx / CARD_HEIGHT_PX; per-slot left/width are
  // expressed as percentages of that, so positions stay proportional at any
  // zoom level. widthPx is the nominal (146/49/ratio) footprint.
  const round = (n: number) => Math.round(n * 100) / 100;

  return (
    <div
      className="battlefield-stack-column"
      data-testid="battlefield-stack-column"
      style={{ aspectRatio: `${widthPx} / ${CARD_HEIGHT_PX}` }}
    >
      {cards.map((card, subPos) => {
        const slotWidth = slotWidthFor(card, attachmentsByParent);
        const leftPct = round((subPos * STACKED_CARD_OFFSET_X_PX * 100) / widthPx);
        const widthPct = round((slotWidth * 100) / widthPx);
        return (
          <div
            key={card.id}
            className="battlefield-stack-column__slot"
            data-sub-position={subPos}
            style={{
              left: `${leftPct}%`,
              width: `${widthPct}%`,
              // Later sub-positions render on top of earlier ones, matching
              // desktop's paint order where cards added later overlay neighbors.
              zIndex: subPos + 1,
            }}
          >
            <AttachmentStack
              parent={card}
              attachments={attachmentsByParent.get(card.id) ?? EMPTY_ATTACHMENTS}
              draggable={draggable}
              ownerPlayerId={ownerPlayerId}
              arrowSourceKey={arrowSourceKey}
              onCardHover={onCardHover}
              onCardClick={onCardClick}
              onCardContextMenu={onCardContextMenu}
              onCardDoubleClick={onCardDoubleClick}
            />
          </div>
        );
      })}
    </div>
  );
}

export default BattlefieldStackColumn;
