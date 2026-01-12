#include "widget.h"
#include "mainwindow.h"
#include "admin_mainwindow.h"

#include <QApplication>
#include <QLocale>
#include <QTranslator>
#include <QFile>
#include <QNetworkProxy>
#include "global.h"

// 用户想重新登录
static const int RETCODE_RESTART = 773;

int main(int argc, char *argv[])
{
    qputenv("QT_AUTO_SCREEN_SCALE_FACTOR", "0");
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    QApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);

    QApplication a(argc, argv);

    // 设置程序图标
    a.setWindowIcon(QIcon(":/image/image/mainIcon.png"));

    // 挂梯子时开启
    QNetworkProxy::setApplicationProxy(QNetworkProxy::NoProxy);

    QString cachePath = QCoreApplication::applicationDirPath() + "/cache";
    QDir dir(cachePath);
    if (!dir.exists()) {
        dir.mkpath(".");
    }

    QFile Qss_file(":/style/dark.qss");
    if(Qss_file.open(QFile::ReadOnly | QFile::Text)){
        QString styleSheet = QLatin1String(Qss_file.readAll()); // 读取所有内容，并转换成string
        a.setStyleSheet(styleSheet);
        Qss_file.close();
    }
    else{
        qDebug() << "open QSS failed!";
    }

    // QTranslator translator;
    // const QStringList uiLanguages = QLocale::system().uiLanguages();
    // for (const QString &locale : uiLanguages) {
    //     const QString baseName = "Sys_ResidentPayment_" + QLocale(locale).name();
    //     if (translator.load(":/i18n/" + baseName)) {
    //         a.installTranslator(&translator);
    //         break;
    //     }
    // }


    int currentExitCode = 0;
    do{
        Widget loginWnd;
        if(loginWnd.exec() == QDialog::Accepted){
            if(loginWnd.m_isUserLogin){
                if(loginWnd.m_mainWin){
                    loginWnd.m_mainWin->show();
                    loginWnd.m_mainWin->raise();
                    loginWnd.m_mainWin->activateWindow();

                    QObject::connect(loginWnd.m_mainWin, &MainWindow::logoutRequested, &a, [&a, &currentExitCode]() {
                        currentExitCode = RETCODE_RESTART;  // 自定义重启码
                        a.exit(currentExitCode);            // 退出当前事件循环
                    });

                    // 进入事件循环，等mainwindow关闭
                    currentExitCode = a.exec();
                }
            }
            else{
                Admin_MainWindow rw;
                rw.show();
                currentExitCode = a.exec();
            }
        }
        else{
            // 用户直接取消登录或者直接从主页叉掉了
            currentExitCode = 0;
            break;
        }
    }while(currentExitCode == RETCODE_RESTART);
    return currentExitCode;
}
