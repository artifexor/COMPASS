/*
 * This file is part of OpenATS COMPASS.
 *
 * COMPASS is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * COMPASS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with COMPASS. If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <memory>

#include <QObject>

class QString;
class QSignalSpy;
class QElapsedTimer;

namespace rtcommand
{

    /**
    */
    class WaitCondition
    {
    public:
        WaitCondition(int timeout_ms = -1) : timeout_ms_(timeout_ms) {}
        virtual ~WaitCondition() = default;

        virtual bool valid() const = 0;
        virtual bool expired() const = 0;
        virtual bool wait() const = 0;

    protected:
        int timeout_ms_;
    };

    /**
     * Condition waiting for an emitted signal.
     */
    class WaitConditionSignal : public WaitCondition
    {
    public:
        WaitConditionSignal(QObject* parent,
                            const QString& obj_name,
                            const QString& signal,
                            int timeout_ms = -1);
        virtual ~WaitConditionSignal();

        virtual bool valid() const override;
        virtual bool expired() const override;
        virtual bool wait() const override;

    protected:
        mutable std::unique_ptr<QSignalSpy> spy_;

    private:
        Q_DISABLE_COPY(WaitConditionSignal)
    };

    /**
     * Condition waiting for a certain amount of time.
     */
    class WaitConditionDelay : public WaitCondition
    {
    public:
        WaitConditionDelay(int msecs);
        virtual ~WaitConditionDelay();

        virtual bool valid() const override;
        virtual bool expired() const override;
        virtual bool wait() const override;

    protected:
        void elapsed();

        int                                    msecs_;
        mutable std::unique_ptr<QElapsedTimer> timer_;

    private:
        Q_DISABLE_COPY(WaitConditionDelay)
    };

    bool waitForCondition(const WaitCondition& condition);

} // namespace rtcommand
