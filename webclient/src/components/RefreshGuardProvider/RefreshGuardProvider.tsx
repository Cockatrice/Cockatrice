import React from 'react';
import { useGameGuard } from 'hooks';
import { RefreshGuardModal } from '../RefreshGuardModal/RefreshGuardModal';

interface RefreshGuardProviderProps {
  children: React.ReactNode;
}

/**
 * Provider component that wraps the entire app to provide refresh and navigation guarding.
 * Should be placed high in the component tree, ideally in AppShell.
 */
export const RefreshGuardProvider: React.FC<RefreshGuardProviderProps> = ({ children }) => {
  const {
    showNavigationModal,
    setShowNavigationModal,
    handleConfirmNavigation,
    handleCancelNavigation,
    navigationMessage,
    isInActiveGame,
    shouldGuard
  } = useGameGuard();

  // Only show custom modal for SPA navigation, not for browser refresh
  // Browser refresh is handled by the browser's native beforeunload dialog
  
  return (
    <>
      {children}
      
      {/* Custom modal only for SPA navigation (React Router) */}
      <RefreshGuardModal
        open={showNavigationModal}
        onClose={handleCancelNavigation}
        onConfirm={handleConfirmNavigation}
        message={navigationMessage}
        title={isInActiveGame ? "Active Game Warning" : "Connection Warning"}
        confirmButtonText={isInActiveGame ? "Leave Game" : "Leave Anyway"}
        cancelButtonText="Stay"
      />
    </>
  );
};