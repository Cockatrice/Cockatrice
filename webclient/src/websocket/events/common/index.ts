import { ProtobufEvents } from '../../services/ProtobufService';
import { playerPropertiesChanged } from './playerPropertiesChanged';

export const CommonEvents: ProtobufEvents = {
  '.Event_PlayerPropertiesChanged.ext': playerPropertiesChanged,
}
