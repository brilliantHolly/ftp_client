#include<iostream>
#include<string.h>
#include<memory.h>
#include<WinSock2.h>
#pragma comment(lib, "ws2_32.lib") //加载ws2_32.dll

using namespace std;

int main() {
	//初始化DLL
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);

	/*//创建套接字
	SOCKET sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

	//向服务器发起请求
	sockaddr_in sockAddr; //创建sockaddr_in结构体变量sockAddr
	memset(&sockAddr, 0, sizeof(sockAddr));  
	sockAddr.sin_family = PF_INET; 
	sockAddr.sin_addr.s_addr = inet_addr("127.0.0.1"); 
	sockAddr.sin_port = htons(1234); 
	connect(sock, (SOCKADDR*)&sockAddr, sizeof(SOCKADDR));*/

	/*part1 客户端连接到FTP服务器，接收欢迎信息*/
	SOCKET control_sock;
	struct hostent *hp;//host entry的缩写，该结构记录主机的信息，包括主机名、别名、地址类型、地址长度和地址列表
	struct sockaddr_in server;
	memset(&server, 0, sizeof(struct sockaddr_in));//每个字节都用0填充

	/* 初始化socket */
	control_sock = socket(AF_INET, SOCK_STREAM, 0);
	hp = gethostbyname(server_name); //用域名或者主机名获取ip地址
	memcpy(&server.sin_addr, hp->h_addr, hp->h_length);//具体的IP地址
	server.sin_family = AF_INET;//使用IPv4地址
	//server.sin_port = htons(port);//端口
	server.sin_port = 21;//端口

	/* 连接到服务器端 */
	connect(control_sock, (struct sockaddr *)&server, sizeof(server));
	/* 客户端接收服务器端的一些欢迎信息 */
	recv(control_sock, read_buf, read_len, NULL);


	/*part2 客户端发送用户名和密码，登入 FTP 服务器*/
	/* 命令 ”USER username\r\n” */
	sprintf(send_buf, "USER %s\r\n", username);
	/*客户端发送用户名到服务器端 */
	send(control_sock, send_buf, strlen(send_buf), NULL);
	/* 客户端接收服务器的响应码和信息，正常为 ”331 User name okay, need password.” */
	recv(control_sock, read_buf, read_len, NULL);

	/* 命令 ”PASS password\r\n” */
	sprintf(send_buf, "PASS %s\r\n", password);
	/* 客户端发送密码到服务器端 */
	send(control_sock, send_buf, strlen(send_buf), NULL);
	/* 客户端接收服务器的响应码和信息，正常为 ”230 User logged in, proceed.” */
	recv(control_sock, read_buf, read_len, NULL);

	/*part3 让服务器进入被动模式，在数据端口监听*/
	/* 命令 ”PASV\r\n” */
	sprintf(send_buf, "PASV\r\n");
	/* 客户端告诉服务器用被动模式 */
	send(control_sock, send_buf, strlen(send_buf), NULL);
	/*客户端接收服务器的响应码和新开的端口号，
	* 正常为 ”227 Entering passive mode (<h1,h2,h3,h4,p1,p2>)” */
	recv(control_sock, read_buf, read_len, NULL);


	/*part4 客户端连接到 FTP 服务器的数据端口并下载文件*/
	SOCKET data_sock;
	data_sock = socket(AF_INET, SOCK_STREAM, 0);
	struct sockaddr_in pasv;
	memset(&pasv, 0, sizeof(struct sockaddr_in));//每个字节都用0填充
	memcpy(&pasv.sin_addr, hp->h_addr, hp->h_length);//具体的IP地址
	pasv.sin_family = AF_INET;//使用IPv4地址
	//server.sin_port = htons(port);//端口
	pasv.sin_port = htons(x * 256 + y);//端口

	


	/* 连接服务器新开的数据端口 */
	connect(data_sock, (struct sockaddr *)&pasv, sizeof(pasv));
	/* 命令 ”CWD dirname\r\n” */
	sprintf(send_buf, "CWD %s\r\n", dirname);
	/* 客户端发送命令改变工作目录 */
	send(control_sock, send_buf, strlen(send_buf), NULL);
	/* 客户端接收服务器的响应码和信息，正常为 ”250 Command okay.” */
	recv(control_sock, read_buf, read_len, NULL);

	/* 命令 ”SIZE filename\r\n” */
	sprintf(send_buf, "SIZE %s\r\n", filename);
	/* 客户端发送命令从服务器端得到下载文件的大小 */
	send(control_sock, send_buf, strlen(send_buf), NULL);
	/* 客户端接收服务器的响应码和信息，正常为 ”213 <size>” */
	recv(control_sock, read_buf, read_len, NULL);

	/* 命令 ”RETR filename\r\n” */
	sprintf(send_buf, "RETR %s\r\n", filename);
	/* 客户端发送命令从服务器端下载文件 */
	send(control_sock, send_buf, strlen(send_buf), NULL);
	/* 客户端接收服务器的响应码和信息，正常为 ”150 Opening data connection.” */
	recv(control_sock, read_buf, read_len, NULL);

	/* 客户端创建文件 */
	file_handle = open(disk_name, CRFLAGS, RWXALL);
	for (; ; ) {
		... ...
			/* 客户端通过数据连接 从服务器接收文件内容 */
			read(data_sock, read_buf, read_len);
		/* 客户端写文件 */
		write(file_handle, read_buf, read_len);
		... ...
	}
	/* 客户端关闭文件 */
	rc = close(file_handle);

	/*part5 客户端关闭数据连接,退出 FTP 服务器并关闭控制连接*/

	/* 客户端关闭数据连接 */
	close(data_sock);
	/* 客户端接收服务器的响应码和信息，正常为 ”226 Transfer complete.” */
	recv(control_sock, read_buf, read_len, NULL);

	/* 命令 ”QUIT\r\n” */
	sprintf(send_buf, "QUIT\r\n");
	/* 客户端将断开与服务器端的连接 */
	send(control_sock, send_buf, strlen(send_buf), NULL);
	/* 客户端接收服务器的响应码，正常为 ”200 Closes connection.” */
	recv(control_sock, read_buf, read_len, NULL);
	/* 客户端关闭控制连接 */
	close(control_sock);





/*//接收服务器传回的数据
	char szBuffer[MAXBYTE] = { 0 };
	recv(sock, szBuffer, MAXBYTE, NULL);

	//输出接收到的数据
	printf("Message form server: %s\n", szBuffer);
*/
	


	/*part6  用主动模式从 FTP 服务器下载文件的示例 C 程序*/
	... ...
		SOCKET data_sock;
	data_sock = socket(AF_INET, SOCK_STREAM, 0);
	struct  sockaddr_in  name;
	name.sin_family = AF_INET;
	name.sin_addr.s_addr = htons(INADDR_ANY);
	server_port = p1 * 256 + p2;
	length = sizeof(name);
	name.sin_port = htons(server_port);
	bind(server_sock, (struct sockaddr *)&name, length);
	struct  sockaddr_in client_name;
	length = sizeof(client_name);

	/* 客户端开始监听端口p1*256+p2 */
	listen(server_sock, 64);

	/* 命令 ”PORT \r\n” */
	sprintf(send_buf, "PORT 1287,0,0,1,%d,%d\r\n", p1, p2);
	write(control_sock, send_buf, strlen(send_buf));
	/* 客户端接收服务器的响应码和信息，正常为 ”200 Port command successful” */
	read(control_sock, read_buf, read_len);

	sprintf(send_buf, "RETR filename.txt\r\n");
	write(control_sock, send_buf, strlen(send_buf));
	/* 客户端接收服务器的响应码和信息，正常为 ”150 Opening data channel for file transfer.” */
	read(control_sock, read_buf, read_len);

	/* ftp客户端接受服务器端的连接请求 */
	data_sock = accept(server_sock, (struct sockaddr *)&client_name, &length);
	... ...

		file_handle = open(disk_name, ROFLAGS, RWXALL);
	for (; ; ) {
		... ...
			read(data_sock, read_buf, read_len);
		write(file_handle, read_buf, read_len);
		... ...
	}
	close(file_handle);

	/*part7 从 FTP 服务器断点续传下载文件*/
	... ...
		/* 命令 ”REST offset\r\n” */
		sprintf(send_buf, "REST %ld\r\n", offset);
	/* 客户端发送命令指定下载文件的偏移量 */
	write(control_sock, send_buf, strlen(send_buf));
	/* 客户端接收服务器的响应码和信息，
	*正常为 ”350 Restarting at <position>. Send STORE or RETRIEVE to initiate transfer.” */
	read(control_sock, read_buf, read_len);
	... ...

		/* 命令 ”RETR filename\r\n” */
		sprintf(send_buf, "RETR %s\r\n", filename);
	/* 客户端发送命令从服务器端下载文件, 并且跳过该文件的前offset字节*/
	write(control_sock, send_buf, strlen(send_buf));
	/* 客户端接收服务器的响应码和信息，*
	*正常为 ”150 Connection accepted, restarting at offset <position>” */
	read(control_sock, read_buf, read_len);
	... ...

		file_handle = open(disk_name, CRFLAGS, RWXALL);
	/* 指向文件写入的初始位置 */
	lseek(file_handle, offset, SEEK_SET);
	... ...








	//关闭套接字
	closesocket(sock);

	//终止使用 DLL
	WSACleanup();

	system("pause");
	return 0;
}


