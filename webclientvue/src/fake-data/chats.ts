function getTimestamp() {
  const now = new Date();
  return `[${now.getHours()}:${now.getMinutes()}:${now.getSeconds()}]`;
}

export const chats = [
  {
    username: 'Zach',
    message: 'Captain Sisay is the best commander',
    avatar: 'user',
    timestamp: getTimestamp(),
  },
];
