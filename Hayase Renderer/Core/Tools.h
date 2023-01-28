#ifndef TOOLS_H
#define TOOLS_H

#include <memory>

#define BIND_EVENT_FUNC(x) [this](auto&&... args) -> decltype(auto) { return this->x(std::forward<decltype(args)>(args)...); }


#endif