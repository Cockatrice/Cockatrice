import { GameSelectors, useAppSelector } from '@app/store';
import { cx } from '@app/utils';

import './PlayerList.css';

export interface PlayerListProps {
  gameId: number | undefined;
}

function PlayerList({ gameId }: PlayerListProps) {
  const players = useAppSelector((state) =>
    gameId != null ? GameSelectors.getPlayers(state, gameId) : undefined,
  );
  const activePlayerId = useAppSelector((state) =>
    gameId != null ? GameSelectors.getActivePlayerId(state, gameId) : undefined,
  );
  const hostId = useAppSelector((state) =>
    gameId != null ? GameSelectors.getHostId(state, gameId) : undefined,
  );

  const entries = players ? Object.values(players) : [];

  return (
    <div className="player-list" data-testid="player-list">
      <div className="player-list__heading">Players</div>
      <ul className="player-list__items">
        {entries.length === 0 && (
          <li className="player-list__empty">no players</li>
        )}
        {entries.map((p) => {
          const pid = p.properties.playerId;
          const name = p.properties.userInfo?.name ?? '(unknown)';
          const isActive = pid === activePlayerId;
          const isHost = pid === hostId;
          return (
            <li
              key={pid}
              className={cx('player-list__item', {
                'player-list__item--active': isActive,
                'player-list__item--conceded': p.properties.conceded,
              })}
              data-testid={`player-list-item-${pid}`}
            >
              <span
                className={cx('player-list__indicator', {
                  'player-list__indicator--active': isActive,
                })}
              />
              {isHost && (
                <span
                  className="player-list__host-badge"
                  aria-label="host"
                  title="Host"
                >
                  ♛
                </span>
              )}
              <span className="player-list__name">{name}</span>
              <span className="player-list__ping">{p.properties.pingSeconds}s</span>
            </li>
          );
        })}
      </ul>
    </div>
  );
}

export default PlayerList;
