export enum StatusEnum {
  DISCONNECTED  =  0,
  CONNECTING    =  1,
  CONNECTED     =  2,
  LOGGINGIN     =  3,
  LOGGEDIN      =  4,
  DISCONNECTING =  99
};

export enum StatusEnumLabel {
  'Disconnected',
  'Connecting',
  'Connected',
  'Loggingin',
  'Loggedin',
  'Disconnecting'
};

export const getStatusEnumLabel = (statusEnum: number) => {
  if (StatusEnumLabel[statusEnum] !== undefined) {
    return StatusEnumLabel[statusEnum];
  }

  return 'Unknown';
}