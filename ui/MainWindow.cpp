#include "MainWindow.h"
#include <QWidget>
#include <QFont>
#include <QDebug>
#include <QFrame>
#include <QDialog>
#include <QTextEdit>
#include <QMessageBox>

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

    // 4. 实例化控制层 (Controller)，将数据模型和视图交给它指挥
    m_navCtrl = new controller::NavigationController(m_campusMap, *m_mapView, this);

    // ================= 核心信号与槽绑定 =================

    // 视图内部绑定：点击地图上的建筑 -> 更新右侧信息面板
    connect(m_mapView, &graphics::MapView::buildingClicked,
            this, &MainWindow::updateInfoPanel);

    // 跨层绑定：点击界面上的设为起点/终点 -> 发送给 Controller
    connect(this, &MainWindow::startBuildingSelected,
            m_navCtrl, &controller::NavigationController::setStartNode);
    connect(this, &MainWindow::endBuildingSelected,
            m_navCtrl, &controller::NavigationController::setEndNode);

    // 跨层绑定：Controller 报告导航结束/重置 -> UI 恢复初始状态
    connect(m_navCtrl, &controller::NavigationController::navigationStateReset,
            this, &MainWindow::onNavigationStateReset);
}

void MainWindow::setupUI() {
    QWidget* centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    // 主布局采用水平布局 (左、中、右)
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

    // 1.1 角色移动模式控制
    m_radioWalk = new QRadioButton("🚶 步行模式 (正常)", this);
    m_radioRun = new QRadioButton("🏃 奔跑模式 (加速)", this);
    m_radioWalk->setFont(QFont("Microsoft YaHei", 10));
    m_radioRun->setFont(QFont("Microsoft YaHei", 10));
    m_radioWalk->setChecked(true); // 默认步行

    envLayout->addWidget(m_radioWalk);
    envLayout->addWidget(m_radioRun);

    // 分割线
    QFrame* line = new QFrame(m_envGroup);
    line->setFrameShape(QFrame::HLine);
    line->setStyleSheet("color: #bdc3c7;");
    envLayout->addWidget(line);

    // 1.2 昼夜滤镜系统控制
    m_checkNightMode = new QCheckBox("🌙 开启夜间模式", this);
    m_checkNightMode->setFont(QFont("Microsoft YaHei", 10, QFont::Bold));
    m_checkNightMode->setStyleSheet("color: #2c3e50; margin-top: 5px;");
    envLayout->addWidget(m_checkNightMode);

    envLayout->addStretch();
    leftPanelLayout->addWidget(m_envGroup);
    leftPanelLayout->addStretch();

    // ================= 2. 中间：地图核心显示区 =================
    m_mapView = new graphics::MapView(m_campusMap, this);

    // ================= 3. 右侧：信息与导航面板 =================
    QVBoxLayout* rightPanelLayout = new QVBoxLayout();

    // 3.1 搜索框模块
    QHBoxLayout* searchLayout = new QHBoxLayout();
    m_searchBox = new QLineEdit(this);
    m_searchBox->setPlaceholderText("🔍 输入建筑名称搜索...");
    m_searchBox->setMinimumHeight(35);
    searchLayout->addWidget(m_searchBox);
    rightPanelLayout->addLayout(searchLayout);

    // 3.2 建筑标题
    m_buildingNameLabel = new QLabel("欢迎来到天津理工大学", this);
    m_buildingNameLabel->setFont(QFont("Microsoft YaHei", 16, QFont::Bold));
    m_buildingNameLabel->setAlignment(Qt::AlignCenter);
    m_buildingNameLabel->setStyleSheet("color: #2c3e50; margin-top: 15px; margin-bottom: 10px;");
    rightPanelLayout->addWidget(m_buildingNameLabel);

    // 3.3 详细信息框 (QTextBrowser 支持富文本)
    m_buildingInfoText = new QTextBrowser(this);
    m_buildingInfoText->setFont(QFont("Microsoft YaHei", 11));
    m_buildingInfoText->setHtml("<b>请在左侧地图中点击建筑，或使用上方搜索框。</b><br><br>"
                                "操作说明：<br>"
                                "1. 第一次点击建筑：设为起点<br>"
                                "2. 第二次点击建筑：设为终点并开始导航<br>"
                                "3. 可以随时使用 WASD 手动漫游校园");
    m_buildingInfoText->setStyleSheet("background-color: #f8f9fa; border: 1px solid #dee2e6; border-radius: 5px; padding: 10px;");
    rightPanelLayout->addWidget(m_buildingInfoText);

    // 3.4 管理员编辑按钮 (数据库修改入口)
    m_btnEditInfo = new QPushButton("✏️ 编辑建筑信息 (管理员)", this);
    m_btnEditInfo->setStyleSheet("color: #27ae60; font-weight: bold; padding: 5px;");
    m_btnEditInfo->setEnabled(false); // 没选中建筑时不可用
    rightPanelLayout->addWidget(m_btnEditInfo);

    // 3.5 导航控制按钮
    QVBoxLayout* controlLayout = new QVBoxLayout();
    QHBoxLayout* btnLayout = new QHBoxLayout();
    m_btnSetStart = new QPushButton("🚩 设为起点", this);
    m_btnSetEnd = new QPushButton("🏁 设为终点", this);
    m_btnSetStart->setMinimumHeight(40);
    m_btnSetEnd->setMinimumHeight(40);
    m_btnSetStart->setEnabled(false);
    m_btnSetEnd->setEnabled(false);

    btnLayout->addWidget(m_btnSetStart);
    btnLayout->addWidget(m_btnSetEnd);
    controlLayout->addLayout(btnLayout);

    // 3.6 状态指示标签
    m_lblCurrentStart = new QLabel("<b>当前起点：</b> 未设置", this);
    m_lblCurrentEnd = new QLabel("<b>当前终点：</b> 未设置", this);
    m_lblCurrentStart->setStyleSheet("color: #d35400; font-size: 14px; margin-top: 10px;");
    m_lblCurrentEnd->setStyleSheet("color: #2980b9; font-size: 14px;");

    controlLayout->addWidget(m_lblCurrentStart);
    controlLayout->addWidget(m_lblCurrentEnd);
    rightPanelLayout->addLayout(controlLayout);

    // ================= 4. 组装三栏比例 =================
    // 比例 1.5 : 6 : 2，确保地图视野最大化
    mainLayout->addLayout(leftPanelLayout, 1);
    mainLayout->addWidget(m_mapView, 6);
    mainLayout->addLayout(rightPanelLayout, 2);

    // ================= 5. UI 内部事件绑定 =================

    // 速度切换
    connect(m_radioWalk, &QRadioButton::toggled, this, [this](bool checked){
        if (checked) m_mapView->setCharacterSpeed(4.0);
    });
    connect(m_radioRun, &QRadioButton::toggled, this, [this](bool checked){
        if (checked) m_mapView->setCharacterSpeed(8.0);
    });

    // 昼夜切换
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

    // 交互按钮
    connect(m_btnSetStart, &QPushButton::clicked, this, &MainWindow::onBtnSetStartClicked);
    connect(m_btnSetEnd, &QPushButton::clicked, this, &MainWindow::onBtnSetEndClicked);
    connect(m_btnEditInfo, &QPushButton::clicked, this, &MainWindow::onBtnEditInfoClicked);
}

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

    // 绑定下拉列表点击事件
    connect(m_completer, qOverload<const QString&>(&QCompleter::activated),
            this, &MainWindow::onSearchTriggered);

    // 绑定回车事件
    connect(m_searchBox, &QLineEdit::returnPressed, this, [this]() {
        onSearchTriggered(m_searchBox->text());
    });
}

void MainWindow::onSearchTriggered(const QString& buildingName) {
    const auto& buildings = m_campusMap.getAllBuildings();
    for (const auto& [id, b] : buildings) {
        if (QString::fromStdString(b.name) == buildingName) {
            updateInfoPanel(id);
            m_mapView->centerOn(b.ui_x, b.ui_y); // 镜头自动追踪
            return;
        }
    }
}

void MainWindow::updateInfoPanel(int buildingId) {
    const core::Building* building = m_campusMap.getBuilding(buildingId);
    if (!building) return;

    m_currentSelectedBuildingId = buildingId;

    // 更新标题和文本内容 (转换换行符为 HTML 换行)
    m_buildingNameLabel->setText(QString::fromStdString(building->name));
    QString infoStr = QString::fromStdString(building->info);
    infoStr.replace("\n", "<br><br>");
    infoStr.replace("名称：", "<b>【名称】</b>：");
    infoStr.replace("简介：", "<b>【简介】</b>：");
    infoStr.replace("开放时间：", "<b>【开放时间】</b>：");
    m_buildingInfoText->setHtml(infoStr);

    // 激活交互按钮
    m_btnSetStart->setEnabled(true);
    m_btnSetEnd->setEnabled(true);
    m_btnEditInfo->setEnabled(true);
}

void MainWindow::onBtnSetStartClicked() {
    if (m_currentSelectedBuildingId != -1) {
        const core::Building* b = m_campusMap.getBuilding(m_currentSelectedBuildingId);
        m_lblCurrentStart->setText("<b>当前起点：</b> " + QString::fromStdString(b->name));
        emit startBuildingSelected(m_currentSelectedBuildingId); // 呼叫 Controller
    }
}

void MainWindow::onBtnSetEndClicked() {
    if (m_currentSelectedBuildingId != -1) {
        const core::Building* b = m_campusMap.getBuilding(m_currentSelectedBuildingId);
        m_lblCurrentEnd->setText("<b>当前终点：</b> " + QString::fromStdString(b->name));
        emit endBuildingSelected(m_currentSelectedBuildingId); // 呼叫 Controller
    }
}

void MainWindow::onNavigationStateReset() {
    // 恢复 UI 初始状态
    m_lblCurrentStart->setText("<b>当前起点：</b> 未设置");
    m_lblCurrentEnd->setText("<b>当前终点：</b> 未设置");
    m_currentSelectedBuildingId = -1;
    m_btnSetStart->setEnabled(false);
    m_btnSetEnd->setEnabled(false);
    m_btnEditInfo->setEnabled(false);
}

void MainWindow::onBtnEditInfoClicked() {
    if (m_currentSelectedBuildingId == -1) return;

    const core::Building* b = m_campusMap.getBuilding(m_currentSelectedBuildingId);
    if (!b) return;

    // 弹出管理员编辑模态框
    QDialog dialog(this);
    dialog.setWindowTitle("管理员编辑模式 - " + QString::fromStdString(b->name));
    dialog.resize(500, 400);

    QVBoxLayout* layout = new QVBoxLayout(&dialog);
    QLabel* hintLabel = new QLabel("请在下方修改建筑详细信息（支持多行文本）：", &dialog);
    layout->addWidget(hintLabel);

    QTextEdit* textEdit = new QTextEdit(&dialog);
    textEdit->setFont(QFont("Microsoft YaHei", 10));
    textEdit->setPlainText(QString::fromStdString(b->info));
    layout->addWidget(textEdit);

    QHBoxLayout* btnLayout = new QHBoxLayout();
    QPushButton* btnSave = new QPushButton("💾 保存修改", &dialog);
    QPushButton* btnCancel = new QPushButton("取消", &dialog);
    btnLayout->addStretch();
    btnLayout->addWidget(btnSave);
    btnLayout->addWidget(btnCancel);
    layout->addLayout(btnLayout);

    connect(btnSave, &QPushButton::clicked, &dialog, &QDialog::accept);
    connect(btnCancel, &QPushButton::clicked, &dialog, &QDialog::reject);

    if (dialog.exec() == QDialog::Accepted) {
        std::string newInfo = textEdit->toPlainText().toStdString();
        // 核心：请求数据层持久化写入 SQLite
        if (m_campusMap.updateBuildingInfo(m_currentSelectedBuildingId, newInfo)) {
            QMessageBox::information(this, "成功", "建筑信息已成功更新至数据库！");
            updateInfoPanel(m_currentSelectedBuildingId); // 刷新界面
        } else {
            QMessageBox::critical(this, "错误", "数据库更新失败！");
        }
    }
}

} // namespace ui