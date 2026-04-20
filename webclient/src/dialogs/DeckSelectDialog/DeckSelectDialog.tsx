import { useState } from 'react';
import { styled } from '@mui/material/styles';
import Dialog from '@mui/material/Dialog';
import DialogContent from '@mui/material/DialogContent';
import DialogTitle from '@mui/material/DialogTitle';
import Typography from '@mui/material/Typography';
import Button from '@mui/material/Button';

import { useWebClient } from '@app/hooks';
import { GameSelectors, useAppSelector } from '@app/store';

import './DeckSelectDialog.css';

const PREFIX = 'DeckSelectDialog';

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

export interface DeckSelectDialogProps {
  isOpen: boolean;
  gameId: number | undefined;
  /**
   * Unlike ZoneViewDialog (which takes a required `handleClose`), the
   * deck-select dialog is auto-gated by the game's pre-ready lobby state:
   * it opens when `!game.started && !spectator && !judge && !readyStart`
   * and closes when any of those flip. Tests pass a no-op; production
   * callers typically omit this prop, letting MUI render a non-dismissable
   * modal (no backdrop-click or ESC close).
   */
  handleClose?: () => void;
}

function DeckSelectDialog({ isOpen, gameId, handleClose }: DeckSelectDialogProps) {
  const webClient = useWebClient();
  const localPlayer = useAppSelector((state) =>
    gameId != null ? GameSelectors.getLocalPlayer(state, gameId) : undefined,
  );
  const [deckText, setDeckText] = useState('');

  const deckHash = localPlayer?.properties.deckHash ?? '';
  const isReady = localPlayer?.properties.readyStart ?? false;
  const hasLocalPlayer = localPlayer != null;
  // Guard Submit/Ready on having a local player — today the deckSelectOpen
  // predicate in Game.tsx implies one, but the dialog mounts before the
  // Event_GameJoined echo populates players during reconnect.
  const canSubmit = hasLocalPlayer && deckText.trim().length > 0;
  const canToggleReady = hasLocalPlayer && deckHash.length > 0;

  const handleSubmitDeck = () => {
    if (!canSubmit || gameId == null) {
      return;
    }
    webClient.request.game.deckSelect(gameId, { deck: deckText.trim() });
  };

  const handleToggleReady = () => {
    if (!canToggleReady || gameId == null) {
      return;
    }
    webClient.request.game.readyStart(gameId, { ready: !isReady });
  };

  return (
    <StyledDialog
      className={'DeckSelectDialog ' + classes.root}
      open={isOpen}
      onClose={handleClose}
      maxWidth={false}
    >
      <DialogTitle className="dialog-title">
        <div className="dialog-title__wrapper">
          <Typography variant="h2">Select Deck</Typography>
        </div>
      </DialogTitle>
      <DialogContent className="dialog-content">
        <Typography className="dialog-content__subtitle" variant="subtitle1">
          Paste your deck list below, then click Submit Deck. After the server
          accepts the deck, the Ready button unlocks.
        </Typography>

        <textarea
          className="deck-select-dialog__textarea"
          rows={10}
          value={deckText}
          onChange={(e) => setDeckText(e.target.value)}
          placeholder="4 Lightning Bolt&#10;20 Mountain&#10;..."
          aria-label="deck list"
        />

        <div className="deck-select-dialog__hash">
          Deck hash: {deckHash.length > 0 ? deckHash : <span className="deck-select-dialog__hash--pending">—</span>}
        </div>

        <div className="deck-select-dialog__actions">
          <Button
            variant="outlined"
            onClick={handleSubmitDeck}
            disabled={!canSubmit}
          >
            Submit Deck
          </Button>
          <Button
            variant="contained"
            color="primary"
            onClick={handleToggleReady}
            disabled={!canToggleReady}
          >
            {isReady ? 'Unready' : 'Ready'}
          </Button>
        </div>
      </DialogContent>
    </StyledDialog>
  );
}

export default DeckSelectDialog;
