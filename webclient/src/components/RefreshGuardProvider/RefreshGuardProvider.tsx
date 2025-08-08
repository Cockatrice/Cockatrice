import React from "react";
import { useGameGuard } from "hooks";
import { RefreshGuardModal } from "../RefreshGuardModal/RefreshGuardModal";

interface RefreshGuardProviderProps {
  children: React.ReactNode;
}

/**
 * Provider component that wraps the entire app to provide refresh and navigation guarding.
 */
export const RefreshGuardProvider: React.FC<RefreshGuardProviderProps> = ({
  children,
}) => {
  const {
    showNavigationModal,
    handleConfirmNavigation,
    handleCancelNavigation,
    navigationMessage,
    showRefreshModal,
    handleConfirmRefresh,
    handleCancelRefresh,
    refreshMessage,
    isInActiveGame,
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
