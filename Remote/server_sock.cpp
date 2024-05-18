#include "pch.h"
#include"server_sock.h" // ִ��ͷ�ļ�������������Ϊȫ�ֱ����������������ڻص�ȫ�����������

Server_sock::Server_sock()
{
    sock_server = INVALID_SOCKET;
    sock_client = INVALID_SOCKET;
    wVersionRequested = MAKEWORD(2, 2);
    if (WSAStartup(wVersionRequested, &wsaData) != 0) {
        MessageBoxW(NULL, L"�����׽��ֿ����", L"����", MB_OK | MB_ICONWARNING);
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
    memset(&addr1, 0, sizeof(addr1)); // Ҫ���㣻
    addr1.sin_family = AF_INET;
    addr1.sin_port = htons(9999);
    addr1.sin_addr.S_un.S_addr = INADDR_ANY; // ����������ip������0��

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

// ��Ϊһ��ʱ���ֻ��һ���ͻ���Զ�̿��ƣ����Ծͽ�����ͻ��˵��׽��ֶ���Ϊ��Ա��������������д��
bool Server_sock::accept_socket()
{
    // TODO: �ڴ˴����� return ���
    sock_client = accept(sock_server, (sockaddr*)&addr_client, &size); // ��������ԭ�������size�ǿյģ�Ӧ�ó�ʼ��Ϊsockaddr_in��С��
    if (sock_client == INVALID_SOCKET) {
        perror("accept");
        return false;
    }
    return true;
}

void Server_sock::close_client_socket() {
    closesocket(sock_client);
}

// ����Ϣ������������û����һ���յ�����Ϣ����4096�������
WORD Server_sock::recv_message()
{
    char buf[4096];
    int index = 0;
    while(true){
        int len = recv(sock_client, buf+index, sizeof(buf)-index, 0); // ���δ���ô���ֽڣ���Ҫ����֮ǰû���װ�����ݣ�
        if (len <= 0) { return -1;} // ���ӶϿ���
        packet = CPacket((BYTE*)buf, len); // ��һ�����Ķγ��ȣ�
        index += len;
        len = index;
        
        if (len > 0) { // ���������Ч���ͽ������������Ƴ���Ȼ�������⻺������������ݣ�
            memmove(buf, buf + len, sizeof(buf) - len); // ������û���������Ƶ�ǰ�棻
            index -= len; // ���»������Ĵ�С��
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

// �������溯��������һ��չ��string���ݺ�İ���
bool Server_sock::send_message(CPacket& packet)
{
    return send_message(packet.get_packet_data(), packet.pack_size());
}

bool Server_sock::get_filepath(std::string& filepath) // filepath�Ǵ���������
{
    if (((this->packet).cmd >= 2 && (this->packet).cmd <= 4) || (this->packet).cmd == 9 || (this->packet).cmd == 10) {
        filepath = (this->packet).data; // ֱ��ͨ��string��ֵ���µ�string������Ҳ��newһ���µĿռ�����ݣ�������ֱ�ӽ�ָ��ָ��ԭʼ��string���洢�����ݣ�
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
    closesocket(sock_server); // ��������ʱ��رգ�
    WSACleanup();
}

CPacket::CPacket() :head{ 0 }, length{ 0 }, cmd{ 0 }, sum{0}
{
}
// ��װ��pdata��c����ַ���������string�ṹ�壬����size�����ݳ��ȶ����ǰ��ĳ��ȣ�
CPacket::CPacket(WORD cmd, BYTE* pdata, int size)
{
    length = size + 4; // ���ݳ���+4��
    this->cmd = cmd;
    if (size > 0) {
        data.resize(size); // ����Ҫresize������ֱ��memcpy���Խ�磻
        memcpy(data.data(), pdata, size); // ���ַ�����ֵ���з�����
    }
    else {
        data.clear();
    }
    sum = 0;
    for (int i = 0; i < size; i++) {
        sum += pdata[i] & 0xFF;
    }
}
// �����������������һ��CPacket���󣬶Է������Ƿ�װ���չ�����ݵ��ֽ�����
CPacket::CPacket(const BYTE* pdata, int& size)
{
    size_t i = 0;
    for (; i < size; i++)
    {
        if (*(WORD*)(pdata+i) == 0xFEFF) { // ��λ��WORD��С�����ݣ�ֱ���ҵ��̶�λ���������ڵ�˼�룻
            i += 2; // du�����̶�λ��������ĳ��Ⱦ��ǹ̶�Ϊ��С���Ӷ��ڶ�����ʱɶ������������+2���������������������ֱ�ӷ��أ�
            break;
        }
    }
    // �������������İ��ͷ��أ�
    if (i >= size) {
        size = 0;
        return;
    }
    // �����ȱ�ٳ���������κ����ݶ���ʾ���͵���Ϣ��ȫ�ͷ��أ�
    if (i + 4 + 2 + 2 > size) {
        size = 0;
        return;
    }
    // �����ȣ�
    length = *(DWORD*)(pdata+i);
    i += 4;
    if (length + i >size) { // ��δ��ȫ���ܵ���
        size = 0;
        return;
    }
    // �����
    cmd = *(WORD*)(pdata + i);
    i += 2;
    // �����ݣ���Ϊ������const char*���ͣ�������string������Ϊ�������洢���ݣ�
    if (length > 4) { // ��������ݲŶ���cmd��sum�������ֽڣ�
        data.resize(length-4, '0'); // ��resize�䳤ʱ����Ҫ����ֵ��������ܻ�Խ�磻
        memcpy(data.data(), pdata+i, length - 4);
    }
    // ����У�飺
    sum = *(WORD*)(pdata + size - 2);
    WORD recv_sum = 0;
    for (auto c : data) {
        recv_sum += (BYTE)c & 0xFF;
    }
    if (recv_sum == sum) {
        size = i + length - 2; // ���Ľ���λ���ڻ������Ķ����ֽڴ���
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
    packet_data.resize(length + 6); // ���CPacket��һ����ֵ���ã���ô����resize����ռ�������memcpy���������ɾ����ֵ�汾��send_message��
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
    return length+2+4; // ������չ����ȫ�����Ĵ�С��
}
