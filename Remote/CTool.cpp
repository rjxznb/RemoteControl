#include "pch.h"
#include "CTool.h"

bool CTool::MakeDriveInfo()
{
    std::string filepath;
    // 获取目录路径成功：
    if (Server_sock::getinstance()->get_filepath(filepath)) {
        // 注意这里：直接chdir("C:")并不会切回C盘，而是需要看环境变量设的是什么，我这里默认为C:/，所以当切回C盘的时候，需要通过_chdir("C://")切回来；
        if (filepath.size() == 2) {
            filepath += "//";
        }
        if (_chdir(filepath.data())) {
            /*Server_sock::getinstance()->send_message(CPacket(2, (BYTE*)"no priviledge/directory", 24));*/
            MessageBox(NULL, L"无访问权限/不存在该目录", L"权限/路径错误", MB_OK | MB_ICONERROR);
            return false;
        }
        // 访问里面的文件信息；
        WIN32_FIND_DATAA file{}; // 文件结构体；
        HANDLE file_h = FindFirstFileA("*", &file);
        if (file_h == INVALID_HANDLE_VALUE) { // 在目录里找不到任何文件；
            /*Server_sock::getinstance()->send_message(CPacket(2, (BYTE*)"no file in this directory", 26));*/
            MessageBox(NULL, L"目录内无文件", L"空目录提示", MB_OK | MB_ICONERROR);
            return false;
        }
        do {
            if (strcmp(file.cFileName, ".") == 0 || strcmp(file.cFileName, "..") == 0) {
                continue;
            }
            Server_sock::getinstance()->send_message(CPacket(2, (BYTE*)&file, sizeof(file)));
        } while (FindNextFileA(file_h, &file));
        return true;// 读完了磁盘的信息封装成包给客户端发回去；
    }
    // 获取目录路径失败：
    MessageBox(NULL, L"命令错误，不是想看目录文件信息", L"命令错误", MB_OK | MB_ICONERROR);
    return false;
}

bool CTool::MakeDirInfo()
{

    std::string filepath;
    // 获取目录路径成功：
    if (Server_sock::getinstance()->get_filepath(filepath)) {
        // 注意这里：直接chdir("C:")并不会切回C盘，而是需要看环境变量设的是什么，我这里默认为C:/，所以当切回C盘的时候，需要通过_chdir("C://")切回来；
        if (filepath.size() == 2) {
            filepath += "//";
        }
        if (_chdir(filepath.data())) {
            /*Server_sock::getinstance()->send_message(CPacket(2, (BYTE*)"no priviledge/directory", 24));*/
            MessageBox(NULL, L"无访问权限/不存在该目录", L"权限/路径错误", MB_OK | MB_ICONERROR);
            return false;
        }
        // 访问里面的文件信息；
        WIN32_FIND_DATAA file{}; // 文件结构体；
        HANDLE file_h = FindFirstFileA("*", &file);
        if (file_h == INVALID_HANDLE_VALUE) { // 在目录里找不到任何文件；
            /*Server_sock::getinstance()->send_message(CPacket(2, (BYTE*)"no file in this directory", 26));*/
            MessageBox(NULL, L"目录内无文件", L"空目录提示", MB_OK | MB_ICONERROR);
            return false;
        }
        do {
            if (strcmp(file.cFileName, ".") == 0 || strcmp(file.cFileName, "..") == 0) {
                continue;
            }
            Server_sock::getinstance()->send_message(CPacket(2, (BYTE*)&file, sizeof(file)));
        } while (FindNextFileA(file_h, &file));
        return true;// 读完了磁盘的信息封装成包给客户端发回去；
    }
    // 获取目录路径失败：
    MessageBox(NULL, L"命令错误，不是想看目录文件信息", L"命令错误", MB_OK | MB_ICONERROR);
    return false;
}

bool CTool::RunFile()
{
    std::string filepath;
    // 获取目录路径成功：
    if (Server_sock::getinstance()->get_filepath(filepath)) {
        HINSTANCE h = ShellExecuteA(NULL, NULL, filepath.data(), NULL, NULL, SW_SHOWNORMAL); // 通过当前进程打开一个文件；类似于创建子进程，但是这种方式并不是父子进程，而只是打开一个进程；
        // 打开失败；
        if ((INT_PTR)h < 32) {
            Server_sock::getinstance()->send_message(CPacket(3, (BYTE*)"open file fail", 15));
            MessageBox(NULL, L"打开文件失败", L"打开失败提示", MB_OK | MB_ICONERROR);
            return false;
        }
        // 打开成功；
        Server_sock::getinstance()->send_message(CPacket(3, (BYTE*)"open file success", 18));
        return true;
    }
    // 获取路径失败；
    MessageBox(NULL, L"命令错误，不是运行文件", L"命令错误", MB_OK | MB_ICONERROR);
    return false;
}

bool CTool::Get_File_Length()
{
    std::string filepath;
    // 获取目录路径成功：
    if (Server_sock::getinstance()->get_filepath(filepath)) {
        FILE* pfile = fopen(filepath.data(), "rb");
        if (pfile == nullptr) {
            Server_sock::getinstance()->send_message(CPacket(10, (BYTE*)"get file length fail", 21));
            MessageBox(NULL, L"下载文件失败", L"下载失败提示", MB_OK | MB_ICONERROR);
            return false;
        }
        // 获取文件总长并传回：先将文件指针通过fseek移动到文件尾，然后通过ftell获取位置，最后再移动回文件头；
        fseek(pfile, 0, SEEK_END);
        long long totle_file_len = _ftelli64(pfile);
        Server_sock::getinstance()->send_message(CPacket(10, (BYTE*)&totle_file_len, sizeof(totle_file_len))); // 发送文件长度便于客户端显示下载进度条；
        fseek(pfile, 0, SEEK_SET);
        return true;
    }
}


