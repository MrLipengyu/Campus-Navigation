#include <QApplication>
#include <QDebug>
// #include "ui/MainWindow.h" // 后续创建 UI 后取消注释

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    qDebug() << "Campus Navigator Initialized.";
    qDebug() << "Running with Qt Version:" << qVersion();

    // MainWindow w;
    // w.show();

    return app.exec();
}