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
    showRefreshModal,
    setShowRefreshModal,
    handleConfirmRefresh,
    handleCancelRefresh,
    refreshMessage,
    isInActiveGame,
    shouldGuard
  } = useGameGuard();

  return (
    <>
      {children}
      
      {/* Custom modal for SPA navigation (React Router) */}
      <RefreshGuardModal
        open={showNavigationModal}
        onClose={handleCancelNavigation}
        onConfirm={handleConfirmNavigation}
        message={navigationMessage}
        title={isInActiveGame ? "Active Game Warning" : "Connection Warning"}
        confirmButtonText={isInActiveGame ? "Leave Game" : "Leave Anyway"}
        cancelButtonText="Stay"
      />
      
      {/* Custom modal for browser refresh (after native dialog) */}
      <RefreshGuardModal
        open={showRefreshModal}
        onClose={handleCancelRefresh}
        onConfirm={handleConfirmRefresh}
        message={refreshMessage}
        title={isInActiveGame ? "Page Refresh Warning" : "Connection Warning"}
        confirmButtonText="Refresh Anyway"
        cancelButtonText="Stay on Page"
      />
    </>
  );
};