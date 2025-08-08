import { useSelector } from 'react-redux';
import { useRefreshGuard } from './useRefreshGuard';
import { useNavigationGuard } from './useNavigationGuard';
import { RoomsSelectors, ServerSelectors } from 'store';
import { StatusEnum } from 'types';
import { webClient } from 'websocket';

interface UseGameGuardReturn {
  // Navigation guard (SPA only)
  showNavigationModal: boolean;
  setShowNavigationModal: (show: boolean) => void;
  pendingLocation: string | null;
  handleConfirmNavigation: () => void;
  handleCancelNavigation: () => void;
  navigationMessage: string;
  
  // Guard state
  isInActiveGame: boolean;
  isConnected: boolean;
  shouldGuard: boolean;
}

/**
 * Combined hook that guards against page refresh (browser dialog) and SPA navigation (custom modal)
 * when user is in an active game or has an active connection.
 * 
 * - Browser refresh/navigation: Shows native browser dialog only
 * - SPA navigation: Shows custom modal with detailed warning
 */
export const useGameGuard = (): UseGameGuardReturn => {
  // Get connection and game state from Redux
  const connectionStatus = useSelector(ServerSelectors.getState);
  const joinedGameIds = useSelector(RoomsSelectors.getJoinedGameIds);
  const joinedRoomIds = useSelector(RoomsSelectors.getJoinedRoomIds);
  
  // Determine if user is in an active state that should be guarded
  const isConnected = connectionStatus === StatusEnum.LOGGED_IN;
  const isInActiveGame = Object.values(joinedGameIds).some(roomGames => 
    Object.keys(roomGames).length > 0
  );
  const isInRoom = Object.keys(joinedRoomIds).length > 0;
  
  // Guard conditions
  const shouldGuard = isConnected && (isInActiveGame || isInRoom);
  
  // Messages based on current state
  const getMessage = (): string => {
    if (isInActiveGame) {
      return 'You are currently in an active game. Leaving will disconnect you from the game and may cause issues for other players.';
    }
    if (isInRoom) {
      return 'You are currently connected to game rooms. Leaving will disconnect you from the server.';
    }
    return 'You are connected to the server. Leaving will end your session.';
  };

  const message = getMessage();

  // Handle graceful disconnect before leaving
  const handleGracefulDisconnect = async () => {
    try {
      // If in active games, try to leave them gracefully
      if (isInActiveGame) {
        console.log('Attempting graceful disconnect from active games...');
        // Note: Specific game leave commands would be implemented here
        // For now, we'll just disconnect the WebSocket
      }
      
      // Disconnect from server
      webClient.disconnect();
    } catch (error) {
      console.error('Error during graceful disconnect:', error);
      // Force disconnect even if graceful fails
      webClient.disconnect();
    }
  };

  // Refresh guard (browser page refresh/navigation only)
  const refreshGuard = useRefreshGuard({
    shouldGuard,
    message,
    onUnload: handleGracefulDisconnect
  });

  // Navigation guard (SPA navigation only)
  const navigationGuard = useNavigationGuard({
    shouldGuard,
    message,
    onBeforeNavigate: (targetPath) => {
      console.log('Navigation blocked, target:', targetPath);
    }
  });

  return {
    // Navigation guard (SPA only)
    showNavigationModal: navigationGuard.showModal,
    setShowNavigationModal: navigationGuard.setShowModal,
    pendingLocation: navigationGuard.pendingLocation,
    handleConfirmNavigation: async () => {
      await handleGracefulDisconnect();
      navigationGuard.handleConfirmNavigation();
    },
    handleCancelNavigation: navigationGuard.handleCancelNavigation,
    navigationMessage: navigationGuard.guardMessage,
    
    // State
    isInActiveGame,
    isConnected,
    shouldGuard
  };
};