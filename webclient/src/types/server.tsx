export enum StatusEnum {
  DISCONNECTED  =  0,
  CONNECTING    =  1,
  CONNECTED     =  2,
  LOGGINGIN     =  3,
  LOGGEDIN      =  4,
  DISCONNECTING =  99
}

export enum StatusEnumLabel {
  "Disconnected" = 0,
  "Connecting" = 1,
  "Connected" = 2,
  "Loggingin" = 3,
  "Loggedin" = 4,
  "Disconnecting" = 99
}

export const getStatusEnumLabel = (statusEnum: number) => {
  if (StatusEnumLabel[statusEnum] !== undefined) {
    return StatusEnumLabel[statusEnum];
  }

  return "Unknown";
};

export interface Log {
  message: string;
  senderId: string;
  senderIp: string;
  senderName: string;
  targetId: string;
  targetName: string;
  targetType: string;
  time: string;
}