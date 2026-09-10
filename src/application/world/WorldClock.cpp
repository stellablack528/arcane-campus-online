#include "application/world/WorldClock.hpp"

namespace arcane::application::world {

TimePeriod WorldClock::periodFromHour(int hour)
{
    if (hour >= 0 && hour < 6)  return TimePeriod::LateNight;
    if (hour < 8)               return TimePeriod::EarlyMorning;
    if (hour < 12)              return TimePeriod::MorningClasses;
    if (hour < 14)              return TimePeriod::Lunch;
    if (hour < 18)              return TimePeriod::AfternoonClasses;
    if (hour < 21)              return TimePeriod::Evening;
    return TimePeriod::Night;   // 21:00-24:00
}

std::string WorldClock::displayName(TimePeriod period)
{
    switch (period) {
        case TimePeriod::EarlyMorning:     return "Early Morning";
        case TimePeriod::MorningClasses:   return "Morning Classes";
        case TimePeriod::Lunch:             return "Lunch Break";
        case TimePeriod::AfternoonClasses: return "Afternoon Classes";
        case TimePeriod::Evening:           return "Evening";
        case TimePeriod::Night:             return "Night";
        case TimePeriod::LateNight:         return "Late Night (Curfew)";
    }
    return "Unknown";
}

bool WorldClock::classesInSession(TimePeriod period)
{
    return period == TimePeriod::MorningClasses || period == TimePeriod::AfternoonClasses;
}

bool WorldClock::publicAreasOpen(TimePeriod period)
{
    return period != TimePeriod::LateNight;
}

bool WorldClock::commonRoomOnly(TimePeriod period)
{
    return period == TimePeriod::LateNight;
}

bool WorldClock::quietHours(TimePeriod period)
{
    return period == TimePeriod::Night || period == TimePeriod::LateNight;
}

} // namespace arcane::application::world
