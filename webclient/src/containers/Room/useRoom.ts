import { useCallback, useEffect } from 'react';
import { useNavigate, useParams, generatePath } from 'react-router-dom';

import { useWebClient } from '@app/hooks';
import { RoomsSelectors, useAppSelector } from '@app/store';
import { App, Data, Enriched } from '@app/types';

export interface Room {
  roomId: number;
  room: Enriched.Room | undefined;
  roomMessages: Enriched.Message[] | undefined;
  users: Data.ServerInfo_User[];
  handleRoomSay: (args: { message: string }) => void;
}

export function useRoom(): Room {
  const joined = useAppSelector((state) => RoomsSelectors.getJoinedRooms(state));
  const rooms = useAppSelector((state) => RoomsSelectors.getRooms(state));
  const messages = useAppSelector((state) => RoomsSelectors.getMessages(state));
  const navigate = useNavigate();
  const params = useParams();

  const parsed = params.roomId != null ? parseInt(params.roomId, 10) : NaN;
  const roomId = Number.isNaN(parsed) ? -1 : parsed;
  const room = roomId === -1 ? undefined : rooms[roomId];
  const roomMessages = roomId === -1 ? undefined : messages[roomId];
  const users = useAppSelector((state) => RoomsSelectors.getSortedRoomUsers(state, roomId));
  const webClient = useWebClient();

  useEffect(() => {
    if (roomId === -1 || !joined.find((r) => r.info.roomId === roomId)) {
      navigate(generatePath(App.RouteEnum.SERVER));
    }
  }, [joined, roomId, navigate]);

  const handleRoomSay = useCallback(({ message }: { message: string }) => {
    if (message) {
      webClient.request.rooms.roomSay(roomId, message);
    }
  }, [webClient, roomId]);

  return { roomId, room, roomMessages, users, handleRoomSay };
}
