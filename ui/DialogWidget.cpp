#include "DialogWidget.h"
#include <QDebug>
#include <QPainter>
#include <QPainterPath>

namespace ui {

DialogWidget::DialogWidget(QWidget* parent)
    : QWidget(parent), m_lineIndex(0), m_charIndex(0)
{
    // ===== 窗口基本属性 =====
    // 无边框 + 工具窗口风格，不会抢走地图的键盘焦点
    setWindowFlags(Qt::FramelessWindowHint | Qt::Tool);
    setAttribute(Qt::WA_TranslucentBackground); // 背景透明，让我们自己画圆角

    setFixedSize(600, 160);

    // ===== 打字机定时器 =====
    m_typewriterTimer = new QTimer(this);
    m_typewriterTimer->setInterval(45); // 每 45ms 打一个字
    connect(m_typewriterTimer, &QTimer::timeout, this, &DialogWidget::onTypewriterTick);

    // ===== 布局 =====
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(16, 12, 16, 12);
    mainLayout->setSpacing(8);

    // 第一行：NPC 名字标签
    m_nameLabel = new QLabel("???", this);
    m_nameLabel->setFixedHeight(24);

    // 第二行：对话文字
    m_textLabel = new QLabel("", this);
    m_textLabel->setWordWrap(true);
    m_textLabel->setFixedHeight(60);
    m_textLabel->setAlignment(Qt::AlignTop | Qt::AlignLeft);

    // 第三行：底部按钮区
    QHBoxLayout* btnLayout = new QHBoxLayout();
    btnLayout->addStretch();
    m_btnContinue = new QPushButton("继续 ▶", this);
    m_btnContinue->setFixedSize(90, 30);
    btnLayout->addWidget(m_btnContinue);

    mainLayout->addWidget(m_nameLabel);
    mainLayout->addWidget(m_textLabel);
    mainLayout->addLayout(btnLayout);

    connect(m_btnContinue, &QPushButton::clicked, this, &DialogWidget::onContinueClicked);

    // ===== 样式 =====
    applyStyle();

    // 默认隐藏
    hide();
}

void DialogWidget::applyStyle() {
    // 整体面板：深色半透明圆角背景
    setStyleSheet(R"(
        DialogWidget {
            background: transparent;
        }
    )");

    // 用 QSS 给内部容器设置样式（通过 objectName 区分）
    // 我们直接用 QPalette + 子控件 style 来控制
    m_nameLabel->setStyleSheet(
        "color: #c8a0ff;"
        "font-family: 'Microsoft YaHei';"
        "font-size: 13px;"
        "font-weight: bold;"
        "background: transparent;"
    );

    m_textLabel->setStyleSheet(
        "color: #f0f0f0;"
        "font-family: 'Microsoft YaHei';"
        "font-size: 12px;"
        "background: transparent;"
        "line-height: 1.5;"
    );

    m_btnContinue->setStyleSheet(
        "QPushButton {"
        "  background-color: #5a3fa0;"
        "  color: white;"
        "  border: 1px solid #8060cc;"
        "  border-radius: 5px;"
        "  font-family: 'Microsoft YaHei';"
        "  font-size: 12px;"
        "  font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "  background-color: #7050c0;"
        "  border: 1px solid #a080ee;"
        "}"
        "QPushButton:pressed {"
        "  background-color: #3a2070;"
        "}"
    );
}

// 重写 paintEvent，自己画圆角半透明背景
void DialogWidget::paintEvent(QPaintEvent* /*event*/) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // 深色半透明圆角面板
    QPainterPath path;
    path.addRoundedRect(rect(), 12, 12);

    // 填充：深紫蓝色半透明
    painter.fillPath(path, QColor(18, 12, 38, 220));

    // 边框：紫色描边
    painter.setPen(QPen(QColor(100, 60, 200, 180), 1.5));
    painter.drawPath(path);
}

void DialogWidget::startDialog(const QString& npcDisplayName, const QVector<QString>& lines) {
    if (lines.isEmpty()) return;

    m_lines     = lines;
    m_lineIndex = 0;
    m_charIndex = 0;

    m_nameLabel->setText("【" + npcDisplayName + "】");

    show();
    raise(); // 确保显示在最顶层
    showCurrentLine();
}

void DialogWidget::showCurrentLine() {
    if (m_lineIndex >= m_lines.size()) return;

    m_charIndex  = 0;
    m_typingDone = false;
    m_textLabel->setText("");

    m_typewriterTimer->start();
}

void DialogWidget::onTypewriterTick() {
    if (m_lineIndex >= m_lines.size()) return;

    const QString& fullLine = m_lines[m_lineIndex];

    if (m_charIndex < fullLine.length()) {
        m_charIndex++;
        m_textLabel->setText(fullLine.left(m_charIndex));
    } else {
        // 当前行全部打完
        m_typewriterTimer->stop();
        m_typingDone = true;

        // 如果是最后一行，按钮改为"关闭"
        if (m_lineIndex >= m_lines.size() - 1) {
            m_btnContinue->setText("关闭 ✕");
        } else {
            m_btnContinue->setText("继续 ▶");
        }
    }
}

void DialogWidget::onContinueClicked() {
    if (!m_typingDone) {
        // 打字机还在打：直接显示完整当前行（"加速"效果）
        m_typewriterTimer->stop();
        m_textLabel->setText(m_lines[m_lineIndex]);
        m_typingDone = true;

        if (m_lineIndex >= m_lines.size() - 1) {
            m_btnContinue->setText("关闭 ✕");
        } else {
            m_btnContinue->setText("继续 ▶");
        }
        return;
    }

    // 打字机已打完：翻到下一行
    m_lineIndex++;

    if (m_lineIndex >= m_lines.size()) {
        // 所有对话结束
        hide();
        m_btnContinue->setText("继续 ▶"); // 重置按钮文字
        qDebug() << "[DialogWidget] 对话结束";
        emit dialogFinished();
    } else {
        showCurrentLine();
    }
}

} // namespace ui
