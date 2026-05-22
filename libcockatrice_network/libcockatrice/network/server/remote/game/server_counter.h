/***************************************************************************
 *   Copyright (C) 2008 by Max-Wilhelm Bruker   *
 *   brukie@laptop   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#ifndef SERVER_COUNTER_H
#define SERVER_COUNTER_H

#include <QString>
#include <libcockatrice/protocol/pb/color.pb.h>
#include <libcockatrice/utility/clamped_arithmetic.h>
#include <limits>

class ServerInfo_Counter;

/**
 * @class Server_Counter
 * @brief Represents a player counter with overflow-safe increment arithmetic and optional bounds.
 *
 * All value modifications return whether the value actually changed,
 * enabling callers to skip unnecessary network events.
 *
 * @note Values are clamped to [minValue, maxValue] on both setCount() and incrementCount().
 * @note Unlike card counters, player counters are never auto-removed
 *       when they reach zero - they persist with value 0.
 */
class Server_Counter
{
protected:
    int id;
    QString name;
    color counterColor;
    int radius;
    int count;
    int minValue;       ///< Minimum allowed value (default: INT_MIN, i.e. unbounded)
    int maxValue;       ///< Maximum allowed value (default: INT_MAX, i.e. unbounded)
    bool active = true; ///< Whether this counter is visible/active (default: true)

    static constexpr int DEFAULT_MAX_VALUE = std::numeric_limits<int>::max();

public:
    /**
     * @brief Constructs a counter.
     * @param _id Unique counter identifier
     * @param _name Display name
     * @param _counterColor Counter color
     * @param _radius Display radius
     * @param _count Initial value (default 0)
     * @param _minValue Minimum allowed value (default INT_MIN)
     * @param _maxValue Maximum allowed value (default INT_MAX)
     */
    Server_Counter(int _id,
                   const QString &_name,
                   const color &_counterColor,
                   int _radius,
                   int _count = 0,
                   int _minValue = std::numeric_limits<int>::min(),
                   int _maxValue = DEFAULT_MAX_VALUE);
    ~Server_Counter()
    {
    }
    int getId() const
    {
        return id;
    }
    QString getName() const
    {
        return name;
    }
    const color &getColor() const
    {
        return counterColor;
    }
    int getRadius() const
    {
        return radius;
    }
    int getCount() const
    {
        return count;
    }
    bool isActive() const
    {
        return active;
    }
    /**
     * @brief Sets the active (visible) state of this counter.
     * @param _active True to show the counter, false to hide it
     * @return true if the state changed
     */
    [[nodiscard]] bool setActive(bool _active)
    {
        bool oldActive = active;
        active = _active;
        return active != oldActive;
    }
    /**
     * @brief Sets the counter value, clamping to [minValue, maxValue].
     * @param _count The desired new value
     * @return true if the clamped value differs from the previous value
     * @note For increment operations, prefer incrementCount() which handles overflow safely.
     */
    [[nodiscard]] bool setCount(int _count)
    {
        int oldCount = count;
        count = qBound(minValue, _count, maxValue);
        return count != oldCount;
    }

    /**
     * @brief Increments the counter by delta with overflow-safe arithmetic.
     * @param delta The amount to add (may be negative for decrement).
     * @return true if the value changed, false otherwise.
     * @note Clamps result to [minValue, maxValue] to prevent overflow.
     */
    [[nodiscard]] bool incrementCount(int delta)
    {
        const int oldCount = count;
        count = addClamped(count, delta, std::numeric_limits<int>::min(), std::numeric_limits<int>::max());
        return count != oldCount;
    }

    /**
     * @brief Populates info with this counter's current state for network serialization.
     * @param info The protobuf message to populate.
     */
    void getInfo(ServerInfo_Counter *info);
};

#endif
