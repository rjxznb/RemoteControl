#pragma once
#include"framework.h"
#pragma pack(1)
// 鼠标事件类：
struct Mouse_Event {
	Mouse_Event() {
		action = -1;
		button = -1;
		point.x = 0;
		point.y = 0;
	}
	WORD action; // 按下:0，抬起:1，不操作仅移动：2；
	WORD button; // 左键:0，右键:1，中键:2，不操作仅移动：3；
	POINT point; // 鼠标坐标；
};

// 包类：
// string的作用是暂存数据，比char*和char[]方便操作，真正发数据的时候还是通过memset将里面的data全都取出来发；
class CPacket {
public:
	WORD head = 0xFEFF; // 固定位；
	DWORD length; // data存储的真实数据大小，再加上cmd和sum；
	WORD cmd; // 用户命令；
	std::string data; // sting里面的字符串指针作为缓冲区首地址保存数据，当真正发送时再通过memcpy取出保存在堆区的字符数据；
	WORD sum; // 和校验；
	std::string packet_data; // 存储将data展开后，全部包的内容；封装数据的时候才用，在构造函数中不用；
public:
	CPacket();
	CPacket(WORD cmd, BYTE* pdata, int size); // 用于封装数据，size只是数据部分的长度；
	CPacket(const BYTE* pdata, int& size); // 用于解封装，读出各字段内容；两个参数各自表示接收到的报文段，和该报文段的长度；其中size会被作为实际包的最终结尾位置返回，为了删除已读的数据而不删除后面的数据，失败size设为0；
	CPacket(const CPacket& packet);
	CPacket& operator=(const CPacket& packet);
	int pack_size(); // 整个packet的大小，这里的大小是4个包头包尾成员大小，再加上把string容器中通过指针存储的数据全转换为真实字节；
	const char* get_packet_data(); // 获得整个packet展开后的数据首地址；
	~CPacket();
};
// 服务器网络接口类：
// 初始化套接字库呦；
class Server_sock {
private: // 将构造和析构还有赋值都设为私有，防止外部能赋值；
	// 初始化socket库；
	WORD wVersionRequested;
	WSADATA wsaData;
	// 服务器/客户端套接字；
	SOCKET sock_server;
	inline thread_local static SOCKET sock_client = NULL;
	// 客户端的地址和大小；
	sockaddr_in addr_client;
	int size = sizeof(addr_client);
	// 存储收到的消息；
	CPacket packet;
	// 单例对象智能指针；
	inline static std::unique_ptr<Server_sock> server_sock = nullptr; // 虽然不可访问，但是可以定义，不然所有的静态成员都无法定义啦呦；
	// 构造函数，实现初始化socket库；
	Server_sock();
public:
	static std::unique_ptr<Server_sock>& getinstance(); // 获得单例对象；
	bool init_socket(); // 进行服务器套接字的绑定和监听；
	SOCKET& get_client_sock();
	bool accept_socket(); // 接收客户端的连接；
	void close_client_socket();
	WORD recv_message();  // 收消息；
	bool send_message(const char* buf, int size); // 发消息；
	bool send_message(CPacket& packet); // 发封装后的消息，左值包；
	bool send_message(CPacket&& packet); // 发封装后的消息，右值包；
	bool get_filepath(std::string& filepath); // 当客户端发出访问目录消息时，读出packet的数据，因为packet是private权限，无法直接读出data成员存储的数据；
	bool get_mouse_event(struct Mouse_Event& mouse); // 当客户端发出鼠标事件消息时，获取当前请求鼠标事件结构体；
	~Server_sock(); // 析构函数，释放套接字和清楚套接字库；
}; 