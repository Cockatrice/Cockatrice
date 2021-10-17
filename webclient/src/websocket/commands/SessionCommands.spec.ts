import { StatusEnum } from "types";

import { SessionCommands } from './SessionCommands';

import webClient from '../WebClient';

describe('SessionCommands', () => {
  const roomId = 1;
  let sendSessionCommandSpy;

  beforeEach(() => {
    sendSessionCommandSpy = spyOn(webClient.protobuf, 'sendSessionCommand');

    webClient.protobuf.controller.SessionCommand = { create: args => args };
  });

  describe('connect', () => {
    it('should call SessionCommands.updateStatus and webClient.connect', () => {
      spyOn(SessionCommands, 'updateStatus');
      spyOn(webClient, 'connect');
      const options = {
        host: 'host',
        port: 'port',
        user: 'user',
        pass: 'pass',
      };

      SessionCommands.connect(options);

      expect(SessionCommands.updateStatus).toHaveBeenCalled();
      expect(SessionCommands.updateStatus).toHaveBeenCalledWith(StatusEnum.CONNECTING, "Connecting...");
      
      expect(webClient.connect).toHaveBeenCalled();
      expect(webClient.connect).toHaveBeenCalledWith(options);
    });
  });

  describe('disconnect', () => {
    it('should call SessionCommands.updateStatus and webClient.disconnect', () => {
      spyOn(SessionCommands, 'updateStatus');
      spyOn(webClient, 'disconnect');

      SessionCommands.disconnect();

      expect(SessionCommands.updateStatus).toHaveBeenCalled();
      expect(SessionCommands.updateStatus).toHaveBeenCalledWith(StatusEnum.DISCONNECTING, "Disconnecting...");
      
      expect(webClient.disconnect).toHaveBeenCalled();
    });
  });
});