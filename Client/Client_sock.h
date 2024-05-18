#pragma once
#include<iostream>
#include<vector>
#include<ws2tcpip.h>
#include<string.h>
#include<list>
#include<atlimage.h>
#include<direct.h>
#include<winsock2.h> // ��������Ҫ����ͷ�ļ���
#pragma comment(lib, "ws2_32.lib") // ����ws2_32.lib�⣻
#pragma pack(1)

// ����¼��ࣺ
struct Mouse_Event {
	Mouse_Event() {
		action = -1;
		button = -1;
		point.x = 0;
		point.y = 0;
	}
	WORD action; // ����:0��̧��:1�����������ƶ���2��
	WORD button; // ���:0���Ҽ�:1���м�:2�����������ƶ���3��
	POINT point; // ������ꣻ
};

// ���ࣺ
// string���������ݴ����ݣ���char*��char[]������������������ݵ�ʱ����ͨ��memset�������dataȫ��ȡ��������
class CPacket {
public:
	WORD head = 0xFEFF; // �̶�λ��
	DWORD length; // data�洢����ʵ���ݴ�С���ټ���cmd��sum��
	WORD cmd; // �û����
	std::string data; // sting������ַ���ָ����Ϊ�������׵�ַ�������ݣ�����������ʱ��ͨ��memcpyȡ�������ڶ������ַ����ݣ�
	WORD sum; // ��У�飻
	std::string packet_data; // �洢��dataչ����ȫ���������ݣ���װ���ݵ�ʱ����ã��ڹ��캯���в��ã�
public:
	CPacket();
	CPacket(WORD cmd, BYTE* pdata, int size); // ���ڷ�װ���ݣ�sizeֻ�����ݲ��ֵĳ��ȣ�
	CPacket(const BYTE* pdata, int& size); // ���ڽ��װ���������ֶ����ݣ������������Ա�ʾ���յ��ı��ĶΣ��͸ñ��Ķεĳ��ȣ�����size�ᱻ��Ϊʵ�ʰ������ս�βλ�÷��أ�Ϊ��ɾ���Ѷ������ݶ���ɾ����������ݣ�ʧ��size��Ϊ0��
	CPacket(const CPacket& packet);
	CPacket& operator=(const CPacket& packet);
	int pack_size(); // ����packet�Ĵ�С������Ĵ�С��4����ͷ��β��Ա��С���ټ��ϰ�string������ͨ��ָ��洢������ȫת��Ϊ��ʵ�ֽڣ�
	const char* get_packet_data(); // �������packetչ����������׵�ַ��
	~CPacket();
};

class Client_sock{
private: // ��������������и�ֵ����Ϊ˽�У���ֹ�ⲿ�ܸ�ֵ��
	// ��ʼ��socket�⣻
	WORD wVersionRequested;
	WSADATA wsaData;
	// ������/�ͻ����׽��֣�
	inline thread_local static SOCKET sock_server = NULL;
	// �洢�յ�����Ϣ��
	CPacket packet;
	// ������������ָ�룻
	inline static std::unique_ptr<Client_sock> client_sock = nullptr; // ��Ȼ���ɷ��ʣ����ǿ��Զ��壬��Ȼ���еľ�̬��Ա���޷��������ϣ�
	// ���캯����ʵ�ֳ�ʼ��socket�⣻
	Client_sock();
public:
	std::list<std::string> packet_data_recv_list;
	static std::unique_ptr<Client_sock>& getinstance(); // ��õ�������
	bool init_socket(DWORD ip, unsigned short port); // ���з������׽��ֵİ󶨺ͼ�����
	SOCKET& get_socket();
	void close_socket();
	std::string recv_message();  // ����Ϣ��
	bool send_message(const char* buf, int size); // ����Ϣ��
	bool send_message(CPacket& packet); // ����װ�����Ϣ����ֵ����
	bool send_message(CPacket&& packet); // ����װ�����Ϣ����ֵ����
	bool get_filepath(std::string& filepath); // ���ͻ��˷�������Ŀ¼��Ϣʱ������packet�����ݣ���Ϊpacket��privateȨ�ޣ��޷�ֱ�Ӷ���data��Ա�洢�����ݣ�
	bool get_mouse_event(struct Mouse_Event& mouse); // ���ͻ��˷�������¼���Ϣʱ����ȡ��ǰ��������¼��ṹ�壻
	~Client_sock(); // �����������ͷ��׽��ֺ�����׽��ֿ⣻
};
