#include "pch.h"
#include "CTool.h"

bool CTool::MakeDriveInfo()
{
    std::string filepath;
    // ��ȡĿ¼·���ɹ���
    if (Server_sock::getinstance()->get_filepath(filepath)) {
        // ע�����ֱ��chdir("C:")�������л�C�̣�������Ҫ���������������ʲô��������Ĭ��ΪC:/�����Ե��л�C�̵�ʱ����Ҫͨ��_chdir("C://")�л�����
        if (filepath.size() == 2) {
            filepath += "//";
        }
        if (_chdir(filepath.data())) {
            /*Server_sock::getinstance()->send_message(CPacket(2, (BYTE*)"no priviledge/directory", 24));*/
            MessageBox(NULL, L"�޷���Ȩ��/�����ڸ�Ŀ¼", L"Ȩ��/·������", MB_OK | MB_ICONERROR);
            return false;
        }
        // ����������ļ���Ϣ��
        WIN32_FIND_DATAA file{}; // �ļ��ṹ�壻
        HANDLE file_h = FindFirstFileA("*", &file);
        if (file_h == INVALID_HANDLE_VALUE) { // ��Ŀ¼���Ҳ����κ��ļ���
            /*Server_sock::getinstance()->send_message(CPacket(2, (BYTE*)"no file in this directory", 26));*/
            MessageBox(NULL, L"Ŀ¼�����ļ�", L"��Ŀ¼��ʾ", MB_OK | MB_ICONERROR);
            return false;
        }
        do {
            if (strcmp(file.cFileName, ".") == 0 || strcmp(file.cFileName, "..") == 0) {
                continue;
            }
            Server_sock::getinstance()->send_message(CPacket(2, (BYTE*)&file, sizeof(file)));
        } while (FindNextFileA(file_h, &file));
        return true;// �����˴��̵���Ϣ��װ�ɰ����ͻ��˷���ȥ��
    }
    // ��ȡĿ¼·��ʧ�ܣ�
    MessageBox(NULL, L"������󣬲����뿴Ŀ¼�ļ���Ϣ", L"�������", MB_OK | MB_ICONERROR);
    return false;
}

bool CTool::MakeDirInfo()
{

    std::string filepath;
    // ��ȡĿ¼·���ɹ���
    if (Server_sock::getinstance()->get_filepath(filepath)) {
        // ע�����ֱ��chdir("C:")�������л�C�̣�������Ҫ���������������ʲô��������Ĭ��ΪC:/�����Ե��л�C�̵�ʱ����Ҫͨ��_chdir("C://")�л�����
        if (filepath.size() == 2) {
            filepath += "//";
        }
        if (_chdir(filepath.data())) {
            /*Server_sock::getinstance()->send_message(CPacket(2, (BYTE*)"no priviledge/directory", 24));*/
            MessageBox(NULL, L"�޷���Ȩ��/�����ڸ�Ŀ¼", L"Ȩ��/·������", MB_OK | MB_ICONERROR);
            return false;
        }
        // ����������ļ���Ϣ��
        WIN32_FIND_DATAA file{}; // �ļ��ṹ�壻
        HANDLE file_h = FindFirstFileA("*", &file);
        if (file_h == INVALID_HANDLE_VALUE) { // ��Ŀ¼���Ҳ����κ��ļ���
            /*Server_sock::getinstance()->send_message(CPacket(2, (BYTE*)"no file in this directory", 26));*/
            MessageBox(NULL, L"Ŀ¼�����ļ�", L"��Ŀ¼��ʾ", MB_OK | MB_ICONERROR);
            return false;
        }
        do {
            if (strcmp(file.cFileName, ".") == 0 || strcmp(file.cFileName, "..") == 0) {
                continue;
            }
            Server_sock::getinstance()->send_message(CPacket(2, (BYTE*)&file, sizeof(file)));
        } while (FindNextFileA(file_h, &file));
        return true;// �����˴��̵���Ϣ��װ�ɰ����ͻ��˷���ȥ��
    }
    // ��ȡĿ¼·��ʧ�ܣ�
    MessageBox(NULL, L"������󣬲����뿴Ŀ¼�ļ���Ϣ", L"�������", MB_OK | MB_ICONERROR);
    return false;
}

bool CTool::RunFile()
{
    std::string filepath;
    // ��ȡĿ¼·���ɹ���
    if (Server_sock::getinstance()->get_filepath(filepath)) {
        HINSTANCE h = ShellExecuteA(NULL, NULL, filepath.data(), NULL, NULL, SW_SHOWNORMAL); // ͨ����ǰ���̴�һ���ļ��������ڴ����ӽ��̣��������ַ�ʽ�����Ǹ��ӽ��̣���ֻ�Ǵ�һ�����̣�
        // ��ʧ�ܣ�
        if ((INT_PTR)h < 32) {
            Server_sock::getinstance()->send_message(CPacket(3, (BYTE*)"open file fail", 15));
            MessageBox(NULL, L"���ļ�ʧ��", L"��ʧ����ʾ", MB_OK | MB_ICONERROR);
            return false;
        }
        // �򿪳ɹ���
        Server_sock::getinstance()->send_message(CPacket(3, (BYTE*)"open file success", 18));
        return true;
    }
    // ��ȡ·��ʧ�ܣ�
    MessageBox(NULL, L"������󣬲��������ļ�", L"�������", MB_OK | MB_ICONERROR);
    return false;
}

bool CTool::Get_File_Length()
{
    std::string filepath;
    // ��ȡĿ¼·���ɹ���
    if (Server_sock::getinstance()->get_filepath(filepath)) {
        FILE* pfile = fopen(filepath.data(), "rb");
        if (pfile == nullptr) {
            Server_sock::getinstance()->send_message(CPacket(10, (BYTE*)"get file length fail", 21));
            MessageBox(NULL, L"�����ļ�ʧ��", L"����ʧ����ʾ", MB_OK | MB_ICONERROR);
            return false;
        }
        // ��ȡ�ļ��ܳ������أ��Ƚ��ļ�ָ��ͨ��fseek�ƶ����ļ�β��Ȼ��ͨ��ftell��ȡλ�ã�������ƶ����ļ�ͷ��
        fseek(pfile, 0, SEEK_END);
        long long totle_file_len = _ftelli64(pfile);
        Server_sock::getinstance()->send_message(CPacket(10, (BYTE*)&totle_file_len, sizeof(totle_file_len))); // �����ļ����ȱ��ڿͻ�����ʾ���ؽ�������
        fseek(pfile, 0, SEEK_SET);
        return true;
    }
}


