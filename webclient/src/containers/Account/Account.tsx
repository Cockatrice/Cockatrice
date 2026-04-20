import { useTranslation } from 'react-i18next';

import Button from '@mui/material/Button';
import ListItemButton from '@mui/material/ListItemButton';
import Paper from '@mui/material/Paper';

import { UserDisplay, VirtualList, AuthGuard, LanguageDropdown } from '@app/components';
import Layout from '../Layout/Layout';

import AddToBuddies from './AddToBuddies';
import AddToIgnore from './AddToIgnore';
import { useAccount } from './useAccount';

import './Account.css';

const Account = () => {
  const { t } = useTranslation();
  const {
    buddyList,
    ignoreList,
    serverName,
    serverVersion,
    user,
    avatarUrl,
    handleAddToBuddies,
    handleAddToIgnore,
    handleDisconnect,
  } = useAccount();
  const { country, realName, name, userLevel, accountageSecs } = user || {};

  return (
    <Layout className="account">
      <AuthGuard />
      <div className="account-column">
        <Paper className="account-list">
          <div>
            Buddies Online: ?/{buddyList.length}
          </div>
          <VirtualList
            items={buddyList.map(user => (
              <ListItemButton key={user.name} dense>
                <UserDisplay user={user} />
              </ListItemButton>
            ))}
          />
          <div style={{ borderTop: '1px solid' }}>
            <AddToBuddies onSubmit={handleAddToBuddies} />
          </div>
        </Paper>
      </div>
      <div className="account-column">
        <Paper className="account-list overflow-scroll">
          <div>
            Ignored Users Online: ?/{ignoreList.length}
          </div>
          <VirtualList
            items={ignoreList.map(user => (
              <ListItemButton key={user.name} dense>
                <UserDisplay user={user} />
              </ListItemButton>
            ))}
          />
          <div style={{ borderTop: '1px solid' }}>
            <AddToIgnore onSubmit={handleAddToIgnore} />
          </div>
        </Paper>
      </div>
      <div className="account-column overflow-scroll">
        <Paper className="account-details" style={{ margin: '0 0 5px 0' }}>
          {avatarUrl && <img src={avatarUrl} alt={name} />}
          <p><strong>{name}</strong></p>
          <p>Location: ({country?.toUpperCase()})</p>
          <p>User Level: {userLevel}</p>
          <p>Account Age: {String(accountageSecs)}</p>
          <p>Real Name: {realName}</p>
          <div className="account-details__actions">
            <Button size="small" color="primary" variant="contained">Edit</Button>
            <Button size="small" color="primary" variant="contained">Change<br />Password</Button>
            <Button size="small" color="primary" variant="contained">Change<br />Avatar</Button>
          </div>

        </Paper>
        <Paper className="account-details">
          <p>Server Name: {serverName}</p>
          <p>Server Version: {serverVersion}</p>
          <Button color="primary" variant="contained" onClick={handleDisconnect}>
            {t('Common.disconnect')}
          </Button>

          <div className="account-details__lang">
            <LanguageDropdown />
          </div>
        </Paper>
      </div>
    </Layout>
  );
};

export default Account;
