import { useEffect, useState } from 'react';
import { useTranslation } from 'react-i18next';

export function useLocaleSort(arr: string[], valueGetter: (value: string) => string) {
  const [state] = useState<string[]>(arr);
  const [sorted, setSorted] = useState<string[]>([]);

  const { i18n } = useTranslation();

  useEffect(() => {
    const collator = new Intl.Collator(i18n.language);
    const sorter = (a, b) => collator.compare(valueGetter(a), valueGetter(b));

    setSorted(state.sort(sorter));
  }, [state, i18n.language]);

  return sorted;
}
