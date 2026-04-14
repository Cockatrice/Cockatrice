import { create } from '@bufbuild/protobuf';
import webClient from '../../WebClient';
import { Command_ReplayListSchema, Command_ReplayList_ext } from 'generated/proto/command_replay_list_pb';
import { SessionPersistence } from '../../persistence';
import { Response_ReplayList_ext } from 'generated/proto/response_replay_list_pb';

export function replayList(): void {
  webClient.protobuf.sendSessionCommand(Command_ReplayList_ext, create(Command_ReplayListSchema), {
    responseExt: Response_ReplayList_ext,
    onSuccess: (response) => {
      SessionPersistence.replayList(response.matchList);
    },
  });
}
