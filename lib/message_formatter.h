#ifndef MESSAGE_FORMATTER_H
#define MESSAGE_FORMATTER_H

#include <format>
// lol, actually what?

// same usage as printf... but and returns formatted string that includes file, line number and function...
#define messageFormatter(fmt, ...) \
    std::format("{}:{} ({}) " + fmt, __FILE__, __LINE__, __func__, ##__VA_ARGS__);

#endif