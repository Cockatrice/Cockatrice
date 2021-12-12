import {
  secondsToDays,
  secondsToString,
  secondsToYears,
  secondsToYearsDays,
} from './convertTime';

const secondsInDay = 86400;
const secondsInYear = 31536000;

describe('secondsToDays', () => {
  test('returns 0 for less than 1 day', () => {
    expect(secondsToDays(3600)).toBe(0);
  });

  test('returns 1 for 1 day', () => {
    expect(secondsToDays(secondsInDay)).toEqual(1);
  });

  test('returns 10 for 10 days', () => {
    expect(secondsToDays(secondsInDay * 10)).toEqual(10);
  });

  test('returns 10 for just over 10 days', () => {
    expect(secondsToDays(secondsInDay * 10 + 1)).toEqual(10);
  });
});

describe('secondsToYears', () => {
  test('returns 0 for less than 1 year', () => {
    expect(secondsToYears(secondsInYear - 1)).toEqual(0);
  });

  test('returns 1 for 1 year', () => {
    expect(secondsToYears(secondsInYear)).toEqual(1);
  });

  test('returns 1 for less than 2 years', () => {
    expect(secondsToYears(secondsInYear * 1.5)).toEqual(1);
  });

  test('returns 10 for 10 years', () => {
    expect(secondsToYears(secondsInYear * 10)).toEqual(10);
  });
});

describe('secondsToYearsDays', () => {
  test('returns 1 day 0 years for 1 day', () => {
    expect(secondsToYearsDays(secondsInDay)).toEqual({ days: 1, years: 0 });
  });

  test('returns 0 days 1 year for 1 year', () => {
    expect(secondsToYearsDays(secondsInYear)).toEqual({ days: 0, years: 1 });
  });

  test('returns 1 day 1 year for 1 day 1 year', () => {
    expect(secondsToYearsDays(secondsInDay + secondsInYear)).toEqual({
      days: 1,
      years: 1,
    });
  });
});

describe('secondsToString', () => {
  test('returns blanks for 0 seconds', () => {
    expect(secondsToString(0)).toEqual({ daysString: '', yearsString: '' });
  });

  test('returns 1 Day', () => {
    expect(secondsToString(secondsInDay)).toEqual({
      daysString: '1 Day',
      yearsString: '',
    });
  });

  test('returns 1 year', () => {
    expect(secondsToString(secondsInYear)).toEqual({
      daysString: '',
      yearsString: '1 Year',
    });
  });

  test('returns 1 year 1 day', () => {
    expect(secondsToString(secondsInYear + secondsInDay)).toEqual({
      daysString: '1 Day',
      yearsString: '1 Year',
    });
  });

  test('returns 2 years 1 day', () => {
    expect(secondsToString(secondsInYear * 2 + secondsInDay)).toEqual({
      daysString: '1 Day',
      yearsString: '2 Years',
    });
  });

  test('returns 2 years 2 days', () => {
    expect(secondsToString(secondsInYear * 2 + secondsInDay * 2)).toEqual({
      daysString: '2 Days',
      yearsString: '2 Years',
    });
  });
});
