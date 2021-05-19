export const AddToList = {
  id: ".Event_AddToList.ext",
  action: ({ listName, userInfo}, webClient) => {
    switch (listName) {
      case 'buddy': {
        webClient.persistence.session.addToBuddyList(userInfo);
        break;
      }
      case 'ignore': {
        webClient.persistence.session.addToIgnoreList(userInfo);
        break;
      }
      default: {
        webClient.debug(() => console.log('Attempted to add to unknown list: ', listName));
      }
    }
  }
};