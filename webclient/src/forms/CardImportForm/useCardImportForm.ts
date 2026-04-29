import { useEffect, useState } from 'react';

import { cardImporterService, CardDTO, SetDTO, TokenDTO } from '@app/services';
import type { App } from '@app/types';

export interface CardImportForm {
  loading: boolean;
  activeStep: number;
  importedCards: App.Card[];
  importedSets: App.Set[];
  error: string | null;
  handleNext: () => void;
  handleBack: () => void;
  handleCardDownload: (args: { cardDownloadUrl: string }) => void;
  handleCardSave: () => Promise<void>;
  handleTokenDownload: (args: { tokenDownloadUrl: string }) => void;
}

export function useCardImportForm(): CardImportForm {
  const [loading, setLoading] = useState(false);
  const [activeStep, setActiveStep] = useState(0);
  const [importedCards, setImportedCards] = useState<App.Card[]>([]);
  const [importedSets, setImportedSets] = useState<App.Set[]>([]);
  const [error, setError] = useState<string | null>(null);

  useEffect(() => {
    if (loading) {
      setError(null);
    }
  }, [loading]);

  const handleNext = () => {
    setActiveStep((prevActiveStep) => prevActiveStep + 1);
  };

  const handleBack = () => {
    setActiveStep((prevActiveStep) => prevActiveStep - 1);
  };

  const handleCardDownload = ({ cardDownloadUrl }: { cardDownloadUrl: string }) => {
    setLoading(true);

    cardImporterService.importCards(cardDownloadUrl)
      .then(({ cards, sets }) => {
        setImportedCards(cards);
        setImportedSets(sets);

        handleNext();
      })
      .catch(({ message }) => setError(message))
      .finally(() => setLoading(false));
  };

  const handleCardSave = async () => {
    setLoading(true);

    try {
      await CardDTO.bulkAdd(importedCards);
      await SetDTO.bulkAdd(importedSets);

      handleNext();
    } catch (e) {
      console.error(e);
      setError('Failed to save cards');
    }

    setLoading(false);
  };

  const handleTokenDownload = ({ tokenDownloadUrl }: { tokenDownloadUrl: string }) => {
    setLoading(true);

    cardImporterService.importTokens(tokenDownloadUrl)
      .then(async (tokens) => {
        await TokenDTO.bulkAdd(tokens);
        handleNext();
      })
      .catch(({ message }) => setError(message))
      .finally(() => setLoading(false));
  };

  return {
    loading,
    activeStep,
    importedCards,
    importedSets,
    error,
    handleNext,
    handleBack,
    handleCardDownload,
    handleCardSave,
    handleTokenDownload,
  };
}
