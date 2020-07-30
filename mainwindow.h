#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMessageBox>
#include <QString>
#include <QtWidgets>
#include <QListWidgetItem>
#include <QDebug>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

const int CODE_FTP_SERVICE_OK = 220;//服务就绪
const int CODE_FTP_PASSWORD_REQUIRED = 331;//要求密码
const int CODE_FTP_LOGGED_IN = 230;//登陆完成
const int CODE_FTP_QUIT = 221;//退出网络
const int CODE_FTP_OK = 257;//路径名建立
const int CODE_FTP_PASV_MODE = 227;//进入被动模式（IP地址、ID端口）
const int CODE_FTP_DATA_CON_OPENED = 125;//打开数据连接，开始传输


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    // 连接
    int fConnect(const char* host, int port);
    // ftp://username:password@192.168.3.6:21
    int fConnect(const char* ftpUrl);
    // Login
    int fLogin(int servfd, const char* username, const char* password);
    // PASV
    int fPasv();
    // PWD
    int fPwd();
    // Ls
    int fLs();
    // GET
    int fGet(const char* srcfpName, const char* dstfpName);
    // PUT
    int fPut(const char* fpName);
    // CD
    int fCd(const char* dir);
    // QUIT
    int fQuit();

private:
    // exec
    std::string command(const char* code, const char* arg = NULL);
    //
    std::string recvByChar();
    //
    int replayLf();
private:
    int         nServSocket_;
    const char* pStrLF_;


public slots:
    //message列表的刷新槽函数
    void message_refresh(QString new_mess);

    void on_button_log_clicked();//点击登录按钮时执行的槽函数

    //server文件列表有关的槽函数
    void on_button_download_clicked();

    void on_button_server_refresh_clicked();

    //client文件列表有关的槽函数
    void on_button_upload_clicked();

    void on_button_client_refresh_clicked();

private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
