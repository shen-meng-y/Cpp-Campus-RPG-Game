#include "mainwindow.h"
#include "startdialog.h"

#include <QApplication>
#include <QDialog>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    StartDialog startDialog;

    // 只有点击“开始游戏”后才创建和显示主窗口
    if (startDialog.exec() == QDialog::Accepted)
    {
        MainWindow mainWindow;
        mainWindow.show();

        return a.exec();
    }

    // 用户直接关闭开始界面时，程序结束
    return 0;
}