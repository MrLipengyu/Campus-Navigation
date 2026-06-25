#include "MainWindow.h"
#include <QWidget>
#include <QFont>
#include <QDebug>

namespace ui {

MainWindow::MainWindow(const core::CampusMap& campusMap, QWidget *parent)
    : QMainWindow(parent), m_campusMap(campusMap) {

    setWindowTitle("天津理工大学 - 校园智能导航系统 v1.0");
    resize(1400, 850);

    setupUI();
    setupSearchCompleter(); // 初始化搜索补全器

    m_navCtrl = new controller::NavigationController(m_campusMap, *m_mapView, this);

    // 绑定地图点击 -> 更新右侧面板
    connect(m_mapView, &graphics::MapView::buildingClicked,
            this, &MainWindow::updateInfoPanel);

    // 🌟 核心绑定：将 MainWindow 发出的起终点设置信号，连接到 Controller 的对应槽函数
    connect(this, &MainWindow::startBuildingSelected,
            m_navCtrl, &controller::NavigationController::setStartNode);
    connect(this, &MainWindow::endBuildingSelected,
            m_navCtrl, &controller::NavigationController::setEndNode);
}

void MainWindow::setupUI() {
    QWidget* centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    QHBoxLayout* mainLayout = new QHBoxLayout(centralWidget);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    mainLayout->setSpacing(15);

    // ================= 1: 左侧：系统与环境面板 (新增) =================
    QVBoxLayout* leftPanelLayout = new QVBoxLayout();

    m_envGroup = new QGroupBox("系统与环境设置", this);
    m_envGroup->setFont(QFont("Microsoft YaHei", 10, QFont::Bold));
    m_envGroup->setStyleSheet("QGroupBox { border: 1px solid #bdc3c7; border-radius: 5px; margin-top: 10px; padding-top: 15px; } "
                              "QGroupBox::title { subcontrol-origin: margin; left: 10px; padding: 0 3px 0 3px; color: #34495e; }");

    // 组装内部单选按钮
    QVBoxLayout* envLayout = new QVBoxLayout(m_envGroup);
    envLayout->setSpacing(15);

    m_radioWalk = new QRadioButton("🚶 步行模式 (正常)", this);
    m_radioRun = new QRadioButton("🏃 奔跑模式 (加速)", this);
    m_radioWalk->setFont(QFont("Microsoft YaHei", 10));
    m_radioRun->setFont(QFont("Microsoft YaHei", 10));

    // 默认选中步行
    m_radioWalk->setChecked(true);

    envLayout->addWidget(m_radioWalk);
    envLayout->addWidget(m_radioRun);
    envLayout->addStretch(); // 把按钮往上顶

    leftPanelLayout->addWidget(m_envGroup);
    // 后续可以继续在这里添加：天气系统 GroupBox、昼夜系统 GroupBox 等等...
    leftPanelLayout->addStretch(); // 整体往上顶

    // ================= 2: 中间：地图区域 =================
    m_mapView = new graphics::MapView(m_campusMap, this);

    // ================= 3: 右侧：信息与控制面板 =================
    QVBoxLayout* rightPanelLayout = new QVBoxLayout();

    // 1. 🔍 搜索模块
    QHBoxLayout* searchLayout = new QHBoxLayout();
    m_searchBox = new QLineEdit(this);
    m_searchBox->setPlaceholderText("请输入建筑名称搜索...");
    m_searchBox->setMinimumHeight(35);
    searchLayout->addWidget(m_searchBox);
    rightPanelLayout->addLayout(searchLayout);

    // 2. 建筑名称标题
    m_buildingNameLabel = new QLabel("欢迎来到天津理工大学", this);
    QFont titleFont("Microsoft YaHei", 16, QFont::Bold);
    m_buildingNameLabel->setFont(titleFont);
    m_buildingNameLabel->setAlignment(Qt::AlignCenter);
    m_buildingNameLabel->setStyleSheet("color: #2c3e50; margin-top: 15px; margin-bottom: 10px;");
    rightPanelLayout->addWidget(m_buildingNameLabel);

    // 3. 建筑详细信息展示框
    m_buildingInfoText = new QTextBrowser(this);
    m_buildingInfoText->setFont(QFont("Microsoft YaHei", 11));
    m_buildingInfoText->setHtml("<b>请在左侧地图中点击建筑，或使用上方搜索框。</b>");
    m_buildingInfoText->setStyleSheet("background-color: #f8f9fa; border: 1px solid #dee2e6; border-radius: 5px; padding: 10px;");
    rightPanelLayout->addWidget(m_buildingInfoText);

    // 4. 🚩 交互按钮与状态显示模块
    QVBoxLayout* controlLayout = new QVBoxLayout(); // 使用垂直布局包住按钮和状态

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

    // 新增：状态显示标签
    m_lblCurrentStart = new QLabel("<b>当前起点：</b> 未设置", this);
    m_lblCurrentEnd = new QLabel("<b>当前终点：</b> 未设置", this);
    m_lblCurrentStart->setStyleSheet("color: #d35400; font-size: 14px; margin-top: 10px;");
    m_lblCurrentEnd->setStyleSheet("color: #2980b9; font-size: 14px;");

    controlLayout->addWidget(m_lblCurrentStart);
    controlLayout->addWidget(m_lblCurrentEnd);

    // 把整个控制区加入右侧主面板
    rightPanelLayout->addLayout(controlLayout);

    // 🌟 绑定按钮点击信号到内部槽函数
    connect(m_btnSetStart, &QPushButton::clicked, this, &MainWindow::onBtnSetStartClicked);
    connect(m_btnSetEnd, &QPushButton::clicked, this, &MainWindow::onBtnSetEndClicked);

    // =================4: 组装三栏布局 =================
    // 设置比例： 左侧占 1.5 份，中间占 6 份，右侧占 2 份。你可以根据实际视觉效果微调！
    mainLayout->addLayout(leftPanelLayout, 1);
    mainLayout->addWidget(m_mapView, 6);
    mainLayout->addLayout(rightPanelLayout, 2);

    // =================5: 绑定左侧按钮逻辑 =================
    // 🌟 核心绑定：当单选按钮状态改变时，通知 MapView 改变角色速度
    connect(m_radioWalk, &QRadioButton::toggled, this, [this](bool checked){
        if (checked) {
            m_mapView->setCharacterSpeed(1.5); // 步行速度，可根据你的帧率手感微调
        }
    });

    connect(m_radioRun, &QRadioButton::toggled, this, [this](bool checked){
        if (checked) {
            m_mapView->setCharacterSpeed(3.0); // 奔跑速度设定为步行的两倍
        }
    });
}

void MainWindow::setupSearchCompleter() {
    // 1. 提取所有建筑名称，放入 QStringList
    QStringList buildingNames;
    const auto& buildings = m_campusMap.getAllBuildings();
    for (const auto& [id, b] : buildings) {
        buildingNames << QString::fromStdString(b.name);
    }

    // 2. 初始化 QCompleter
    m_completer = new QCompleter(buildingNames, this);
    m_completer->setCaseSensitivity(Qt::CaseInsensitive); // 不区分大小写
    m_completer->setFilterMode(Qt::MatchContains);        // 模糊匹配（包含关键字即可）

    // 3. 将补全器绑定到搜索框
    m_searchBox->setCompleter(m_completer);

    // ================== 🌟 修复后的信号绑定部分 ==================

    // 交互场景 1：用户在 QCompleter 的下拉列表中点击了某一项
    // 由于 activated 是重载信号，使用 qOverload<const QString&> 来明确指定我们需要带文本参数的版本
    connect(m_completer, qOverload<const QString&>(&QCompleter::activated),
            this, &MainWindow::onSearchTriggered);

    // 交互场景 2：用户在输入框内输完文字后，直接敲击了回车键
    // 我们利用 C++11 的 Lambda 表达式，捕获 this 指针，获取当前输入框的文本并触发搜索
    connect(m_searchBox, &QLineEdit::returnPressed, this, [this]() {
        onSearchTriggered(m_searchBox->text());
    });
}

void MainWindow::onSearchTriggered(const QString& buildingName) {
    // 遍历字典，通过名字反查 ID (因为 Map 的 Key 是 ID)
    const auto& buildings = m_campusMap.getAllBuildings();
    for (const auto& [id, b] : buildings) {
        if (QString::fromStdString(b.name) == buildingName) {
            // 找到对应的建筑，相当于模拟了一次鼠标点击
            updateInfoPanel(id);

            // 🌟 扩展体验：让地图自动滚动/居中到该建筑的位置
            m_mapView->centerOn(b.ui_x, b.ui_y);
            return;
        }
    }
}

void MainWindow::updateInfoPanel(int buildingId) {
    const core::Building* building = m_campusMap.getBuilding(buildingId);
    if (!building) return;

    // 记录当前选中的 ID
    m_currentSelectedBuildingId = buildingId;

    m_buildingNameLabel->setText(QString::fromStdString(building->name));

    QString infoStr = QString::fromStdString(building->info);
    infoStr.replace("\n", "<br><br>");
    infoStr.replace("名称：", "<b>【名称】</b>：");
    infoStr.replace("简介：", "<b>【简介】</b>：");
    infoStr.replace("开放时间：", "<b>【开放时间】</b>：");

    m_buildingInfoText->setHtml(infoStr);

    // 激活下方的起终点按钮
    m_btnSetStart->setEnabled(true);
    m_btnSetEnd->setEnabled(true);
}

void MainWindow::onBtnSetStartClicked() {
    if (m_currentSelectedBuildingId != -1) {
        // 1. 更新 UI 状态显示
        const core::Building* b = m_campusMap.getBuilding(m_currentSelectedBuildingId);
        m_lblCurrentStart->setText("<b>当前起点：</b> " + QString::fromStdString(b->name));

        // 2. 发射信号通知 Controller 执行逻辑
        emit startBuildingSelected(m_currentSelectedBuildingId);
    }
}

void MainWindow::onBtnSetEndClicked() {
    if (m_currentSelectedBuildingId != -1) {
        // 1. 更新 UI 状态显示
        const core::Building* b = m_campusMap.getBuilding(m_currentSelectedBuildingId);
        m_lblCurrentEnd->setText("<b>当前终点：</b> " + QString::fromStdString(b->name));

        // 2. 发射信号通知 Controller 执行逻辑
        emit endBuildingSelected(m_currentSelectedBuildingId);
    }
}

} // namespace ui