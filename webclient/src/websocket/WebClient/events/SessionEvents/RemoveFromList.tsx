export const RemoveFromList = {
  id: ".Event_RemoveFromList.ext",
  action: ({ listName, userName }, webClient) => {
    switch (listName) {
      case 'buddy': {
        webClient.services.session.removeFromBuddyList(userName);
        break;
      }
      case 'ignore': {
        webClient.services.session.removeFromIgnoreList(userName);
        break;
      }
      default: {
        webClient.debug(() => console.log('Attempted to remove from unknown list: ', listName));
      }
    }
  }
};