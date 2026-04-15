import protobuf from 'protobufjs';

import { SessionPersistence } from '../persistence';
import ProtoFiles from '../../proto-files.json';

const PB_FILE_DIR = `${process.env.PUBLIC_URL}/pb`;

// Leaf module — no imports from the websocket layer other than persistence.
// Both BackendService and ProtobufService import this; neither should import
// the other for controller access, avoiding circular dependency cycles.
export const ProtoController = {
  root: null as any,

  load(): void {
    const files = ProtoFiles.map(file => `${PB_FILE_DIR}/${file}`);
    ProtoController.root = new protobuf.Root();
    ProtoController.root.load(files, { keepCase: false }, (err: Error) => {
      if (err) {
        throw err;
      }
      SessionPersistence.initialized();
    });
  },
};
