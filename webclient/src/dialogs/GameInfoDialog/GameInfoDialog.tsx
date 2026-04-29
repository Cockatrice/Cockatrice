import { styled } from '@mui/material/styles';
import Dialog from '@mui/material/Dialog';
import DialogActions from '@mui/material/DialogActions';
import DialogContent from '@mui/material/DialogContent';
import DialogTitle from '@mui/material/DialogTitle';
import IconButton from '@mui/material/IconButton';
import Typography from '@mui/material/Typography';
import Button from '@mui/material/Button';
import CloseIcon from '@mui/icons-material/Close';

import { GameSelectors, useAppSelector } from '@app/store';

import './GameInfoDialog.css';

const PREFIX = 'GameInfoDialog';

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

export interface GameInfoDialogProps {
  isOpen: boolean;
  gameId: number | undefined;
  onClose: () => void;
}

function formatElapsed(totalSeconds: number): string {
  const s = Math.max(0, Math.floor(totalSeconds));
  const hh = String(Math.floor(s / 3600)).padStart(2, '0');
  const mm = String(Math.floor((s % 3600) / 60)).padStart(2, '0');
  const ss = String(s % 60).padStart(2, '0');
  return `${hh}:${mm}:${ss}`;
}

/**
 * Read-only summary of the current game: id, description, elapsed time,
 * and a role-tagged player list. Mirrors desktop's Game Info popup on the
 * Player menu.
 */
function GameInfoDialog({ isOpen, gameId, onClose }: GameInfoDialogProps) {
  const game = useAppSelector((state) =>
    gameId != null ? GameSelectors.getGame(state, gameId) : undefined,
  );

  if (!game) {
    return null;
  }

  const description = game.info?.description ?? '';
  const name = game.info?.description ?? `Game ${game.info?.gameId ?? gameId ?? '—'}`;
  const players = Object.values(game.players);

  return (
    <StyledDialog
      className={'GameInfoDialog ' + classes.root}
      open={isOpen}
      onClose={onClose}
      maxWidth={false}
    >
      <DialogTitle className="dialog-title">
        <div className="dialog-title__wrapper">
          <Typography variant="h2">Game info</Typography>
          <IconButton onClick={onClose} size="large" aria-label="close game info">
            <CloseIcon fontSize="large" />
          </IconButton>
        </div>
      </DialogTitle>
      <DialogContent className="dialog-content game-info-dialog__body">
        <dl className="game-info-dialog__dl">
          <dt>Game ID</dt>
          <dd>{String(game.info?.gameId ?? gameId ?? '—')}</dd>
          <dt>Name</dt>
          <dd>{name || '(no description)'}</dd>
          <dt>Description</dt>
          <dd>{description || '(none)'}</dd>
          <dt>Started</dt>
          <dd>{game.started ? 'Yes' : 'No'}</dd>
          <dt>Elapsed</dt>
          <dd>{formatElapsed(game.secondsElapsed)}</dd>
          <dt>Host</dt>
          <dd>{
            players.find((p) => p.properties.playerId === game.hostId)
              ?.properties.userInfo?.name ?? `p${game.hostId}`
          }</dd>
        </dl>
        <hr />
        <Typography variant="h3" className="game-info-dialog__section">Players</Typography>
        <ul className="game-info-dialog__players">
          {players.map((p) => {
            const pid = p.properties.playerId;
            const pname = p.properties.userInfo?.name ?? `p${pid}`;
            const tags: string[] = [];
            if (pid === game.hostId) {
              tags.push('host');
            }
            if (p.properties.spectator) {
              tags.push('spectator');
            }
            if (p.properties.judge) {
              tags.push('judge');
            }
            if (pid === game.localPlayerId) {
              tags.push('you');
            }
            return (
              <li key={pid}>
                <span className="game-info-dialog__player-name">{pname}</span>
                {tags.length > 0 && (
                  <span className="game-info-dialog__player-tags">
                    {tags.map((t) => (
                      <span key={t} className="game-info-dialog__tag">{t}</span>
                    ))}
                  </span>
                )}
              </li>
            );
          })}
        </ul>
      </DialogContent>
      <DialogActions>
        <Button type="button" onClick={onClose} autoFocus>Close</Button>
      </DialogActions>
    </StyledDialog>
  );
}

export default GameInfoDialog;
