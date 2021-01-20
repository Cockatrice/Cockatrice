export const AddToList = {
  id: ".Event_AddToList.ext",
  action: ({ listName, userInfo}, webClient) => {
    switch (listName) {
      case 'buddy': {
        webClient.services.session.addToBuddyList(userInfo);
        break;
      }
      case 'ignore': {
        webClient.services.session.addToIgnoreList(userInfo);
        break;
      }
      default: {
        webClient.debug(() => console.log('Attempted to add to unknown list: ', listName));
      }
    }
  }
};