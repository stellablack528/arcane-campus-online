#include "CharacterDAO.hpp"
#include "DBConnection.hpp"
#include "DBConnectionPool.hpp"
#include "InventoryDAO.hpp"
#include "MessageDAO.hpp"
#include "RoomDAO.hpp"
#include "UserDAO.hpp"

#include <cassert>
#include <memory>

using namespace arcane::database;

int main()
{
    DBConfig config;
    config.database = "arcane_campus_online_test";

    auto connection = std::make_shared<DBConnection>(config);
    assert(connection->connect());
    assert(connection->isConnected());
    assert(connection->driverName() == "noop");

    UserDAO userDao(connection);
    assert(userDao.createUser(UserRecord{
        .username = "smoke_user",
        .passwordHash = "$argon2id$smoke_hash_placeholder",
        .email = "smoke@example.com",
        .avatar = "avatars/smoke.png",
        .status = "active",
    }));

    RoomDAO roomDao(connection);
    assert(roomDao.createRoom(RoomRecord{
        .roomName = "Smoke Room",
        .roomType = "test",
        .description = "DAO smoke test room.",
        .maxPlayers = 8,
    }));

    CharacterDAO characterDao(connection);
    assert(characterDao.createCharacter(CharacterRecord{
        .userId = 1,
        .nickname = "Smoke Character",
        .house = "Ravenclaw",
        .level = 1,
        .experience = 0,
        .gold = 0,
        .currentRoomId = 1,
        .title = "Tester",
    }));
    assert(characterDao.updateCharacterRoom(1, 2));

    MessageDAO messageDao(connection);
    assert(messageDao.saveMessage(MessageRecord{
        .senderCharacterId = 1,
        .roomId = 1,
        .content = "Smoke test message.",
    }));

    InventoryDAO inventoryDao(connection);
    assert(inventoryDao.createItem(ItemRecord{
        .itemName = "Smoke Item",
        .itemType = "test",
        .description = "DAO smoke test item.",
        .stackable = true,
    }));
    assert(inventoryDao.addInventoryItem(InventoryRecord{
        .characterId = 1,
        .itemId = 1,
        .quantity = 1,
    }));

    DBConnectionPool pool(config, 1, 2);
    {
        auto pooled = pool.acquire();
        assert(pooled);
        assert(pooled->isConnected());
        assert(pooled->driverName() == "noop");
    }
    assert(pool.availableCount() == 1);

    return 0;
}
