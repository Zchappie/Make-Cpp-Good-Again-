#include <string>
#include <iomanip>

#include "format.h"

/**
 * INPUT: Long int measuring seconds
 * OUTPUT: HH:MM:SS
 * Ref: https://knowledge.udacity.com/questions/155686
 */
std::string Format::ElapsedTime(long seconds) { 
    int hour, minute, second;
    second = seconds % 60;
    minute = seconds / 60;
    hour = minute / 60;

    // to avoid single digit problem, the field width should be set and filled by leading 0
    // to print to ncurses, ostringstream is used
    std::ostringstream stream;
    stream << std::setw(2) << std::setfill('0') << std::to_string(hour) << ":" << 
    std::setw(2) << std::setfill('0') << std::to_string(minute % 60) << ":" <<
    std::setw(2) << std::setfill('0') << std::to_string(second);
    return stream.str();
}