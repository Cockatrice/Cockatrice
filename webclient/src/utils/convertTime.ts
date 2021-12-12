export const DAYS_IN_YEAR = 365;
export const HOURS_IN_DAY = 24;
export const MINUTES_IN_HOUR = 60;
export const SECONDS_IN_MINUTE = 60;

export const secondsToDays = (seconds) =>
  Math.floor(seconds / SECONDS_IN_MINUTE / MINUTES_IN_HOUR / HOURS_IN_DAY);

export const secondsToYears = (seconds) =>
  Math.floor(secondsToDays(seconds) / DAYS_IN_YEAR);

export const secondsToYearsDays = (seconds) => {
  const years = secondsToYears(seconds);
  const days = secondsToDays(seconds) - years * DAYS_IN_YEAR;

  return {
    days,
    years,
  };
};

export const secondsToString = (seconds) => {
  const { years, days } = secondsToYearsDays(seconds);

  let yearsString = '';
  let daysString = '';

  if (years !== 0) {
    yearsString += `${years} Year`;
  }

  if (years > 1) {
    yearsString += 's';
  }
  if (days !== 0) {
    daysString += `${days} Day`;
  }

  if (days > 1) {
    daysString += 's';
  }

  return { daysString, yearsString };
};
