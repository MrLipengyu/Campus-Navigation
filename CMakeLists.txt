cmake_minimum_required(VERSION 3.16)

project(CampusNavigator VERSION 1.0 LANGUAGES CXX)

# 指定 C++ 标准为 C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 自动处理 Qt 的元对象系统
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# 寻找 Qt 6 组件 (Core, Gui, Widgets, Sql)
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Sql)

# 收集源文件 (初期先将主入口和UI包含进来，后续建文件夹后再补充)
set(PROJECT_SOURCES
    main.cpp

    # ui/MainWindow.cpp
    # ui/MainWindow.h

    core/graph/Graph.h
    core/graph/Graph.cpp

    core/pathfinding/Dijkstra.h
    core/pathfinding/Dijkstra.cpp

    resources/resources.qrc

    graphics/MapView.h
    graphics/MapView.cpp

    graphics/CharacterItem.h
    graphics/CharacterItem.cpp

    graphics/NpcItem.h
    graphics/NpcItem.cpp

    core/map/Building.h

    core/map/CampusMap.h
    core/map/CampusMap.cpp

    controller/NavigationController.h
    controller/NavigationController.cpp

    ui/MainWindow.h
    ui/MainWindow.cpp

    ui/DialogWidget.h
    ui/DialogWidget.cpp

    database/DatabaseManager.h
    database/DatabaseManager.cpp
)

# 添加可执行文件
add_executable(${PROJECT_NAME} ${PROJECT_SOURCES})

# 链接 Qt 库
target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Sql
)