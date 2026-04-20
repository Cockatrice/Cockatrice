import ListItemButton from '@mui/material/ListItemButton';
import Paper from '@mui/material/Paper';

import { ScrollToBottomOnChanges, ThreePaneLayout, UserDisplay, VirtualList, AuthGuard } from '@app/components';
import Layout from '../Layout/Layout';

import GameSelector from './GameSelector/GameSelector';
import Messages from './Messages';
import SayMessage from './SayMessage';
import { useRoom } from './useRoom';

import './Room.css';

const Room = () => {
  const { room, roomMessages, users, handleRoomSay } = useRoom();

  if (!room) {
    return null;
  }

  return (
    <Layout className="room-view">
      <AuthGuard />

      <div className="room-view__main">
        <ThreePaneLayout
          fixedHeight

          top={(
            <div className="room-view__games">
              <GameSelector room={room} />
            </div>
          )}

          bottom={(
            <div className="room-view__messages">
              <Paper className="room-view__messages-content overflow-scroll">
                <ScrollToBottomOnChanges changes={roomMessages} content={(
                  <Messages messages={roomMessages} />
                )} />
              </Paper>
              <Paper className="room-view__messages-sayMessage">
                <SayMessage onSubmit={handleRoomSay} />
              </Paper>
            </div>
          )}

          side={(
            <Paper className="room-view__side overflow-scroll">
              <div className="room-view__side-label">
                Users in this room: {users.length}
              </div>
              <VirtualList
                className="room-view__side-list"
                items={users.map(user => (
                  <ListItemButton key={user.name} className="room-view__side-list__item">
                    <UserDisplay user={user} />
                  </ListItemButton>
                ))}
              />
            </Paper>
          )}
        />
      </div>
    </Layout>
  );
};

export default Room;
