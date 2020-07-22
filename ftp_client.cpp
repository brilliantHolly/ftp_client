#include<iostream>
#include<string.h>
#include<memory.h>
#include<WinSock2.h>
#pragma comment(lib, "ws2_32.lib") //����ws2_32.dll

using namespace std;

int main() {
	//��ʼ��DLL
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);

	/*//�����׽���
	SOCKET sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

	//���������������
	sockaddr_in sockAddr; //����sockaddr_in�ṹ�����sockAddr
	memset(&sockAddr, 0, sizeof(sockAddr));  
	sockAddr.sin_family = PF_INET; 
	sockAddr.sin_addr.s_addr = inet_addr("127.0.0.1"); 
	sockAddr.sin_port = htons(1234); 
	connect(sock, (SOCKADDR*)&sockAddr, sizeof(SOCKADDR));*/

	/*part1 �ͻ������ӵ�FTP�����������ջ�ӭ��Ϣ*/
	SOCKET control_sock;
	struct hostent *hp;//host entry����д���ýṹ��¼��������Ϣ����������������������ַ���͡���ַ���Ⱥ͵�ַ�б�
	struct sockaddr_in server;
	memset(&server, 0, sizeof(struct sockaddr_in));//ÿ���ֽڶ���0���

	/* ��ʼ��socket */
	control_sock = socket(AF_INET, SOCK_STREAM, 0);
	hp = gethostbyname(server_name); //������������������ȡip��ַ
	memcpy(&server.sin_addr, hp->h_addr, hp->h_length);//�����IP��ַ
	server.sin_family = AF_INET;//ʹ��IPv4��ַ
	//server.sin_port = htons(port);//�˿�
	server.sin_port = 21;//�˿�

	/* ���ӵ��������� */
	connect(control_sock, (struct sockaddr *)&server, sizeof(server));
	/* �ͻ��˽��շ������˵�һЩ��ӭ��Ϣ */
	recv(control_sock, read_buf, read_len, NULL);


	/*part2 �ͻ��˷����û��������룬���� FTP ������*/
	/* ���� ��USER username\r\n�� */
	sprintf(send_buf, "USER %s\r\n", username);
	/*�ͻ��˷����û������������� */
	send(control_sock, send_buf, strlen(send_buf), NULL);
	/* �ͻ��˽��շ���������Ӧ�����Ϣ������Ϊ ��331 User name okay, need password.�� */
	recv(control_sock, read_buf, read_len, NULL);

	/* ���� ��PASS password\r\n�� */
	sprintf(send_buf, "PASS %s\r\n", password);
	/* �ͻ��˷������뵽�������� */
	send(control_sock, send_buf, strlen(send_buf), NULL);
	/* �ͻ��˽��շ���������Ӧ�����Ϣ������Ϊ ��230 User logged in, proceed.�� */
	recv(control_sock, read_buf, read_len, NULL);

	/*part3 �÷��������뱻��ģʽ�������ݶ˿ڼ���*/
	/* ���� ��PASV\r\n�� */
	sprintf(send_buf, "PASV\r\n");
	/* �ͻ��˸��߷������ñ���ģʽ */
	send(control_sock, send_buf, strlen(send_buf), NULL);
	/*�ͻ��˽��շ���������Ӧ����¿��Ķ˿ںţ�
	* ����Ϊ ��227 Entering passive mode (<h1,h2,h3,h4,p1,p2>)�� */
	recv(control_sock, read_buf, read_len, NULL);


	/*part4 �ͻ������ӵ� FTP �����������ݶ˿ڲ������ļ�*/
	SOCKET data_sock;
	data_sock = socket(AF_INET, SOCK_STREAM, 0);
	struct sockaddr_in pasv;
	memset(&pasv, 0, sizeof(struct sockaddr_in));//ÿ���ֽڶ���0���
	memcpy(&pasv.sin_addr, hp->h_addr, hp->h_length);//�����IP��ַ
	pasv.sin_family = AF_INET;//ʹ��IPv4��ַ
	//server.sin_port = htons(port);//�˿�
	pasv.sin_port = htons(x * 256 + y);//�˿�

	


	/* ���ӷ������¿������ݶ˿� */
	connect(data_sock, (struct sockaddr *)&pasv, sizeof(pasv));
	/* ���� ��CWD dirname\r\n�� */
	sprintf(send_buf, "CWD %s\r\n", dirname);
	/* �ͻ��˷�������ı乤��Ŀ¼ */
	send(control_sock, send_buf, strlen(send_buf), NULL);
	/* �ͻ��˽��շ���������Ӧ�����Ϣ������Ϊ ��250 Command okay.�� */
	recv(control_sock, read_buf, read_len, NULL);

	/* ���� ��SIZE filename\r\n�� */
	sprintf(send_buf, "SIZE %s\r\n", filename);
	/* �ͻ��˷�������ӷ������˵õ������ļ��Ĵ�С */
	send(control_sock, send_buf, strlen(send_buf), NULL);
	/* �ͻ��˽��շ���������Ӧ�����Ϣ������Ϊ ��213 <size>�� */
	recv(control_sock, read_buf, read_len, NULL);

	/* ���� ��RETR filename\r\n�� */
	sprintf(send_buf, "RETR %s\r\n", filename);
	/* �ͻ��˷�������ӷ������������ļ� */
	send(control_sock, send_buf, strlen(send_buf), NULL);
	/* �ͻ��˽��շ���������Ӧ�����Ϣ������Ϊ ��150 Opening data connection.�� */
	recv(control_sock, read_buf, read_len, NULL);

	/* �ͻ��˴����ļ� */
	file_handle = open(disk_name, CRFLAGS, RWXALL);
	for (; ; ) {
		... ...
			/* �ͻ���ͨ���������� �ӷ����������ļ����� */
			read(data_sock, read_buf, read_len);
		/* �ͻ���д�ļ� */
		write(file_handle, read_buf, read_len);
		... ...
	}
	/* �ͻ��˹ر��ļ� */
	rc = close(file_handle);

	/*part5 �ͻ��˹ر���������,�˳� FTP ���������رտ�������*/

	/* �ͻ��˹ر��������� */
	close(data_sock);
	/* �ͻ��˽��շ���������Ӧ�����Ϣ������Ϊ ��226 Transfer complete.�� */
	recv(control_sock, read_buf, read_len, NULL);

	/* ���� ��QUIT\r\n�� */
	sprintf(send_buf, "QUIT\r\n");
	/* �ͻ��˽��Ͽ���������˵����� */
	send(control_sock, send_buf, strlen(send_buf), NULL);
	/* �ͻ��˽��շ���������Ӧ�룬����Ϊ ��200 Closes connection.�� */
	recv(control_sock, read_buf, read_len, NULL);
	/* �ͻ��˹رտ������� */
	close(control_sock);





/*//���շ��������ص�����
	char szBuffer[MAXBYTE] = { 0 };
	recv(sock, szBuffer, MAXBYTE, NULL);

	//������յ�������
	printf("Message form server: %s\n", szBuffer);
*/
	


	/*part6  ������ģʽ�� FTP �����������ļ���ʾ�� C ����*/
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

	/* �ͻ��˿�ʼ�����˿�p1*256+p2 */
	listen(server_sock, 64);

	/* ���� ��PORT \r\n�� */
	sprintf(send_buf, "PORT 1287,0,0,1,%d,%d\r\n", p1, p2);
	write(control_sock, send_buf, strlen(send_buf));
	/* �ͻ��˽��շ���������Ӧ�����Ϣ������Ϊ ��200 Port command successful�� */
	read(control_sock, read_buf, read_len);

	sprintf(send_buf, "RETR filename.txt\r\n");
	write(control_sock, send_buf, strlen(send_buf));
	/* �ͻ��˽��շ���������Ӧ�����Ϣ������Ϊ ��150 Opening data channel for file transfer.�� */
	read(control_sock, read_buf, read_len);

	/* ftp�ͻ��˽��ܷ������˵��������� */
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

	/*part7 �� FTP �������ϵ����������ļ�*/
	... ...
		/* ���� ��REST offset\r\n�� */
		sprintf(send_buf, "REST %ld\r\n", offset);
	/* �ͻ��˷�������ָ�������ļ���ƫ���� */
	write(control_sock, send_buf, strlen(send_buf));
	/* �ͻ��˽��շ���������Ӧ�����Ϣ��
	*����Ϊ ��350 Restarting at <position>. Send STORE or RETRIEVE to initiate transfer.�� */
	read(control_sock, read_buf, read_len);
	... ...

		/* ���� ��RETR filename\r\n�� */
		sprintf(send_buf, "RETR %s\r\n", filename);
	/* �ͻ��˷�������ӷ������������ļ�, �����������ļ���ǰoffset�ֽ�*/
	write(control_sock, send_buf, strlen(send_buf));
	/* �ͻ��˽��շ���������Ӧ�����Ϣ��*
	*����Ϊ ��150 Connection accepted, restarting at offset <position>�� */
	read(control_sock, read_buf, read_len);
	... ...

		file_handle = open(disk_name, CRFLAGS, RWXALL);
	/* ָ���ļ�д��ĳ�ʼλ�� */
	lseek(file_handle, offset, SEEK_SET);
	... ...








	//�ر��׽���
	closesocket(sock);

	//��ֹʹ�� DLL
	WSACleanup();

	system("pause");
	return 0;
}


