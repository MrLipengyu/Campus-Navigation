#include <QApplication>
#include <windows.h>
#include "core/map/CampusMap.h"
#include "ui/MainWindow.h" // 引入全新的主窗口

int main(int argc, char *argv[]) {
    // 强制控制台输出 UTF-8
    SetConsoleOutputCP(CP_UTF8);

    QApplication app(argc, argv);

    // 1. 加载核心数据模型
    core::CampusMap campus;
    if (!campus.loadFromJson(":/map_data_fuben.json")) {
        qFatal("地图加载失败，请检查文件路径！");
    }

    // 2. 实例化并显示主窗口 (依赖注入思想：把数据交给 UI)
    ui::MainWindow mainWindow(campus);
    mainWindow.show();

    // 3. 进入 Qt 事件循环
    return app.exec();
}