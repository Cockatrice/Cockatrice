import { timeService } from './TimeService';
import { DateTime, Settings } from 'luxon';

const secondsInDay = 86400;
const secondsInYear = 31536000;

describe('renderAccountAge', () => {
  // Set mock time now - only test non leap years
  const mockNow = DateTime.local(2019, 6, 1, 0, 0, 0);
  Settings.now = () => mockNow.toMillis();

  test('returns "" for 0 seconds', () => {
    expect(timeService.renderAccountAge(0)).toEqual('');
  });

  test('returns "" for less than one day', () => {
    expect(timeService.renderAccountAge(120)).toEqual('');
  });

  test('returns "1 Day" for exactly', () => {
    expect(timeService.renderAccountAge(secondsInDay)).toEqual('1 Day');
  });

  test('returns "1 Day" for just over', () => {
    expect(timeService.renderAccountAge(secondsInDay + 100)).toEqual('1 Day');
  });

  test('returns "2 Days"', () => {
    expect(timeService.renderAccountAge(secondsInDay * 2)).toEqual('2 Days');
  });

  test('returns "1 Year"', () => {
    expect(timeService.renderAccountAge(secondsInYear)).toEqual('1 Year');
  });

  test('returns "2 Years"', () => {
    expect(timeService.renderAccountAge(secondsInYear * 2)).toEqual('2 Years');
  });

  test('returns "1 Year 1 Day', () => {
    expect(timeService.renderAccountAge(secondsInDay + secondsInYear)).toEqual('1 Year 1 Day');
  });

  test('returns "1 Year 2 Days', () => {
    expect(timeService.renderAccountAge(secondsInDay * 2 + secondsInYear)).toEqual('1 Year 2 Days');
  });

  test('returns "2 Years 1 Day', () => {
    expect(timeService.renderAccountAge(secondsInDay + secondsInYear * 2)).toEqual('2 Years 1 Day');
  });

  test('returns "2 Years 2 Days', () => {
    expect(timeService.renderAccountAge(secondsInDay * 2 + secondsInYear * 2)).toEqual('2 Years 2 Days');
  });
});
