#pragma once
#include <QMainWindow>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QTextBrowser>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>    // 新增：单行输入框
#include <QCompleter>   // 新增：自动补全器
#include <QStringList>  // 新增：字符串列表

#include <QGroupBox>    // 👇 新增
#include <QRadioButton> // 👇 新增
#include "../core/map/CampusMap.h"
#include "../graphics/MapView.h"
#include "../controller/NavigationController.h"

namespace ui {

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(const core::CampusMap& campusMap, QWidget *parent = nullptr);
    ~MainWindow() = default;

// 👇 1. 定义向 Controller 发送指令的信号
signals:
    void startBuildingSelected(int buildingId); // 发送设置起点指令
    void endBuildingSelected(int buildingId);   // 发送设置终点指令

private slots:
    void updateInfoPanel(int buildingId);

    // 👇 新增槽函数：处理搜索框的回车或选中事件
    void onSearchTriggered(const QString& buildingName);

    // 👇 2. 新增：处理内部按钮点击的槽函数
    void onBtnSetStartClicked();
    void onBtnSetEndClicked();

private:
    void setupUI();
    // 👇 新增方法：初始化自动补全器数据
    void setupSearchCompleter();

private:
    const core::CampusMap& m_campusMap;

    graphics::MapView* m_mapView;
    controller::NavigationController* m_navCtrl;

    // 右侧面板 UI 组件
    QLineEdit* m_searchBox;           // 🔍 搜索框
    QCompleter* m_completer;          // 自动补全器

    QLabel* m_buildingNameLabel;
    QTextBrowser* m_buildingInfoText;

    QPushButton* m_btnSetStart;       // 🚩 设为起点按钮
    QPushButton* m_btnSetEnd;         // 🏁 设为终点按钮

    // 👇 3. 新增：用于在界面上显示当前选中的起终点名称
    QLabel* m_lblCurrentStart;
    QLabel* m_lblCurrentEnd;

    // 记录当前信息面板正在展示的建筑 ID
    int m_currentSelectedBuildingId = -1;

    // 👇 新增：左侧环境面板组件
    QGroupBox* m_envGroup;        // “系统与环境”分组框
    QRadioButton* m_radioWalk;    // 步行单选钮
    QRadioButton* m_radioRun;     // 奔跑单选钮
};

} // namespace ui