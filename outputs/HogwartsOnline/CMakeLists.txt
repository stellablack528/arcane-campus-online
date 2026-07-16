cmake_minimum_required(VERSION 3.20)

project(HogwartsOnline VERSION 0.2.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)
include(CTest)

# ====== 数据库抽象层（静态库）======
add_library(ArcaneCampusDatabase STATIC
    database/include/IDatabaseDriver.hpp
    database/include/DBConnection.hpp
    database/include/DBConnectionPool.hpp
    database/include/UserDAO.hpp
    database/include/CharacterDAO.hpp
    database/include/RoomDAO.hpp
    database/include/FriendDAO.hpp
    database/include/MessageDAO.hpp
    database/include/NPCDAO.hpp
    database/include/InventoryDAO.hpp
    database/src/DBConnection.cpp
    database/src/NoopDatabaseDriver.hpp
    database/src/NoopDatabaseDriver.cpp
    database/src/DBConnectionPool.cpp
    database/src/UserDAO.cpp
    database/src/CharacterDAO.cpp
    database/src/RoomDAO.cpp
    database/src/FriendDAO.cpp
    database/src/MessageDAO.cpp
    database/src/NPCDAO.cpp
    database/src/InventoryDAO.cpp
)

target_include_directories(ArcaneCampusDatabase PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/database/include
)

target_compile_features(ArcaneCampusDatabase PUBLIC cxx_std_20)

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
endif()

# ====== Qt6 桌面客户端（可执行文件）======
qt_add_executable(HogwartsOnline
    src/main.cpp

    # 核心
    src/core/LanguageManager.h
    src/core/LanguageManager.cpp
    src/core/DeepSeekClient.h
    src/core/DeepSeekClient.cpp

    # 窗口
    src/ui/WelcomeWindow.h
    src/ui/WelcomeWindow.cpp
    src/ui/LoginWindow.h
    src/ui/LoginWindow.cpp
    src/ui/MainWindow.h
    src/ui/MainWindow.cpp

    # Widget
    src/ui/widgets/CampusTimeWidget.h
    src/ui/widgets/CampusTimeWidget.cpp
    src/ui/widgets/CharacterStatusWidget.h
    src/ui/widgets/CharacterStatusWidget.cpp
    src/ui/widgets/ChatEventWidget.h
    src/ui/widgets/ChatEventWidget.cpp
    src/ui/widgets/CourseScheduleWidget.h
    src/ui/widgets/CourseScheduleWidget.cpp
    src/ui/widgets/HouseRankingWidget.h
    src/ui/widgets/HouseRankingWidget.cpp
    src/ui/widgets/InventoryWidget.h
    src/ui/widgets/InventoryWidget.cpp
    src/ui/widgets/LocationActionBar.h
    src/ui/widgets/LocationActionBar.cpp
    src/ui/widgets/MemberListWidget.h
    src/ui/widgets/MemberListWidget.cpp
    src/ui/widgets/NpcChatDialog.h
    src/ui/widgets/NpcChatDialog.cpp

    # 资源
    src/resources/resources.qrc

    # 国际化
    src/i18n/arcane_campus_zh_CN.ts
)

target_include_directories(HogwartsOnline PRIVATE src)

target_link_libraries(HogwartsOnline PRIVATE
    ArcaneCampusDatabase
    Qt6::Widgets
)

if(WIN32)
    set_target_properties(HogwartsOnline PROPERTIES
        WIN32_EXECUTABLE TRUE
    )
endif()

# ====== 国际化：编译 .ts → .qm ======
find_package(Qt6 COMPONENTS LinguistTools QUIET)
if(Qt6LinguistTools_FOUND)
    qt_add_translation(HogwartsTranslation
        TS_FILES src/i18n/arcane_campus_zh_CN.ts
        QM_FILES_OUTPUT_VARIABLE QM_FILES
    )
    add_dependencies(HogwartsOnline HogwartsTranslation)
endif()

# 如果 Qt6 LinguistTools 未找到，说明检查:
message(STATUS "LinguistTools found: ${Qt6LinguistTools_FOUND}")
message(STATUS "⚡ To compile .ts → .qm manually:")
message(STATUS "   cd src/i18n && lrelease arcane_campus_zh_CN.ts")
