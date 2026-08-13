#include "DatabaseConfigLoader.hpp"
#include "RoomDAO.hpp"
#include "UserDAO.hpp"

#include <cstdlib>
#include <iostream>
#include <memory>

using namespace arcane::database;

int main()
{
    const char* configPath = std::getenv("ARCANE_DB_CONFIG");
    if (!configPath) {
        std::cerr << "ARCANE_DB_CONFIG must point to database.local.ini.\n";
        return 2;
    }

    std::string error;
    const auto config = DatabaseConfigLoader::loadFromFile(configPath, &error);
    if (!config) {
        std::cerr << error << '\n';
        return 3;
    }

    auto connection = std::make_shared<DBConnection>(*config);
    if (!connection->connect()) {
        std::cerr << connection->lastError() << '\n';
        return 4;
    }

    RoomDAO roomDao(connection);
    const auto rooms = roomDao.getAllRooms();
    if (rooms.size() < 7) {
        std::cerr << "Expected at least seven seeded rooms.\n";
        return 5;
    }

    UserDAO userDao(connection);
    if (!userDao.getUserByName("raven_test")) {
        std::cerr << "Seeded user raven_test was not found.\n";
        return 6;
    }

    std::cout << "MySQL integration test passed with " << rooms.size() << " rooms.\n";
    return 0;
}
