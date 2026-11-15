#include "application/open_world/talestate/OpenWorldRouteGuard.hpp"

#include <iostream>

int main()
{
    using arcane::application::open_world::talestate::OpenWorldRouteGuard;
    if (!OpenWorldRouteGuard::check("charms_classroom", "library").allowed
        || !OpenWorldRouteGuard::check("library", "hogsmeade").allowed
        || !OpenWorldRouteGuard::check("courtyard", "great_hall").allowed) {
        std::cerr << "A declared campus route was rejected\n";
        return 1;
    }
    if (OpenWorldRouteGuard::check("great_hall", "upper_floor").allowed
        || OpenWorldRouteGuard::check("unknown_room", "library").allowed
        || OpenWorldRouteGuard::check("unknown_room", "unknown_room").allowed) {
        std::cerr << "An undeclared route was accepted\n";
        return 1;
    }
    return 0;
}
