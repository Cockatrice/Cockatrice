export { default as webClient } from './WebClient';
export { default as ProtoFiles } from './ProtoFiles';


// Export common used services

export { NormalizeService, RoomService} from "./services";


// Note: this has to come after webClient
export { AuthenticationService, ModeratorService, RoomsService, SessionService } from "./instanceService";