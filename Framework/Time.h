#pragma once

namespace fw
{

class Time
{
public:
    Time() {};
    ~Time() {};
    Time(const Time&) = delete;
    Time(Time&&) = delete;
    Time& operator=(const Time&) = delete;
    Time& operator=(Time&&) = delete;

    void update();
    float getSinceStart() const;
    float getDelta() const;

private:
    float sinceStart = 0.0;
    float delta = 0.0;
};

} // namespace fw
