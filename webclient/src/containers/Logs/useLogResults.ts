import { useState } from 'react';

export interface LogResults {
  value: number;
  handleChange: (event: unknown, newValue: number) => void;
}

export function useLogResults(): LogResults {
  const [value, setValue] = useState(0);

  const handleChange = (_event: unknown, newValue: number) => {
    setValue(newValue);
  };

  return { value, handleChange };
}
