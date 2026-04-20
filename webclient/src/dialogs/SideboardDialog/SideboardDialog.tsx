import { useEffect, useMemo, useState } from 'react';
import { styled } from '@mui/material/styles';
import Dialog from '@mui/material/Dialog';
import DialogActions from '@mui/material/DialogActions';
import DialogContent from '@mui/material/DialogContent';
import DialogTitle from '@mui/material/DialogTitle';
import Typography from '@mui/material/Typography';
import Button from '@mui/material/Button';
import FormControlLabel from '@mui/material/FormControlLabel';
import Checkbox from '@mui/material/Checkbox';

import { App, Enriched } from '@app/types';

import './SideboardDialog.css';

const PREFIX = 'SideboardDialog';

const classes = {
  root: `${PREFIX}-root`,
};

const StyledDialog = styled(Dialog)(({ theme }) => ({
  [`&.${classes.root}`]: {
    '& .dialog-title__wrapper': {
      borderColor: theme.palette.grey[300],
    },
  },
}));

export interface SideboardPlanMove {
  cardName: string;
  startZone: string;
  targetZone: string;
}

export interface SideboardDialogProps {
  isOpen: boolean;
  playerName: string;
  /** Snapshot of the player's deck zone at open time (identified by card.name). */
  deckCards: ReadonlyArray<{ id: number; name: string }>;
  /** Snapshot of the player's sideboard zone at open time. */
  sideboardCards: ReadonlyArray<{ id: number; name: string }>;
  /** Current persisted lock state from ServerInfo_PlayerProperties.sideboardLocked. */
  isLocked: boolean;
  onSubmit: (moveList: SideboardPlanMove[]) => void;
  onCancel: () => void;
  onToggleLock: (next: boolean) => void;
}

type Card = { id: number; name: string };

// Applies a draft move sequence to the initial lists to derive the
// currently-displayed deck and sideboard. Mirrors desktop's
// DeckView::applyPlan logic (identify-by-name, one copy per entry).
function applyMoves(
  initialDeck: ReadonlyArray<Card>,
  initialSideboard: ReadonlyArray<Card>,
  moves: ReadonlyArray<SideboardPlanMove>,
): { deck: Card[]; sideboard: Card[] } {
  const deck = [...initialDeck];
  const sideboard = [...initialSideboard];
  for (const move of moves) {
    const from = move.startZone === App.ZoneName.DECK ? deck : sideboard;
    const to = move.targetZone === App.ZoneName.DECK ? deck : sideboard;
    const idx = from.findIndex((c) => c.name === move.cardName);
    if (idx < 0) {
      continue;
    }
    const [card] = from.splice(idx, 1);
    to.push(card);
  }
  return { deck, sideboard };
}

function SideboardDialog({
  isOpen,
  playerName,
  deckCards,
  sideboardCards,
  isLocked,
  onSubmit,
  onCancel,
  onToggleLock,
}: SideboardDialogProps) {
  const [moves, setMoves] = useState<SideboardPlanMove[]>([]);

  // Reset the draft every time the dialog opens, and also when the server
  // locks the sideboard mid-edit (desktop's resetSideboardPlan parity).
  useEffect(() => {
    if (isOpen) {
      setMoves([]);
    }
  }, [isOpen]);

  useEffect(() => {
    if (isLocked && moves.length > 0) {
      setMoves([]);
    }
  }, [isLocked, moves.length]);

  const { deck, sideboard } = useMemo(
    () => applyMoves(deckCards, sideboardCards, moves),
    [deckCards, sideboardCards, moves],
  );

  const addMove = (cardName: string, startZone: string, targetZone: string) => {
    setMoves((prev) => [...prev, { cardName, startZone, targetZone }]);
  };

  const handleMoveToSideboard = (card: Card) => {
    if (isLocked) {
      return;
    }
    addMove(card.name, App.ZoneName.DECK, App.ZoneName.SIDEBOARD);
  };

  const handleMoveToDeck = (card: Card) => {
    if (isLocked) {
      return;
    }
    addMove(card.name, App.ZoneName.SIDEBOARD, App.ZoneName.DECK);
  };

  const handleApply = () => {
    onSubmit(moves);
  };

  return (
    <StyledDialog
      className={'SideboardDialog ' + classes.root}
      open={isOpen}
      onClose={onCancel}
      maxWidth={false}
    >
      <DialogTitle className="dialog-title">
        <div className="dialog-title__wrapper">
          <Typography variant="h2">Sideboard — {playerName}</Typography>
          <FormControlLabel
            control={
              <Checkbox
                checked={isLocked}
                onChange={(e) => onToggleLock(e.target.checked)}
                slotProps={{ input: { 'aria-label': 'Lock sideboard' } }}
              />
            }
            label="Lock sideboard"
            className="sideboard-dialog__lock"
          />
        </div>
      </DialogTitle>
      <DialogContent className="dialog-content sideboard-dialog__body">
        {isLocked && (
          <div className="sideboard-dialog__locked-note" role="note">
            The sideboard is locked. Unlock to change your plan.
          </div>
        )}
        <div className="sideboard-dialog__columns">
          <section
            className="sideboard-dialog__column"
            aria-label={`Main deck (${deck.length})`}
          >
            <h3 className="sideboard-dialog__column-heading">
              Main deck ({deck.length})
            </h3>
            <ul className="sideboard-dialog__list" data-testid="sideboard-dialog-deck">
              {deck.map((card, idx) => (
                <li key={`${card.id}-${idx}`} className="sideboard-dialog__row">
                  <span className="sideboard-dialog__name">{card.name}</span>
                  <Button
                    type="button"
                    size="small"
                    onClick={() => handleMoveToSideboard(card)}
                    disabled={isLocked}
                    aria-label={`Move ${card.name} to sideboard`}
                  >
                    →
                  </Button>
                </li>
              ))}
              {deck.length === 0 && (
                <li className="sideboard-dialog__empty">(empty)</li>
              )}
            </ul>
          </section>
          <section
            className="sideboard-dialog__column"
            aria-label={`Sideboard (${sideboard.length})`}
          >
            <h3 className="sideboard-dialog__column-heading">
              Sideboard ({sideboard.length})
            </h3>
            <ul className="sideboard-dialog__list" data-testid="sideboard-dialog-sb">
              {sideboard.map((card, idx) => (
                <li key={`${card.id}-${idx}`} className="sideboard-dialog__row">
                  <Button
                    type="button"
                    size="small"
                    onClick={() => handleMoveToDeck(card)}
                    disabled={isLocked}
                    aria-label={`Move ${card.name} to main deck`}
                  >
                    ←
                  </Button>
                  <span className="sideboard-dialog__name">{card.name}</span>
                </li>
              ))}
              {sideboard.length === 0 && (
                <li className="sideboard-dialog__empty">(empty)</li>
              )}
            </ul>
          </section>
        </div>
      </DialogContent>
      <DialogActions>
        <Button type="button" onClick={onCancel}>Cancel</Button>
        <Button
          type="button"
          variant="contained"
          color="primary"
          onClick={handleApply}
          disabled={isLocked || moves.length === 0}
        >
          Apply plan{moves.length > 0 ? ` (${moves.length})` : ''}
        </Button>
      </DialogActions>
    </StyledDialog>
  );
}

export default SideboardDialog;

// Exported for tests and the Game.tsx wiring layer.
export { applyMoves };

// Helper to derive the card-display arrays a parent needs to pass.
// Takes the normalized Enriched.ZoneEntry and materializes the
// [{id, name}] shape the dialog expects.
export function cardsFromZone(
  zone: Enriched.ZoneEntry | undefined,
): Card[] {
  if (!zone) {
    return [];
  }
  return zone.order.map((id) => ({ id, name: zone.byId[id]?.name ?? '' }));
}
