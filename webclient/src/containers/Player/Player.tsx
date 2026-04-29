import { useMemo } from 'react';
import { useTranslation } from 'react-i18next';
import Button from '@mui/material/Button';
import Paper from '@mui/material/Paper';
import Typography from '@mui/material/Typography';

import Layout from '../Layout/Layout';
import { AuthGuard } from '@app/components';
import { Images } from '@app/images';
import { Data } from '@app/types';

import { usePlayer } from './usePlayer';

import './Player.css';

const AVATAR_DATA_URI_PREFIX = 'data:image/png;base64,';

/** Builds a data URI from a raw avatar payload, or returns null when no avatar is set. */
function avatarSrc(bmp: Uint8Array | undefined): string | null {
  if (!bmp || bmp.byteLength === 0) {
    return null;
  }
  let binary = '';
  for (let i = 0; i < bmp.byteLength; i += 1) {
    binary += String.fromCharCode(bmp[i]);
  }
  return AVATAR_DATA_URI_PREFIX + btoa(binary);
}

/** Matches desktop UserInfoBox level hierarchy: admin > moderator > registered > unregistered, plus Judge marker. */
function userLevelLabel(userLevel: number, t: (k: string) => string): string {
  const Flag = Data.ServerInfo_User_UserLevelFlag;
  const parts: string[] = [];
  if ((userLevel & Flag.IsAdmin) === Flag.IsAdmin) {
    parts.push(t('Player.level.administrator'));
  } else if ((userLevel & Flag.IsModerator) === Flag.IsModerator) {
    parts.push(t('Player.level.moderator'));
  } else if ((userLevel & Flag.IsRegistered) === Flag.IsRegistered) {
    parts.push(t('Player.level.registered'));
  } else {
    parts.push(t('Player.level.unregistered'));
  }
  if ((userLevel & Flag.IsJudge) === Flag.IsJudge) {
    parts.push(t('Player.level.judge'));
  }
  return parts.join(' | ');
}

/** Formats account age like desktop's getAgeString: "Unknown" | "N days" | "Y years, D days". */
function formatAccountAge(
  accountageSecs: bigint | undefined,
  userLevel: number,
  t: (k: string, params?: Record<string, unknown>) => string,
): string {
  const Flag = Data.ServerInfo_User_UserLevelFlag;
  const isRegistered =
    (userLevel & Flag.IsAdmin) === Flag.IsAdmin ||
    (userLevel & Flag.IsModerator) === Flag.IsModerator ||
    (userLevel & Flag.IsRegistered) === Flag.IsRegistered;
  if (!isRegistered) {
    return t('Player.level.unregistered');
  }
  if (!accountageSecs || accountageSecs <= 0n) {
    return t('Player.age.unknown');
  }
  const totalDays = Number(accountageSecs / 86400n);
  const years = Math.floor(totalDays / 365);
  const days = totalDays - years * 365;
  if (years > 0) {
    return t('Player.age.daysWithYears', { years, days });
  }
  return t('Player.age.days', { count: days });
}

const Player = () => {
  const { t } = useTranslation();
  const {
    name,
    userInfo,
    isSelf,
    isABuddy,
    isIgnored,
    isModerator,
    onAddBuddy,
    onRemoveBuddy,
    onAddIgnore,
    onRemoveIgnore,
    onSendMessage,
    onWarnUser,
    onBanFromServer,
  } = usePlayer();

  const avatar = useMemo(() => avatarSrc(userInfo?.avatarBmp), [userInfo?.avatarBmp]);
  const countryCode = userInfo?.country?.toUpperCase() ?? '';

  return (
    <Layout>
      <AuthGuard />
      <div className="player-view">
        <Paper className="player-view__card" elevation={2}>
          <Typography variant="h5" className="player-view__name">
            {t('Player.title')}
          </Typography>

          {!userInfo && (
            <Typography className="player-view__empty">{t('Player.action.notFound')}</Typography>
          )}

          {userInfo && (
            <>
              <div className="player-view__avatar-wrapper">
                {avatar
                  ? <img className="player-view__avatar" src={avatar} alt={name ?? ''} />
                  : <div className="player-view__avatar" aria-hidden="true" />}
              </div>

              <Typography variant="h6" className="player-view__name">{userInfo.name}</Typography>
              <Typography className="player-view__level-badge">
                {userLevelLabel(userInfo.userLevel, t)}
                {userInfo.privlevel && userInfo.privlevel !== 'NONE' ? ` | ${userInfo.privlevel}` : ''}
              </Typography>

              <div className="player-view__details">
                <span className="player-view__label">{t('Player.label.realName')}</span>
                <span>{userInfo.realName || '—'}</span>

                <span className="player-view__label">{t('Player.label.location')}</span>
                <span>
                  {countryCode && (
                    <img
                      className="player-view__country-flag"
                      src={Images.Countries[userInfo.country]}
                      alt={countryCode}
                    />
                  )}
                  {countryCode || '—'}
                </span>

                <span className="player-view__label">{t('Player.label.userLevel')}</span>
                <span>{userLevelLabel(userInfo.userLevel, t)}</span>

                <span className="player-view__label">{t('Player.label.accountAge')}</span>
                <span>{formatAccountAge(userInfo.accountageSecs, userInfo.userLevel, t)}</span>
              </div>

              {!isSelf && (
                <div className="player-view__actions">
                  <Button variant="outlined" onClick={isABuddy ? onRemoveBuddy : onAddBuddy}>
                    {isABuddy ? t('Player.action.removeBuddy') : t('Player.action.addBuddy')}
                  </Button>
                  <Button variant="outlined" onClick={isIgnored ? onRemoveIgnore : onAddIgnore}>
                    {isIgnored ? t('Player.action.removeIgnore') : t('Player.action.addIgnore')}
                  </Button>
                  <Button variant="outlined" onClick={() => onSendMessage('')}>
                    {t('Player.action.message')}
                  </Button>
                  {isModerator && (
                    <>
                      <Button variant="outlined" color="warning" onClick={() => onWarnUser('')}>
                        {t('Player.action.warn')}
                      </Button>
                      <Button variant="outlined" color="error" onClick={() => onBanFromServer(0, '', '')}>
                        {t('Player.action.ban')}
                      </Button>
                    </>
                  )}
                </div>
              )}
            </>
          )}
        </Paper>
      </div>
    </Layout>
  );
};

export default Player;
