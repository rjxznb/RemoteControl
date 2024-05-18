#include "pch.h"
#include"server_sock.h" // 执行头文件，会先声明其为全局变量，所以生命周期回到全部程序结束；

Server_sock::Server_sock()
{
    sock_server = INVALID_SOCKET;
    sock_client = INVALID_SOCKET;
    wVersionRequested = MAKEWORD(2, 2);
    if (WSAStartup(wVersionRequested, &wsaData) != 0) {
        MessageBoxW(NULL, L"加载套接字库错误", L"错误", MB_OK | MB_ICONWARNING);
        exit(-1);
    }
}

std::unique_ptr<Server_sock>& Server_sock::getinstance()
{
	if (server_sock == nullptr) {
		auto temp = new Server_sock;
		server_sock.reset(temp);
	}
	return server_sock;
}

bool Server_sock::init_socket()
{
    sock_server = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_server == -1) {
        perror("socket");
        return false;
    }
    sockaddr_in addr1;
    memset(&addr1, 0, sizeof(addr1)); // 要清零；
    addr1.sin_family = AF_INET;
    addr1.sin_port = htons(9999);
    addr1.sin_addr.S_un.S_addr = INADDR_ANY; // 该主机所有ip，就是0；

    int flag1 = bind(sock_server, (sockaddr*)&addr1, sizeof(addr1));
    if (flag1) {
        perror("bind");
        return false;
    }

    int flag2 = listen(sock_server, 5);
    if (flag2) {
        perror("listen");
        return false;
    }
    return true;
}

// 因为一个时间段只有一个客户来远程控制，所以就将这个客户端的套接字定义为成员变量，方便后面读写；
bool Server_sock::accept_socket()
{
    // TODO: 在此处插入 return 语句
    sock_client = accept(sock_server, (sockaddr*)&addr_client, &size); // 不阻塞的原因是你的size是空的，应该初始化为sockaddr_in大小；
    if (sock_client == INVALID_SOCKET) {
        perror("accept");
        return false;
    }
    return true;
}

void Server_sock::close_client_socket() {
    closesocket(sock_client);
}

// 读消息，做处理；但是没考虑一次收到的消息超过4096的情况；
WORD Server_sock::recv_message()
{
    char buf[4096];
    int index = 0;
    while(true){
        int len = recv(sock_client, buf+index, sizeof(buf)-index, 0); // 本次存这么多字节，不要覆盖之前没拆封装的数据；
        if (len <= 0) { return -1;} // 连接断开；
        packet = CPacket((BYTE*)buf, len); // 读一个报文段长度；
        index += len;
        len = index;
        
        if (len > 0) { // 如果数据有效，就将读出的数据移除，然后继续拆解缓冲区后面的内容；
            memmove(buf, buf + len, sizeof(buf) - len); // 将后面没读的内容移到前面；
            index -= len; // 更新缓冲区的大小；
            return packet.cmd;
        }
    }
    return -1;
}

bool Server_sock::send_message(const char* buf, int len)
{
    int size = send(sock_client, buf, len, 0);
    if(size <= 0) return false;
    return true;
}

// 调用上面函数来发送一个展开string数据后的包；
bool Server_sock::send_message(CPacket& packet)
{
    return send_message(packet.get_packet_data(), packet.pack_size());
}

bool Server_sock::get_filepath(std::string& filepath) // filepath是传出参数；
{
    if (((this->packet).cmd >= 2 && (this->packet).cmd <= 4) || (this->packet).cmd == 9 || (this->packet).cmd == 10) {
        filepath = (this->packet).data; // 直接通过string赋值，新的string的数据也会new一块新的空间存数据，而不是直接将指针指向原始的string所存储的数据；
        return true;
    }
    return false;
}

SOCKET& Server_sock::get_client_sock()
{
    return sock_client;
}

bool Server_sock::get_mouse_event(struct Mouse_Event& mouse)
{
    if (this->packet.cmd == 5) {
        mouse = *(struct Mouse_Event*)((this->packet).data.data());
        return true;
    }
    return false;
}

bool Server_sock::send_message(CPacket&& packet)
{
    return send_message(packet.get_packet_data(), packet.pack_size());
}

Server_sock::~Server_sock()
{
    closesocket(sock_server); // 在析构的时候关闭；
    WSACleanup();
}

CPacket::CPacket() :head{ 0 }, length{ 0 }, cmd{ 0 }, sum{0}
{
}
// 封装：pdata是c风格字符串而不是string结构体，这里size是数据长度而不是包的长度；
CPacket::CPacket(WORD cmd, BYTE* pdata, int size)
{
    length = size + 4; // 数据长度+4；
    this->cmd = cmd;
    if (size > 0) {
        data.resize(size); // 必须要resize，否则直接memcpy你就越界；
        memcpy(data.data(), pdata, size); // 把字符串赋值进行发包；
    }
    else {
        data.clear();
    }
    sum = 0;
    for (int i = 0; i < size; i++) {
        sum += pdata[i] & 0xFF;
    }
}
// 解包：将整个包存入一个CPacket对象，对方发的是封装后的展开数据的字节流；
CPacket::CPacket(const BYTE* pdata, int& size)
{
    size_t i = 0;
    for (; i < size; i++)
    {
        if (*(WORD*)(pdata+i) == 0xFEFF) { // 逐位读WORD大小的数据，直到找到固定位，滑动窗口的思想；
            i += 2; // du跳过固定位，避免包的长度就是固定为大小，从而在读数据时啥都读不出报错。+2就让其满足下面的条件，直接返回；
            break;
        }
    }
    // 不是满足条件的包就返回；
    if (i >= size) {
        size = 0;
        return;
    }
    // 如果包缺少除数据外的任何内容都表示发送的消息不全就返回；
    if (i + 4 + 2 + 2 > size) {
        size = 0;
        return;
    }
    // 读长度；
    length = *(DWORD*)(pdata+i);
    i += 4;
    if (length + i >size) { // 包未完全接受到；
        size = 0;
        return;
    }
    // 读命令：
    cmd = *(WORD*)(pdata + i);
    i += 2;
    // 读数据：因为数据是const char*类型，我们用string容器作为缓冲区存储数据；
    if (length > 4) { // 如果有数据才读，cmd和sum各两个字节；
        data.resize(length-4, '0'); // 当resize变长时，需要给初值，否则可能会越界；
        memcpy(data.data(), pdata+i, length - 4);
    }
    // 读和校验：
    sum = *(WORD*)(pdata + size - 2);
    WORD recv_sum = 0;
    for (auto c : data) {
        recv_sum += (BYTE)c & 0xFF;
    }
    if (recv_sum == sum) {
        size = i + length - 2; // 包的结束位置在缓冲区的多少字节处；
        return;
    }
    size = 0;
    return;
}

CPacket::CPacket(const CPacket& packet)
{
    this->head = packet.head;
    this->length = packet.length;
    this->data = packet.data;
    this->cmd = packet.cmd;
    this->sum = packet.sum;
}

CPacket& CPacket::operator=(const CPacket& packet){
    if (&packet == this) {
        return *this;
    }
    this->head = packet.head;
    this->length = packet.length;
    this->data = packet.data;
    this->cmd = packet.cmd;
    this->sum = packet.sum;
    return *this;
}

CPacket::~CPacket()
{
}

const char* CPacket::get_packet_data()
{
    packet_data.resize(length + 6); // 如果CPacket是一个右值引用，那么这里resize申请空间和下面的memcpy会出错，所以删除右值版本的send_message；
    BYTE* pdata = (BYTE*)packet_data.data();
    *(WORD*)pdata = head;
    *(DWORD*)(pdata + 2) = length;
    *(WORD*)(pdata + 6) = cmd;
    memcpy((pdata + 8), data.data(), length - 4);
    *(WORD*)(pdata + packet_data.size() - 2) = sum;
    return packet_data.data();
}

int CPacket::pack_size()
{
    return length+2+4; // 把数据展开后全部包的大小；
}
