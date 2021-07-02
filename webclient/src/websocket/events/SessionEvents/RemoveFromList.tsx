export const RemoveFromList = {
  id: ".Event_RemoveFromList.ext",
  action: ({ listName, userName }, webClient) => {
    switch (listName) {
      case 'buddy': {
        webClient.persistence.session.removeFromBuddyList(userName);
        break;
      }
      case 'ignore': {
        webClient.persistence.session.removeFromIgnoreList(userName);
        break;
      }
      default: {
        webClient.debug(() => console.log('Attempted to remove from unknown list: ', listName));
      }
    }
  }
};