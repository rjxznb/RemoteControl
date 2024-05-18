#include "pch.h"
#include "Client_sock.h"

Client_sock::Client_sock()
{
    sock_server = INVALID_SOCKET;
    wVersionRequested = MAKEWORD(2, 2);
    if (WSAStartup(wVersionRequested, &wsaData) != 0) {
        MessageBoxW(NULL, L"加载套接字库错误", L"错误", MB_OK | MB_ICONWARNING);
        exit(-1);
    }
}

std::unique_ptr<Client_sock>& Client_sock::getinstance()
{
    if (client_sock == nullptr) {
        auto temp = new Client_sock;
        client_sock.reset(temp);
    }
    return client_sock;
}

bool Client_sock::init_socket(DWORD ip, unsigned short port)
{
    if(sock_server == INVALID_SOCKET)
    closesocket(sock_server);
    
    sock_server = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_server == -1) {
        TRACE("socket wrong:%d", WSAGetLastError());
        return false;
    }
    sockaddr_in addr1;
    memset(&addr1, 0, sizeof(addr1)); // 要清零；
    addr1.sin_family = AF_INET;
    addr1.sin_port = htons(port);
    addr1.sin_addr.S_un.S_addr = htonl(ip);

    int flag1 = connect(sock_server, (sockaddr*)&addr1, sizeof(addr1));
    if (flag1) {
        TRACE("connect wrong:%d", WSAGetLastError());
        return false;
    }

    return true;
}

SOCKET& Client_sock::get_socket()
{
    return sock_server;
}

void Client_sock::close_socket()
{
    closesocket(sock_server);
    sock_server = INVALID_SOCKET;
}

// 获取包的数据部分内容，并以packet类中存储全部包展开后的string类成员对象packet_data作为缓冲区，之后拆包，返回数据部分内容data成员变量；出错返回"-1"；
std::string Client_sock::recv_message()
{
    packet.packet_data.clear(); // 先对之前用过的包数据缓冲区进行清理；
    packet.packet_data.resize(6); // 先获取包头，查看数据长度，以此来申请缓冲区大小；
    char* buf = packet.packet_data.data(); // 获取堆区new的存数据缓冲区的首地址；
    int len = recv(sock_server, buf, 6, 0); // 考虑数据太大，如果固定缓冲区大小，一次收不完的情况，就先读取大小再申请缓冲区；
    TRACE("client recv packet head length %d", len);
    // 服务器发完啦，连接正常断开，或者是该命令不返回数据；如果对方断连，recv函数会认为对方发送了一个空的关闭通知，然后len=0；
    if (len == 0) {
        return "";
    }
    // recv异常；
    if (len < 0) { 
        return "-1"; }
    // 如果校验位对，就看长度，按长度分配缓冲区；
    DWORD packet_length = 0; // 用于获取该包的数据长度；
    if (*(WORD*)(buf) == 0xFEFF) { // 如果前两个字节是校验位，就读后面四个字节表示的数据长度，如果不是就表示读取包格式不对，按照默认长度申请缓冲区，之后再解包的时候会报错；
        packet_length = *(DWORD*)(buf + 2); // 按照长度大小扩展缓冲区；
    }
    packet.packet_data.resize(6 + packet_length);
    buf = packet.packet_data.data(); // resize之后，缓冲区首地址可能会改变；
    // 将len定义为实际累积读了多长；
    len = 0;
    // 如果实际recv的长度不足我们前面读的包头存储的数据长度，那么就循环读，知道读到包长的数据量为止；
    while (len < packet_length) {
        int once_recv_len = recv(sock_server, buf + 6 + len, packet_length - len, 0);
        len += once_recv_len;
    }
    len += 6;
    packet = CPacket((BYTE*)buf, len); // 解包；
    TRACE("client recv data length %d", len);
    if (len > 0) { // 数据有效;
        return packet.data;
    }
    return "-1";
}

bool Client_sock::send_message(const char* buf, int len)
{
    int size = send(sock_server, buf, len, 0);
    if (size <= 0) return false;
    return true;
}

// 调用上面函数来发送一个展开string数据后的包；
bool Client_sock::send_message(CPacket& packet)
{
    // 如果命令不在范围内；
    if (packet.cmd <= 0 || packet.cmd >= 11) {
        MessageBox(NULL, _T("命令号不在有效范围内"), L"执行命令失败", MB_OK | MB_ICONERROR);
        TRACE("Wrong Cmd:%d, cmd number should among 1 - 11\n", packet.cmd);
        return false;
    }
    return send_message(packet.get_packet_data(), packet.pack_size());
}

bool Client_sock::get_filepath(std::string& filepath) // filepath是传出参数；
{
    if (((this->packet).cmd >= 2 && (this->packet).cmd <= 4) || (this->packet).cmd == 9 || (this->packet).cmd == 10) {
        filepath = (this->packet).data; // 直接通过string赋值，新的string的数据也会new一块新的空间存数据，而不是直接将指针指向原始的string所存储的数据；
        return true;
    }
    return false;
}

bool Client_sock::get_mouse_event(struct Mouse_Event& mouse)
{
    if (this->packet.cmd == 5) {
        mouse = *(struct Mouse_Event*)((this->packet).data.data());
        return true;
    }
    return false;
}

bool Client_sock::send_message(CPacket&& packet)
{
    return send_message(packet.get_packet_data(), packet.pack_size());
}

Client_sock::~Client_sock()
{
    closesocket(sock_server); // 在析构的时候关闭；
    WSACleanup();
}

CPacket::CPacket() :head{ 0 }, length{ 0 }, cmd{ 0 }, sum{ 0 }
{
}
// 封装：pdata是c风格字符串而不是string结构体；
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
        if (*(WORD*)(pdata + i) == 0xFEFF) { // 逐位读WORD大小的数据，直到找到固定位，滑动窗口的思想；
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
    length = *(DWORD*)(pdata + i);
    i += 4;
    if (length + i > size) { // 包未完全接受到；
        size = 0;
        return;
    }
    // 读命令：
    cmd = *(WORD*)(pdata + i);
    i += 2;
    // 读数据：因为数据是const char*类型，我们用string容器作为缓冲区存储数据；
    if (length > 4) { // 如果有数据才读，cmd和sum各两个字节；
        data.resize(length - 4); // 通过memcpy赋值前，需要先resize为足够大的缓冲区；
        memcpy(data.data(), pdata + i, length - 4);
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
    this->cmd = packet.cmd;
    this->data = packet.data;
    this->sum = packet.sum;
}

CPacket& CPacket::operator=(const CPacket& packet) {
    if (&packet == this) {
        return *this;
    }
    this->head = packet.head;
    this->length = packet.length;
    this->cmd = packet.cmd;
    this->data = packet.data;
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
    return length + 2 + 4; // 把数据展开后全部包的大小；
}
