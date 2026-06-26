#include "MainWindow.h"
#include <QWidget>
#include <QFont>
#include <QDebug>
#include <QFrame>
#include <QDialog>
#include <QTextEdit>
#include <QMessageBox>
#include <QResizeEvent>

namespace ui {

MainWindow::MainWindow(core::CampusMap& campusMap, QWidget *parent)
    : QMainWindow(parent), m_campusMap(campusMap) {

    // 1. 初始化窗口基本属性
    setWindowTitle("天津理工大学 - 校园智能导航系统 v1.0");
    resize(1440, 850);

    // 2. 搭建 UI 三栏布局
    setupUI();

    // 3. 初始化搜索框智能补全器
    setupSearchCompleter();

    // 4. 实例化控制层 (Controller)，将数据模型和视图交给它统一指挥
    m_navCtrl = new controller::NavigationController(m_campusMap, *m_mapView, this);

    // ================= 核心信号与槽绑定 =================

    // 视图内部交互：点击地图上的建筑 -> 触发右侧信息面板更新
    connect(m_mapView, &graphics::MapView::buildingClicked,
            this, &MainWindow::updateInfoPanel);

    // 跨层通信：UI 选点完毕 -> 发送给 Controller 进行逻辑处理
    connect(this, &MainWindow::startBuildingSelected,
            m_navCtrl, &controller::NavigationController::setStartNode);
    connect(this, &MainWindow::destBuildingAdded,
            m_navCtrl, &controller::NavigationController::addDestNode);

    // 跨层通信：Controller 逻辑处理完毕 -> UI 接收反馈并刷新
    connect(m_navCtrl, &controller::NavigationController::navigationStateReset,
            this, &MainWindow::onNavigationStateReset);
    connect(m_navCtrl, &controller::NavigationController::routeOrderUpdated,
            this, &MainWindow::onRouteOrderUpdated);

    // ================= NPC 系统初始化 =================

    // 1. 创建悬浮对话框（父对象为 this，自动居中对齐到 mapView 底部）
    m_dialogWidget = new DialogWidget(this);

    // 2. 创建 mad_professor NPC
    //    服务于 「朝阳广场」地标（请根据地图 JSON 数据确认坐标）
    graphics::NpcData teacher;
    teacher.name          = "teacher";
    teacher.displayName   = QString::fromUtf8("老师");
    teacher.position      = QPointF(878, 470);  // 朝阳广场真实坐标 (来自 map_data_fuben.json)
    teacher.triggerRadius = 18.0;               // 80像素触发范围

    auto* madProf = new graphics::NpcItem(teacher);
    m_mapView->addNpc(madProf);

    // 3. 串联信号槽：NPC 触发 → MainWindow 处理 → 弹出对话框
    connect(m_mapView, &graphics::MapView::npcTriggered,
            this, &MainWindow::onNpcTriggered);

    // 4. 对话结束 → 解冻玩家移动
    connect(m_dialogWidget, &DialogWidget::dialogFinished,
            this, &MainWindow::onDialogFinished);
}

void MainWindow::setupUI() {
    QWidget* centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    // 主布局采用水平布局，分为左、中、右三块
    QHBoxLayout* mainLayout = new QHBoxLayout(centralWidget);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    mainLayout->setSpacing(15);

    // ================= 1. 左侧：系统与环境控制面板 =================
    QVBoxLayout* leftPanelLayout = new QVBoxLayout();

    m_envGroup = new QGroupBox("系统与环境设置", this);
    m_envGroup->setFont(QFont("Microsoft YaHei", 10, QFont::Bold));
    m_envGroup->setStyleSheet("QGroupBox { border: 1px solid #bdc3c7; border-radius: 5px; margin-top: 10px; padding-top: 15px; } "
                              "QGroupBox::title { subcontrol-origin: margin; left: 10px; padding: 0 3px 0 3px; color: #34495e; }");

    QVBoxLayout* envLayout = new QVBoxLayout(m_envGroup);
    envLayout->setSpacing(15);

    // 1.1 角色移动模式控制（步行/奔跑）
    m_radioWalk = new QRadioButton("🚶 步行模式 (正常)", this);
    m_radioRun = new QRadioButton("🏃 奔跑模式 (加速)", this);
    m_radioWalk->setFont(QFont("Microsoft YaHei", 10));
    m_radioRun->setFont(QFont("Microsoft YaHei", 10));
    m_radioWalk->setChecked(true); // 默认选中步行

    envLayout->addWidget(m_radioWalk);
    envLayout->addWidget(m_radioRun);

    // 分割线美化
    QFrame* line = new QFrame(m_envGroup);
    line->setFrameShape(QFrame::HLine);
    line->setStyleSheet("color: #bdc3c7;");
    envLayout->addWidget(line);

    // 1.2 昼夜滤镜系统开关
    m_checkNightMode = new QCheckBox("🌙 开启夜间模式", this);
    m_checkNightMode->setFont(QFont("Microsoft YaHei", 10, QFont::Bold));
    m_checkNightMode->setStyleSheet("color: #2c3e50; margin-top: 5px;");
    envLayout->addWidget(m_checkNightMode);

    envLayout->addStretch(); // 保持把按钮往上顶
    leftPanelLayout->addWidget(m_envGroup);
    leftPanelLayout->addStretch();

    // ================= 2. 中间：地图核心显示区 =================
    m_mapView = new graphics::MapView(m_campusMap, this);

    // ================= 3. 右侧：信息与多点导航面板 =================
    QVBoxLayout* rightPanelLayout = new QVBoxLayout();

    // 3.1 智能搜索框
    QHBoxLayout* searchLayout = new QHBoxLayout();
    m_searchBox = new QLineEdit(this);
    m_searchBox->setPlaceholderText("🔍 输入建筑名称快速定位...");
    m_searchBox->setMinimumHeight(35);
    searchLayout->addWidget(m_searchBox);
    rightPanelLayout->addLayout(searchLayout);

    // 3.2 建筑名称大标题
    m_buildingNameLabel = new QLabel("欢迎来到天津理工大学", this);
    m_buildingNameLabel->setFont(QFont("Microsoft YaHei", 16, QFont::Bold));
    m_buildingNameLabel->setAlignment(Qt::AlignCenter);
    m_buildingNameLabel->setStyleSheet("color: #2c3e50; margin-top: 15px; margin-bottom: 10px;");
    rightPanelLayout->addWidget(m_buildingNameLabel);

    // 3.3 详细信息展示框 (QTextBrowser 支持富文本显示)
    m_buildingInfoText = new QTextBrowser(this);
    m_buildingInfoText->setFont(QFont("Microsoft YaHei", 11));
    m_buildingInfoText->setHtml("<b>请在左侧地图中点击建筑，或使用上方搜索框。</b><br><br>"
                                "操作说明：<br>"
                                "1. 选定一个建筑：设为起点<br>"
                                "2. 继续选定其他建筑：添加为途经点（可多个）<br>"
                                "3. 点击【开始多点导航】，系统将自动规划最短遍历路线<br>"
                                "4. 可以随时使用 WASD 手动漫游校园");
    m_buildingInfoText->setStyleSheet("background-color: #f8f9fa; border: 1px solid #dee2e6; border-radius: 5px; padding: 10px;");
    rightPanelLayout->addWidget(m_buildingInfoText);

    // 3.4 数据库管理员编辑按钮
    m_btnEditInfo = new QPushButton("✏️ 编辑建筑信息 (管理员)", this);
    m_btnEditInfo->setStyleSheet("color: #27ae60; font-weight: bold; padding: 5px;");
    m_btnEditInfo->setEnabled(false); // 没选中建筑时不可用
    rightPanelLayout->addWidget(m_btnEditInfo);

    // 3.5 多点导航控制操作区 (TSP算法入口)
    QVBoxLayout* controlLayout = new QVBoxLayout();

    // 第一排：选点操作
    QHBoxLayout* btnLayout1 = new QHBoxLayout();
    m_btnSetStart = new QPushButton("🚩 设为起点", this);
    m_btnAddDest = new QPushButton("📍 添加途经点", this);
    m_btnSetStart->setMinimumHeight(40);
    m_btnAddDest->setMinimumHeight(40);
    m_btnSetStart->setEnabled(false);
    m_btnAddDest->setEnabled(false);
    btnLayout1->addWidget(m_btnSetStart);
    btnLayout1->addWidget(m_btnAddDest);

    // 第二排：导航执行与重置
    QHBoxLayout* btnLayout2 = new QHBoxLayout();
    m_btnStartNav = new QPushButton("🚀 开始多点导航", this);
    m_btnClearNav = new QPushButton("🗑️ 清空重置", this);
    m_btnStartNav->setMinimumHeight(40);
    m_btnClearNav->setMinimumHeight(40);
    m_btnStartNav->setStyleSheet("background-color: #3498db; color: white; font-weight: bold;");
    btnLayout2->addWidget(m_btnStartNav);
    btnLayout2->addWidget(m_btnClearNav);

    controlLayout->addLayout(btnLayout1);
    controlLayout->addLayout(btnLayout2);

    // 3.6 状态与结果指示标签
    m_lblCurrentStart = new QLabel("<b>当前起点：</b> 未设置", this);
    m_lblDestinations = new QLabel("<b>途经点：</b> 0 个", this);
    m_lblDestinations->setWordWrap(true); // 允许多个途经点自动换行

    m_lblCurrentStart->setStyleSheet("color: #d35400; font-size: 14px; margin-top: 10px;");
    m_lblDestinations->setStyleSheet("color: #2980b9; font-size: 14px;");

    controlLayout->addWidget(m_lblCurrentStart);
    controlLayout->addWidget(m_lblDestinations);
    rightPanelLayout->addLayout(controlLayout);

    // ================= 4. 组装三栏比例 =================
    // 比例 1.5 : 6 : 2，确保居中的地图视野最大化
    mainLayout->addLayout(leftPanelLayout, 1);
    mainLayout->addWidget(m_mapView, 6);
    mainLayout->addLayout(rightPanelLayout, 2);

    // ================= 5. UI 内部事件绑定 =================

    // 角色速度切换
    connect(m_radioWalk, &QRadioButton::toggled, this, [this](bool checked){
        if (checked) m_mapView->setCharacterSpeed(2.0); // 步行速度
    });
    connect(m_radioRun, &QRadioButton::toggled, this, [this](bool checked){
        if (checked) m_mapView->setCharacterSpeed(4.0); // 奔跑速度
    });

    // 昼夜滤镜切换
    connect(m_checkNightMode, &QCheckBox::toggled, this, [this](bool checked) {
        m_mapView->setNightMode(checked);
        if (checked) {
            m_checkNightMode->setText("☀️ 切换为日间模式");
            m_checkNightMode->setStyleSheet("color: #f39c12; margin-top: 5px;");
        } else {
            m_checkNightMode->setText("🌙 开启夜间模式");
            m_checkNightMode->setStyleSheet("color: #2c3e50; margin-top: 5px;");
        }
    });

    // 按钮槽函数绑定
    connect(m_btnSetStart, &QPushButton::clicked, this, &MainWindow::onBtnSetStartClicked);
    connect(m_btnAddDest, &QPushButton::clicked, this, &MainWindow::onBtnAddDestClicked);
    connect(m_btnStartNav, &QPushButton::clicked, this, &MainWindow::onBtnStartNavClicked);
    connect(m_btnClearNav, &QPushButton::clicked, this, &MainWindow::onBtnClearNavClicked);
    connect(m_btnEditInfo, &QPushButton::clicked, this, &MainWindow::onBtnEditInfoClicked);
}

// 模糊搜索与自动补全初始化
void MainWindow::setupSearchCompleter() {
    QStringList buildingNames;
    const auto& buildings = m_campusMap.getAllBuildings();
    for (const auto& [id, b] : buildings) {
        buildingNames << QString::fromStdString(b.name);
    }

    m_completer = new QCompleter(buildingNames, this);
    m_completer->setCaseSensitivity(Qt::CaseInsensitive);
    m_completer->setFilterMode(Qt::MatchContains);
    m_searchBox->setCompleter(m_completer);

    // 用户点击下拉提示框中的项
    connect(m_completer, qOverload<const QString&>(&QCompleter::activated),
            this, &MainWindow::onSearchTriggered);

    // 用户直接敲击回车
    connect(m_searchBox, &QLineEdit::returnPressed, this, [this]() {
        onSearchTriggered(m_searchBox->text());
    });
}

// 搜索触发处理
void MainWindow::onSearchTriggered(const QString& buildingName) {
    const auto& buildings = m_campusMap.getAllBuildings();
    for (const auto& [id, b] : buildings) {
        if (QString::fromStdString(b.name) == buildingName) {
            updateInfoPanel(id);
            m_mapView->centerOn(b.ui_x, b.ui_y); // 镜头自动追踪至目标建筑
            return;
        }
    }
}

// 更新右侧信息面板
void MainWindow::updateInfoPanel(int buildingId) {
    const core::Building* building = m_campusMap.getBuilding(buildingId);
    if (!building) return;

    m_currentSelectedBuildingId = buildingId;

    // 1. 更新标题
    m_buildingNameLabel->setText(QString::fromStdString(building->name));

    // 2. 将数据库中的纯文本转换为排版精美的 HTML
    QString infoStr = QString::fromStdString(building->info);
    infoStr.replace("\n", "<br><br>");
    infoStr.replace("名称：", "<b>【名称】</b>：");
    infoStr.replace("简介：", "<b>【简介】</b>：");
    infoStr.replace("开放时间：", "<b>【开放时间】</b>：");
    m_buildingInfoText->setHtml(infoStr);

    // 3. 激活所有受限交互按钮
    m_btnSetStart->setEnabled(true);
    m_btnAddDest->setEnabled(true);
    m_btnEditInfo->setEnabled(true);
}

// ======================== 导航相关槽函数 ========================

void MainWindow::onBtnSetStartClicked() {
    if (m_currentSelectedBuildingId != -1) {
        const core::Building* b = m_campusMap.getBuilding(m_currentSelectedBuildingId);
        if (!b) return; // 防止空指针解引用
        m_lblCurrentStart->setText("<b>当前起点：</b> " + QString::fromStdString(b->name));
        emit startBuildingSelected(m_currentSelectedBuildingId);
    }
}

void MainWindow::onBtnAddDestClicked() {
    if (m_currentSelectedBuildingId != -1) {
        const core::Building* b = m_campusMap.getBuilding(m_currentSelectedBuildingId);
        if (!b) return; // 防止空指针解引用

        // 动态追加显示途经点名称
        QString currentText = m_lblDestinations->text();
        if (currentText.contains("0 个")) {
            m_lblDestinations->setText("<b>已选途经点：</b><br>" + QString::fromStdString(b->name));
        } else {
            m_lblDestinations->setText(currentText + "，" + QString::fromStdString(b->name));
        }

        emit destBuildingAdded(m_currentSelectedBuildingId);
    }
}

void MainWindow::onBtnStartNavClicked() {
    // 呼叫控制器执行 TSP 算法
    m_navCtrl->startMultiNavigation();
}

void MainWindow::onBtnClearNavClicked() {
    // 呼叫控制器清理寻路状态和画布
    m_navCtrl->clearRoute();
}

void MainWindow::onRouteOrderUpdated(const QString& text) {
    // 接收控制器发来的最优路线排列，并在界面上更新显示
    m_lblDestinations->setText(text);
}

void MainWindow::onNavigationStateReset() {
    // 系统被重置时，UI 也必须恢复初始状态
    m_lblCurrentStart->setText("<b>当前起点：</b> 未设置");
    m_lblDestinations->setText("<b>途经点：</b> 0 个");

    m_currentSelectedBuildingId = -1;
    m_btnSetStart->setEnabled(false);
    m_btnAddDest->setEnabled(false);
    m_btnEditInfo->setEnabled(false);
}

// ======================== 管理员数据库编辑 ========================

void MainWindow::onBtnEditInfoClicked() {
    if (m_currentSelectedBuildingId == -1) return;

    const core::Building* b = m_campusMap.getBuilding(m_currentSelectedBuildingId);
    if (!b) return;

    // 1. 创建模态对话框
    QDialog dialog(this);
    dialog.setWindowTitle("管理员编辑模式 - " + QString::fromStdString(b->name));
    dialog.resize(500, 400);

    QVBoxLayout* layout = new QVBoxLayout(&dialog);
    QLabel* hintLabel = new QLabel("请在下方修改建筑详细信息（支持多行文本）：", &dialog);
    layout->addWidget(hintLabel);

    // 2. 将纯文本放入输入框
    QTextEdit* textEdit = new QTextEdit(&dialog);
    textEdit->setFont(QFont("Microsoft YaHei", 10));
    textEdit->setPlainText(QString::fromStdString(b->info));
    layout->addWidget(textEdit);

    // 3. 构建底部按钮区
    QHBoxLayout* btnLayout = new QHBoxLayout();
    QPushButton* btnSave = new QPushButton("💾 保存修改", &dialog);
    QPushButton* btnCancel = new QPushButton("取消", &dialog);
    btnLayout->addStretch();
    btnLayout->addWidget(btnSave);
    btnLayout->addWidget(btnCancel);
    layout->addLayout(btnLayout);

    connect(btnSave, &QPushButton::clicked, &dialog, &QDialog::accept);
    connect(btnCancel, &QPushButton::clicked, &dialog, &QDialog::reject);

    // 4. 等待用户操作并执行数据库持久化
    if (dialog.exec() == QDialog::Accepted) {
        std::string newInfo = textEdit->toPlainText().toStdString();

        // 调用底层核心数据层，将数据持久化写入 SQLite 并在内存中同步
        if (m_campusMap.updateBuildingInfo(m_currentSelectedBuildingId, newInfo)) {
            QMessageBox::information(this, "成功", "建筑信息已成功更新至数据库！");
            updateInfoPanel(m_currentSelectedBuildingId); // 刷新界面以显示新排版
        } else {
            QMessageBox::critical(this, "错误", "数据库更新失败！");
        }
    }
}


// ======================== NPC 对话系统槽函数 ========================

void MainWindow::onNpcTriggered(graphics::NpcItem* npc) {
    if (!m_dialogWidget || !npc) return;

    // 冻结玩家移动
    m_mapView->setTalkingMode(true);

    QVector<QString> lines;
    lines << QString::fromUtf8("「站住！……开玩笑的，我只是太久没人搭理了。」");
    lines << QString::fromUtf8("「这学校是我的领地——当然，保安大叔不这么认为。」");
    lines << QString::fromUtf8("「迷路了吗？用导航，别靠直觉——我见过太多靠直觉绕了半小时的学生。」");
    lines << QString::fromUtf8("「好了，去吧，我继续看风景。」");

    // 定位并弹出对话框
    positionDialogWidget();
    m_dialogWidget->startDialog(npc->data().displayName, lines);
}

void MainWindow::onDialogFinished() {
    m_mapView->setTalkingMode(false);
}

void MainWindow::positionDialogWidget() {
    if (!m_dialogWidget || !m_mapView) return;
    QPoint mapViewPos = m_mapView->mapTo(this, QPoint(0, 0));
    int mapW = m_mapView->width();
    int mapH = m_mapView->height();
    int dlgW = m_dialogWidget->width();
    int dlgH = m_dialogWidget->height();
    int x = mapViewPos.x() + (mapW - dlgW) / 2;
    int y = mapViewPos.y() + mapH - dlgH - 20;
    m_dialogWidget->move(x, y);
}

} // namespace ui
