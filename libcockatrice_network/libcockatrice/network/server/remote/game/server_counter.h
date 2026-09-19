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
 * @brief Represents a player counter with overflow-safe increment arithmetic.
 *
 * All value modifications return whether the value actually changed,
 * enabling callers to skip unnecessary network events.
 *
 * @note Direct assignment via setCount() does not clamp; only
 *       incrementCount() enforces int boundary saturation.
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

public:
    Server_Counter(int _id, const QString &_name, const color &_counterColor, int _radius, int _count = 0);
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

    /**
     * @brief Sets the counter to an exact value.
     * @param _count The new value (assigned directly without clamping).
     * @return true if the value changed, false otherwise.
     * @warning This performs raw assignment. For overflow-safe incrementing,
     *          use incrementCount().
     */
    [[nodiscard]] bool setCount(int _count)
    {
        const int oldCount = count;
        count = _count;
        return count != oldCount;
    }

    /**
     * @brief Increments the counter by delta with overflow-safe arithmetic.
     * @param delta The amount to add (may be negative for decrement).
     * @return true if the value changed, false otherwise.
     * @note Clamps result to [INT_MIN, INT_MAX] to prevent overflow.
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
