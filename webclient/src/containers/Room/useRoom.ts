import { useEffect } from 'react';
import { useNavigate, useParams, generatePath } from 'react-router-dom';

import { useWebClient } from '@app/hooks';
import { RoomsSelectors, useAppSelector } from '@app/store';
import { App } from '@app/types';

export interface Room {
  roomId: number;
  room: any;
  roomMessages: any;
  users: any[];
  handleRoomSay: (args: { message: string }) => void;
}

export function useRoom(): Room {
  const joined = useAppSelector((state) => RoomsSelectors.getJoinedRooms(state));
  const rooms = useAppSelector((state) => RoomsSelectors.getRooms(state));
  const messages = useAppSelector((state) => RoomsSelectors.getMessages(state));
  const navigate = useNavigate();
  const params = useParams();

  const roomId = parseInt(params.roomId, 10);
  const room = rooms[roomId];
  const roomMessages = messages[roomId];
  const users = useAppSelector((state) => RoomsSelectors.getSortedRoomUsers(state, roomId));
  const webClient = useWebClient();

  useEffect(() => {
    if (!joined.find((r) => r.info.roomId === roomId)) {
      navigate(generatePath(App.RouteEnum.SERVER));
    }
  }, [joined]);

  const handleRoomSay = ({ message }: { message: string }) => {
    if (message) {
      webClient.request.rooms.roomSay(roomId, message);
    }
  };

  return { roomId, room, roomMessages, users, handleRoomSay };
}
