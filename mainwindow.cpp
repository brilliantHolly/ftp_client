#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <errno.h>
#include <fcntl.h>
//#include <netdb.h>
//#include <netinet/in.h> // for sockaddr_in
#include <stdio.h>    // for printf
#include <stdlib.h>   // for exit
#include <string.h>  // for bzero
//#include <strings.h>
//#include <sys/socket.h> // for socket
#include <sys/stat.h>
#include <sys/types.h>
//#include <unistd.h>

#include <string>
#include <fstream>
#include <io.h>
#include<iostream>
#include<stdio.h>
#include<memory.h>

//#include <arpa/inet.h>
#include <windows.h>

#define FTP_PRINTF printf
#define MAXBUF 256

#include <QDebug>

//#pragma execution_character_set("utf-8")


//#include<WinSock2.h>
#pragma comment(lib, "ws2_32.lib") //加载ws2_32.dll

#define _CRT_SECURE_NO_WARNINGS



MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setWindowTitle("FTP_client");

    //初始化DLL
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    nServSocket_ = -1;
    pStrLF_      = "\n";
}

/*********************ftp.h开始**********************/
/*
class CommFtp {
public:
    CommFtp();
    ~CommFtp();
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
  //  const char* pUsername_;
   // const char* pPassword_;
};*/
/*********************ftp.h结束**********************/


int prase_code(const char* src)
{
    if (src) {
        FTP_PRINTF("%s\n", src);
        char strCode[4] = { 0 };
        strncpy(strCode, src, 3);
        return atoi(strCode);
    }
    return -1;
}

/*
CommFtp::CommFtp()
{
    //初始化DLL
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    nServSocket_ = -1;
    pStrLF_      = "\n";
}
CommFtp::~CommFtp()
{
    closesocket(nServSocket_);
    nServSocket_ = -1;

    //终止使用 DLL
    WSACleanup();
}*/
/*********************ftp功能开始**********************/
// 连接
int MainWindow::fConnect(const char* host, int port)
{
    int             sock = -1;
    struct hostent* ht   = NULL;
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        return -1;
    }

    if ((ht = gethostbyname(host)) == NULL) {
        return -1;
    }
    struct sockaddr_in servAddr = { 0 };
    memset(&servAddr, 0, sizeof(struct sockaddr_in));
    memcpy(&servAddr.sin_addr.s_addr, ht->h_addr, ht->h_length);
    servAddr.sin_family = AF_INET;
    servAddr.sin_port   = htons(port);

    if (::connect(sock, ( struct sockaddr* )&servAddr, sizeof(struct sockaddr)) == -1) {
        return -1;
    }
    return sock;
}

// Login
int MainWindow::fLogin(int servfd, const char* username, const char* password)
{
    nServSocket_ = servfd;
    if (this->replayLf() != CODE_FTP_SERVICE_OK) {
        return -1;
    }
    if (prase_code(command("USER", username).c_str()) != CODE_FTP_PASSWORD_REQUIRED) {
        return -1;
    }
    // pUsername_ = username;
    if (prase_code(command("PASS", password).c_str()) != CODE_FTP_LOGGED_IN) {
        return -1;
    }
    // pPassword_ = password;
    return 0;
}
// PASV
int MainWindow::fPasv()
{
    std::string recvline = this->command("PASV");
    if (prase_code(recvline.c_str()) < 0) {
        return -1;
    }
    int addr[6];
    sscanf(recvline.c_str(), "%*[^(](%d,%d,%d,%d,%d,%d)", &addr[0], &addr[1], &addr[2], &addr[3], &addr[4], &addr[5]);
    struct sockaddr_in dataAddr = { 0 };
    dataAddr.sin_family         = AF_INET;
    dataAddr.sin_addr.s_addr    = htonl((addr[0] << 24) | (addr[1] << 16) | (addr[2] << 8) | addr[3]);
    dataAddr.sin_port           = htons((addr[4] << 8) | addr[5]);

    int s = socket(AF_INET, SOCK_STREAM, 0);
    if (::connect(s, ( struct sockaddr* )&dataAddr, sizeof(dataAddr)) < 0) {
        perror("ftp: connect");
        return -1;
    }
    return s;
}
// PWD
int MainWindow::fPwd()
{
    std::string recvline = this->command("PWD");
    if (prase_code(recvline.c_str()) != CODE_FTP_OK) {
        return -1;
    }
    int         i               = 0;
    char        currendir[1024] = { 0 };
    const char* ptr             = recvline.c_str() + 5;
    while (*(ptr) != '"') {
        currendir[i++] = *(ptr);
        ptr++;
    }
    printf("Dir is:%s\n", currendir);
    return 0;
}
// Ls
int MainWindow::fLs()
{
    int ds = this->fPasv();
    if (ds <= 0) {
        return -1;
    }
    if (prase_code(command("LIST").c_str()) != CODE_FTP_DATA_CON_OPENED) {
        return -1;
    }
    for (;;) {
        char buffer[BUFSIZ] = { 0 };
        int nread = recv(ds, buffer, BUFSIZ, 0);
        if (nread <= 0) {
            break;
        }
      //  FTP_PRINTF("%s", buffer);
        QString str = buffer;
        QListWidgetItem *item = new QListWidgetItem;
        item->setText(str);
        ui->listW_server->addItem(item);
    }
    closesocket(ds);
    //FTP_PRINTF("%s\n", recvByChar().c_str());
    QString str = recvByChar().c_str();
    QListWidgetItem *item = new QListWidgetItem;
    item->setText(str);
    ui->listW_message->addItem(item);

    return 0;
}
// GET
int MainWindow::fGet(const char* srcfpName, const char* dstfpName)
{
    int ds = this->fPasv();
    if (ds <= 0) {
        return -1;
    }
    if (prase_code(command("RETR", srcfpName).c_str()) != CODE_FTP_DATA_CON_OPENED) {
        return -1;
    }
    int fpHandle = open(dstfpName, O_WRONLY | O_CREAT | O_TRUNC, S_IREAD | S_IWRITE);
    for (;;) {
        char buffer[BUFSIZ] = { 0 };

        int nread = recv(ds, buffer, BUFSIZ, 0);
        if (nread <= 0) {
            break;
        }
        FTP_PRINTF("recv len: %d\n", nread);
        if (write(fpHandle, buffer, nread) != nread) {
            FTP_PRINTF("receive error from server!");
        }
    }
    ::close(fpHandle);
    closesocket(ds);
    //FTP_PRINTF("%s\n", recvByChar().c_str());
    QString str = recvByChar().c_str();
    QListWidgetItem *item = new QListWidgetItem;
    item->setText(str);
    ui->listW_message->addItem(item);

    return 0;
}
// PUT
int MainWindow::fPut(const char* fpName)
{
    int ds = this->fPasv();
    if (ds <= 0) {
        return -1;
    }
    if (prase_code(command("STOR", fpName).c_str()) != CODE_FTP_DATA_CON_OPENED) {
        return -1;
    }
    int fpHandle = open(fpName, O_RDWR);
    for (;;) {
        char buffer[BUFSIZ] = { 0 };

        int nread = read(fpHandle, buffer, BUFSIZ);
        if (nread <= 0) {
            break;
        }
        if (write(ds, buffer, nread) != nread) {
            FTP_PRINTF("receive error from server!");
        }
    }
    ::close(fpHandle);
    closesocket(ds);
    FTP_PRINTF("%s\n", recvByChar().c_str());
    return 0;
}

// CD
int MainWindow::fCd(const char* dir)
{
    std::string recvline = this->command("CWD", dir);
    if (prase_code(recvline.c_str()) != CODE_FTP_OK) {
        return -1;
    }
    return 0;
}

// QUIT
int MainWindow::fQuit()
{
    std::string quit_reply = this->command("QUIT");
    const char* quit_signal = quit_reply.c_str();
    return atoi(quit_signal);
}

// exec
std::string MainWindow::command(const char* code, const char* arg)
{
    char buffer[1024] = { 0 };
    if (arg) {
        sprintf(buffer, "%s %s%s", code, arg, pStrLF_);
    } else {
        sprintf(buffer, "%s%s", code, pStrLF_);
    }
    // FTP_PRINTF("> %s\n", buffer);
    if (send(nServSocket_, buffer, ( int )strlen(buffer), 0) < 0) {
        FTP_PRINTF("tcp send msg: %s error\n", buffer);
        return "";
    }
    return recvByChar();
}
//
std::string MainWindow::recvByChar()
{
    char           replyString[BUFSIZ] = { 0 };
    recv(nServSocket_, replyString, BUFSIZ, 0);
    /*register int   c;
    register char* cp   = replyString;
    qDebug()<<"111111111";
    static FILE*   fpIn = _fdopen(nServSocket_, "r");
    qDebug()<<"2222222";

    while ((c = getc(fpIn)) != '\n') {
        if (cp < &replyString[sizeof(replyString) - 1])
            *cp++ = c;
    }

    fclose(fpIn);*/

    return replyString;
}
//
int MainWindow::replayLf()
{
    char recvline[1024] = { 0 };
    if (recv(nServSocket_, recvline, sizeof(recvline), 0) < 0) {
        return -1;
    }
    FTP_PRINTF("%s\n", recvline);
    if (strstr(recvline, "Microsoft")) {
        pStrLF_ = "\r\n";
    }
    char code[4] = { 0 };
    strncpy(code, recvline, 3);
    return atoi(code);
}
/*********************ftp功能结束**********************/

//初始化一个ftp
//CommFtp ftp;



void MainWindow::message_refresh(QString new_mess)
{
    QListWidgetItem *new_item = new QListWidgetItem;
    new_item->setText(new_mess);
    ui->listW_message->addItem(new_item);
}


//将键入的ip地址、用户名和密码发送给服务器，根据服务器返回的信息判断是否登录成功
void MainWindow::on_button_log_clicked()
{
    //message列表显示
    QString str1 = "begin to connect the server";
    message_refresh(str1);

    // 连接服务器
    std::string ip = ui->lineEdit_ip->text().toStdString();
    const char* ip_addr = ip.c_str();
    int s = fConnect(ip_addr, 21);

    if(s < 0)
    {
        //message列表显示
        QString str4 = "connect failed";
        message_refresh(str4);

        //服务器连接错误的弹窗
        QString dlgTitle="warnning";
        QString strInfo="connect failed";
        QMessageBox::information(this, dlgTitle, strInfo, QMessageBox::Yes);
    }
    else  //使用用户名、密码登录服务器
    {
        //message列表显示
        QString str5 = "connect successfully";
        message_refresh(str5);
        QString str6 = "begin to login";
        message_refresh(str6);


        std::string user = ui->lineEdit_user->text().toStdString();
        const char* userName = user.c_str();
        std::string pwd = ui->lineEdit_pwd->text().toStdString();
        const char* password = pwd.c_str();

        int ftp_login = fLogin(s, userName, password);
        qDebug()<<ftp_login;
        if(ftp_login == 0 )
        {
            //message列表显示
            QString str2 = "login successfully";
            message_refresh(str2);

            //成功登录的弹窗
            QString dlgTitle="welcome";
            QString strInfo="login successfully";
            QMessageBox::information(this, dlgTitle, strInfo, QMessageBox::Yes);

            //清空密码输入框
            ui->lineEdit_pwd->clear();
        }
        else
        {
            //message列表显示
            QString str3 = "login failed";
            message_refresh(str3);

            //登录失败的弹窗
            QString dlgTitle="warnning";
            QString strInfo="username or password maybe wrong";
            QMessageBox::warning(this, dlgTitle, strInfo, QMessageBox::Yes);

            //清空输入框内容
            ui->lineEdit_ip->clear();
            ui->lineEdit_user->clear();
            ui->lineEdit_pwd->clear();

            //光标定位
            ui->lineEdit_ip->setFocus();
        }
    }
}

//从服务器下载在server文件列表中所选中的文件
void MainWindow::on_button_download_clicked()
{
    //message列表显示
    QListWidgetItem *cur_item = ui->listW_server->currentItem();
    QString str1 = cur_item->text();
    QString str2 = "begin to download";
    str2.append(str1);
    message_refresh(str2);

    //文件下载
   // printf("remote-file> ");
    const char* rbuffer;
    rbuffer = str1.toStdString().data();

    const char* lbuffer;
    lbuffer = rbuffer;

   /* scanf("%s", rbuffer);
    printf("local-file> ");
    char lbuffer[MAXBUF] = { 0 };
    scanf("%s", lbuffer);*/
    fGet(rbuffer, lbuffer);

    //message列表显示
    QString str3 = "download successfully";
    QString str4 = "download failed";
    str3.append(str1);
    message_refresh(str3);
}

//刷新server文件列表
void MainWindow::on_button_server_refresh_clicked()
{
    //message列表显示
    QString str1 = "refresh server file list";
    message_refresh(str1);

    //调用socket实现LS
    fLs();

}

//从客户端上传在client文件列表中所选中的文件
void MainWindow::on_button_upload_clicked()
{
    //message列表显示
    QListWidgetItem *cur_item = ui->listW_client->currentItem();
    QString str1 = cur_item->text();
    QString str2 = "begin to upload";
    str2.append(str1);
    message_refresh(str2);

    //文件上传
   // printf("remote-file> ");
    //const char* rbuffer;
    //rbuffer = str1.toStdString().data();

    const char* lbuffer;
    lbuffer = str1.toStdString().data();

   // fPut(lbuffer);


    //message列表显示
    QString str3 = "upload successfully";
    QString str4 = "upload failed";
    str3.append(str1);
    message_refresh(str3);
}

//刷新client文件列表
void MainWindow::on_button_client_refresh_clicked()
{
    //message列表显示
    QString str1 = "refresh client file list";
    message_refresh(str1);
}


MainWindow::~MainWindow()
{
    closesocket(nServSocket_);
    nServSocket_ = -1;

    //终止使用 DLL
    WSACleanup();
    delete ui;
}

