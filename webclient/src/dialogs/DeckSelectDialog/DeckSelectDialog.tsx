import { useRef } from 'react';
import { styled } from '@mui/material/styles';
import Dialog from '@mui/material/Dialog';
import DialogContent from '@mui/material/DialogContent';
import DialogTitle from '@mui/material/DialogTitle';
import Typography from '@mui/material/Typography';
import Button from '@mui/material/Button';

import { useDeckSelectDialog } from './useDeckSelectDialog';

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
  const {
    deckText,
    setDeckText,
    fileName,
    handleFilePicked,
    validationError,
    deckHash,
    isReady,
    canSubmit,
    canToggleReady,
    handleSubmitDeck,
    handleToggleReady,
  } = useDeckSelectDialog(gameId);

  const fileInputRef = useRef<HTMLInputElement | null>(null);

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
          Pick a .cod file from your computer or paste its XML below, then click
          Submit Deck. After the server accepts the deck, the Ready button unlocks.
        </Typography>

        <div className="deck-select-dialog__file-row">
          <input
            ref={fileInputRef}
            type="file"
            accept=".cod"
            aria-label="deck file"
            style={{ display: 'none' }}
            onChange={(e) => {
              const file = e.target.files?.[0] ?? null;
              handleFilePicked(file);
              e.target.value = '';
            }}
          />
          <Button
            variant="outlined"
            size="small"
            onClick={() => fileInputRef.current?.click()}
          >
            Choose .cod file
          </Button>
          <span className="deck-select-dialog__file-name">
            {fileName ?? 'No file selected'}
          </span>
        </div>

        <Typography className="deck-select-dialog__divider" variant="caption">
          — or paste XML below —
        </Typography>

        <textarea
          className="deck-select-dialog__textarea"
          rows={10}
          value={deckText}
          onChange={(e) => setDeckText(e.target.value)}
          placeholder={'<?xml version="1.0"?>\n<cockatrice_deck version="1">\n  ...\n</cockatrice_deck>'}
          aria-label="deck list"
        />

        {validationError != null && (
          <div className="deck-select-dialog__error" role="alert">
            {validationError}
          </div>
        )}

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
