import { DateTime } from 'luxon';

class TimeService {
  renderAccountAge(seconds: number) {
    const time = DateTime.now().diff(DateTime.now().minus({ seconds }), ['days', 'years']);

    const years = Math.floor(time.years);
    const days = Math.floor(time.days);

    let accountAge = '';

    if (years > 0) {
      accountAge += `${years} Year`;
    }

    if (years > 1) {
      accountAge += 's';
    }

    if (days > 0) {
      years > 0 ? (accountAge += ' ') : (accountAge += '');
      accountAge += `${days} Day`;
    }

    if (days > 1) {
      accountAge += 's';
    }

    return accountAge;
  }
}

export const timeService = new TimeService();
