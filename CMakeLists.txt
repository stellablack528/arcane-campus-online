cmake_minimum_required(VERSION 3.20)

project(HogwartsOnline VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets Network)
include(CTest)

set(ARCANE_MYSQL_ROOT "D:/mysql-8.0.25-winx64" CACHE PATH "MySQL Server or Connector/C installation")
find_path(ARCANE_MYSQL_INCLUDE_DIR mysql.h
    HINTS "${ARCANE_MYSQL_ROOT}/include"
)

if(NOT WIN32 OR NOT ARCANE_MYSQL_INCLUDE_DIR)
    message(FATAL_ERROR "A Windows MySQL C API installation is required. Set ARCANE_MYSQL_ROOT to its root directory.")
endif()

add_library(ArcaneCampusDatabase STATIC
    database/include/IDatabaseDriver.hpp
    database/include/DatabaseConfigLoader.hpp
    database/include/DBConnection.hpp
    database/include/DBConnectionPool.hpp
    database/include/DBTransaction.hpp
    database/include/UserDAO.hpp
    database/include/CharacterDAO.hpp
    database/include/RoomDAO.hpp
    database/include/FriendDAO.hpp
    database/include/MessageDAO.hpp
    database/include/NPCDAO.hpp
    database/include/InventoryDAO.hpp
    database/include/LocationDAO.hpp
    database/include/QuestDAO.hpp
    database/include/StoryDAO.hpp
    database/src/DBConnection.cpp
    database/src/DatabaseConfigLoader.cpp
    database/src/MySqlDatabaseDriver.hpp
    database/src/MySqlDatabaseDriver.cpp
    database/src/NoopDatabaseDriver.hpp
    database/src/NoopDatabaseDriver.cpp
    database/src/DBConnectionPool.cpp
    database/src/DBTransaction.cpp
    database/src/UserDAO.cpp
    database/src/CharacterDAO.cpp
    database/src/RoomDAO.cpp
    database/src/FriendDAO.cpp
    database/src/MessageDAO.cpp
    database/src/NPCDAO.cpp
    database/src/InventoryDAO.cpp
    database/src/LocationDAO.cpp
    database/src/QuestDAO.cpp
    database/src/StoryDAO.cpp
)

target_include_directories(ArcaneCampusDatabase PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/database/include
)

target_include_directories(ArcaneCampusDatabase PRIVATE
    ${ARCANE_MYSQL_INCLUDE_DIR}
)

target_link_libraries(ArcaneCampusDatabase PRIVATE ws2_32)

target_compile_features(ArcaneCampusDatabase PUBLIC cxx_std_20)

add_library(ArcaneStoryEngine STATIC
    src/application/story/StoryTypes.hpp
    src/application/story/StoryContentLoader.hpp
    src/application/story/StoryContentLoader.cpp
    src/application/story/StoryContentValidator.hpp
    src/application/story/StoryContentValidator.cpp
    src/application/story/StoryEngine.hpp
    src/application/story/StoryEngine.cpp
    src/application/story/StoryRuntimeService.hpp
    src/application/story/StoryRuntimeService.cpp
)

target_include_directories(ArcaneStoryEngine PUBLIC src)
target_link_libraries(ArcaneStoryEngine PUBLIC Qt6::Core)
target_compile_features(ArcaneStoryEngine PUBLIC cxx_std_20)

add_library(ArcaneStoryPersistence STATIC
    src/application/story/persistence/StoryPersistenceService.hpp
    src/application/story/persistence/StoryPersistenceService.cpp
)
target_include_directories(ArcaneStoryPersistence PUBLIC src)
target_link_libraries(ArcaneStoryPersistence PUBLIC
    ArcaneStoryEngine
    ArcaneCampusDatabase
    Qt6::Core
)
target_compile_features(ArcaneStoryPersistence PUBLIC cxx_std_20)

if(BUILD_TESTING)
    add_executable(ArcaneCampusDatabaseSmokeTest
        tests/database/DatabaseSmokeTest.cpp
    )

    target_link_libraries(ArcaneCampusDatabaseSmokeTest PRIVATE
        ArcaneCampusDatabase
    )

    add_test(NAME ArcaneCampusDatabaseSmokeTest
        COMMAND ArcaneCampusDatabaseSmokeTest
    )

    add_executable(ArcaneCampusDiagonAlleyRuntimeTest
        tests/application/DiagonAlleyRuntimeTest.cpp
        src/application/diagon_alley/DiagonAlleyCatalog.hpp
        src/application/diagon_alley/DiagonAlleyCatalog.cpp
        src/application/diagon_alley/DiagonAlleyActionParser.hpp
        src/application/diagon_alley/DiagonAlleyActionParser.cpp
        src/application/diagon_alley/DiagonAlleyService.hpp
        src/application/diagon_alley/DiagonAlleyService.cpp
        src/ui/I18n.hpp
        src/ui/I18n.cpp
        src/resources/resources.qrc
    )

    target_include_directories(ArcaneCampusDiagonAlleyRuntimeTest PRIVATE src)
    target_link_libraries(ArcaneCampusDiagonAlleyRuntimeTest PRIVATE Qt6::Widgets)

    add_test(NAME ArcaneCampusDiagonAlleyRuntimeTest
        COMMAND ArcaneCampusDiagonAlleyRuntimeTest
    )
    set_tests_properties(ArcaneCampusDiagonAlleyRuntimeTest PROPERTIES
        ENVIRONMENT_MODIFICATION "PATH=path_list_prepend:$<TARGET_FILE_DIR:Qt6::Core>"
    )

    add_executable(ArcaneCampusStoryEngineTest
        tests/story/StoryEngineTest.cpp
    )
    target_link_libraries(ArcaneCampusStoryEngineTest PRIVATE ArcaneStoryEngine)
    add_test(NAME ArcaneCampusStoryEngineTest COMMAND ArcaneCampusStoryEngineTest)
    set_tests_properties(ArcaneCampusStoryEngineTest PROPERTIES
        ENVIRONMENT_MODIFICATION "PATH=path_list_prepend:$<TARGET_FILE_DIR:Qt6::Core>"
    )

    add_executable(ArcaneCampusStoryRuntimeServiceTest
        tests/story/StoryRuntimeServiceTest.cpp
    )
    target_link_libraries(ArcaneCampusStoryRuntimeServiceTest PRIVATE ArcaneStoryEngine)
    add_test(NAME ArcaneCampusStoryRuntimeServiceTest
        COMMAND ArcaneCampusStoryRuntimeServiceTest
    )
    set_tests_properties(ArcaneCampusStoryRuntimeServiceTest PROPERTIES
        ENVIRONMENT_MODIFICATION "PATH=path_list_prepend:$<TARGET_FILE_DIR:Qt6::Core>"
    )

    add_executable(ArcaneCampusStoryPersistenceServiceTest
        tests/story/StoryPersistenceServiceTest.cpp
    )
    target_link_libraries(ArcaneCampusStoryPersistenceServiceTest PRIVATE
        ArcaneStoryPersistence
    )
    add_test(NAME ArcaneCampusStoryPersistenceServiceTest
        COMMAND ArcaneCampusStoryPersistenceServiceTest
    )
    set_tests_properties(ArcaneCampusStoryPersistenceServiceTest PROPERTIES
        ENVIRONMENT_MODIFICATION "PATH=path_list_prepend:$<TARGET_FILE_DIR:Qt6::Core>"
    )

    add_executable(ArcaneCampusDiagonAlleyParityTest
        tests/story/DiagonAlleyParityTest.cpp
        src/application/story/adapters/DiagonAlleyChapterAdapter.hpp
        src/application/story/adapters/DiagonAlleyChapterAdapter.cpp
        src/application/diagon_alley/DiagonAlleyCatalog.hpp
        src/application/diagon_alley/DiagonAlleyCatalog.cpp
        src/application/diagon_alley/DiagonAlleyActionParser.hpp
        src/application/diagon_alley/DiagonAlleyActionParser.cpp
        src/application/diagon_alley/DiagonAlleyService.hpp
        src/application/diagon_alley/DiagonAlleyService.cpp
        src/ui/I18n.hpp
        src/ui/I18n.cpp
        src/resources/resources.qrc
    )
    target_include_directories(ArcaneCampusDiagonAlleyParityTest PRIVATE src)
    target_link_libraries(ArcaneCampusDiagonAlleyParityTest PRIVATE
        ArcaneStoryEngine
        Qt6::Widgets
    )
    add_test(NAME ArcaneCampusDiagonAlleyParityTest
        COMMAND ArcaneCampusDiagonAlleyParityTest
            --report ${CMAKE_CURRENT_BINARY_DIR}/test-results/diagon_alley_parity_report.txt
    )
    set_tests_properties(ArcaneCampusDiagonAlleyParityTest PROPERTIES
        ENVIRONMENT_MODIFICATION "PATH=path_list_prepend:$<TARGET_FILE_DIR:Qt6::Core>"
    )
endif()

option(ARCANE_BUILD_MYSQL_INTEGRATION_TESTS "Build tests that require a configured local MySQL database" OFF)

if(BUILD_TESTING AND ARCANE_BUILD_MYSQL_INTEGRATION_TESTS)
    add_executable(ArcaneCampusMySqlIntegrationTest
        tests/database/MySqlIntegrationTest.cpp
    )

    target_link_libraries(ArcaneCampusMySqlIntegrationTest PRIVATE
        ArcaneCampusDatabase
    )

    add_test(NAME ArcaneCampusMySqlIntegrationTest
        COMMAND ArcaneCampusMySqlIntegrationTest
    )
endif()

qt_add_executable(HogwartsOnline
    src/main.cpp
    src/application/do/CampusDO.hpp
    src/application/dto/CampusDTO.hpp
    src/application/vo/CampusVO.hpp
    src/application/service/SessionService.hpp
    src/application/service/SessionService.cpp
    src/application/service/CampusService.hpp
    src/application/service/CampusService.cpp
    src/application/service/ChatService.hpp
    src/application/service/ChatService.cpp
    src/application/service/InventoryService.hpp
    src/application/service/InventoryService.cpp
    src/application/service/MapService.hpp
    src/application/service/MapService.cpp
    src/application/service/SocialService.hpp
    src/application/service/SocialService.cpp
    src/application/service/QuestService.hpp
    src/application/service/QuestService.cpp
    src/application/service/StoryProgressService.hpp
    src/application/service/StoryProgressService.cpp
    src/application/narrative/NarrativeDO.hpp
    src/application/narrative/NarrativeVO.hpp
    src/application/narrative/NarrativeDTO.hpp
    src/application/narrative/NarrativeService.hpp
    src/application/narrative/NarrativeService.cpp
    src/application/prologue/PrologueService.hpp
    src/application/prologue/PrologueService.cpp
    src/application/prologue/WandLoreService.hpp
    src/application/prologue/WandLoreService.cpp
    src/application/diagon_alley/DiagonAlleyCatalog.hpp
    src/application/diagon_alley/DiagonAlleyCatalog.cpp
    src/application/diagon_alley/DiagonAlleyActionParser.hpp
    src/application/diagon_alley/DiagonAlleyActionParser.cpp
    src/application/diagon_alley/DiagonAlleyService.hpp
    src/application/diagon_alley/DiagonAlleyService.cpp
    src/application/story/adapters/DiagonAlleyChapterAdapter.hpp
    src/application/story/adapters/DiagonAlleyChapterAdapter.cpp
    src/application/controller/CampusController.hpp
    src/application/controller/CampusController.cpp
    src/application/router/ClientCommandRouter.hpp
    src/application/router/ClientCommandRouter.cpp
    src/application/world/WorldClock.hpp
    src/application/world/WorldClock.cpp
    src/application/world/WorldEvent.hpp
    src/application/world/WorldEvent.cpp
    src/core/DeepSeekClient.hpp
    src/core/DeepSeekClient.cpp
    src/ui/LoginWindow.h
    src/ui/LoginWindow.cpp
    src/ui/I18n.hpp
    src/ui/I18n.cpp
    src/ui/ApiKeyDialog.h
    src/ui/ApiKeyDialog.cpp
    src/ui/EnrollmentDialog.h
    src/ui/EnrollmentDialog.cpp
    src/ui/MainWindow.h
    src/ui/MainWindow.cpp
    src/ui/widgets/MemberListWidget.h
    src/ui/widgets/MemberListWidget.cpp
    src/ui/widgets/ChatEventWidget.h
    src/ui/widgets/ChatEventWidget.cpp
    src/ui/widgets/CharacterStatusWidget.h
    src/ui/widgets/CharacterStatusWidget.cpp
    src/ui/widgets/InventoryWidget.h
    src/ui/widgets/InventoryWidget.cpp
    src/ui/widgets/CourseScheduleWidget.h
    src/ui/widgets/CourseScheduleWidget.cpp
    src/ui/widgets/CampusTimeWidget.h
    src/ui/widgets/CampusTimeWidget.cpp
    src/ui/widgets/HouseRankingWidget.h
    src/ui/widgets/HouseRankingWidget.cpp
    src/ui/widgets/LocationActionBar.h
    src/ui/widgets/LocationActionBar.cpp
    src/ui/widgets/LocationPanel.h
    src/ui/widgets/LocationPanel.cpp
    src/resources/resources.qrc
)

target_include_directories(HogwartsOnline PRIVATE src)

target_link_libraries(HogwartsOnline PRIVATE
    ArcaneCampusDatabase
    ArcaneStoryEngine
    ArcaneStoryPersistence
    Qt6::Widgets
    Qt6::Network
)

if(WIN32)
    set_target_properties(HogwartsOnline PROPERTIES
        WIN32_EXECUTABLE TRUE
    )
endif()
