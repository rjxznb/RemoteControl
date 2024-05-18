#include "pch.h"
#include "Client_sock.h"

Client_sock::Client_sock()
{
    sock_server = INVALID_SOCKET;
    wVersionRequested = MAKEWORD(2, 2);
    if (WSAStartup(wVersionRequested, &wsaData) != 0) {
        MessageBoxW(NULL, L"�����׽��ֿ����", L"����", MB_OK | MB_ICONWARNING);
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
    memset(&addr1, 0, sizeof(addr1)); // Ҫ���㣻
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

// ��ȡ�������ݲ������ݣ�����packet���д洢ȫ����չ�����string���Ա����packet_data��Ϊ��������֮�������������ݲ�������data��Ա������������"-1"��
std::string Client_sock::recv_message()
{
    packet.packet_data.clear(); // �ȶ�֮ǰ�ù��İ����ݻ�������������
    packet.packet_data.resize(6); // �Ȼ�ȡ��ͷ���鿴���ݳ��ȣ��Դ������뻺������С��
    char* buf = packet.packet_data.data(); // ��ȡ����new�Ĵ����ݻ��������׵�ַ��
    int len = recv(sock_server, buf, 6, 0); // ��������̫������̶���������С��һ���ղ������������ȶ�ȡ��С�����뻺������
    TRACE("client recv packet head length %d", len);
    // �����������������������Ͽ��������Ǹ�����������ݣ�����Է�������recv��������Ϊ�Է�������һ���յĹر�֪ͨ��Ȼ��len=0��
    if (len == 0) {
        return "";
    }
    // recv�쳣��
    if (len < 0) { 
        return "-1"; }
    // ���У��λ�ԣ��Ϳ����ȣ������ȷ��仺������
    DWORD packet_length = 0; // ���ڻ�ȡ�ð������ݳ��ȣ�
    if (*(WORD*)(buf) == 0xFEFF) { // ���ǰ�����ֽ���У��λ���Ͷ������ĸ��ֽڱ�ʾ�����ݳ��ȣ�������Ǿͱ�ʾ��ȡ����ʽ���ԣ�����Ĭ�ϳ������뻺������֮���ٽ����ʱ��ᱨ��
        packet_length = *(DWORD*)(buf + 2); // ���ճ��ȴ�С��չ��������
    }
    packet.packet_data.resize(6 + packet_length);
    buf = packet.packet_data.data(); // resize֮�󣬻������׵�ַ���ܻ�ı䣻
    // ��len����Ϊʵ���ۻ����˶೤��
    len = 0;
    // ���ʵ��recv�ĳ��Ȳ�������ǰ����İ�ͷ�洢�����ݳ��ȣ���ô��ѭ������֪������������������Ϊֹ��
    while (len < packet_length) {
        int once_recv_len = recv(sock_server, buf + 6 + len, packet_length - len, 0);
        len += once_recv_len;
    }
    len += 6;
    packet = CPacket((BYTE*)buf, len); // �����
    TRACE("client recv data length %d", len);
    if (len > 0) { // ������Ч;
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

// �������溯��������һ��չ��string���ݺ�İ���
bool Client_sock::send_message(CPacket& packet)
{
    // �������ڷ�Χ�ڣ�
    if (packet.cmd <= 0 || packet.cmd >= 11) {
        MessageBox(NULL, _T("����Ų�����Ч��Χ��"), L"ִ������ʧ��", MB_OK | MB_ICONERROR);
        TRACE("Wrong Cmd:%d, cmd number should among 1 - 11\n", packet.cmd);
        return false;
    }
    return send_message(packet.get_packet_data(), packet.pack_size());
}

bool Client_sock::get_filepath(std::string& filepath) // filepath�Ǵ���������
{
    if (((this->packet).cmd >= 2 && (this->packet).cmd <= 4) || (this->packet).cmd == 9 || (this->packet).cmd == 10) {
        filepath = (this->packet).data; // ֱ��ͨ��string��ֵ���µ�string������Ҳ��newһ���µĿռ�����ݣ�������ֱ�ӽ�ָ��ָ��ԭʼ��string���洢�����ݣ�
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
    closesocket(sock_server); // ��������ʱ��رգ�
    WSACleanup();
}

CPacket::CPacket() :head{ 0 }, length{ 0 }, cmd{ 0 }, sum{ 0 }
{
}
// ��װ��pdata��c����ַ���������string�ṹ�壻
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
        if (*(WORD*)(pdata + i) == 0xFEFF) { // ��λ��WORD��С�����ݣ�ֱ���ҵ��̶�λ���������ڵ�˼�룻
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
    length = *(DWORD*)(pdata + i);
    i += 4;
    if (length + i > size) { // ��δ��ȫ���ܵ���
        size = 0;
        return;
    }
    // �����
    cmd = *(WORD*)(pdata + i);
    i += 2;
    // �����ݣ���Ϊ������const char*���ͣ�������string������Ϊ�������洢���ݣ�
    if (length > 4) { // ��������ݲŶ���cmd��sum�������ֽڣ�
        data.resize(length - 4); // ͨ��memcpy��ֵǰ����Ҫ��resizeΪ�㹻��Ļ�������
        memcpy(data.data(), pdata + i, length - 4);
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
    return length + 2 + 4; // ������չ����ȫ�����Ĵ�С��
}
