// RemoteCtrl.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include "framework.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 唯一的应用程序对象

CWinApp theApp;

using namespace std;
// 查看主机有几个磁盘分区，发送成功返回true；
bool MakeDriveInfo() {
    string drive;
    for (int i = 1; i <= 26;i++) {
        if (_chdrive(i) == 0) {
            drive += 'A' + i - 1;
            drive += ','; //最后要有一个逗号，在客户端读的时候会以此来做判断，即使最后一个盘号后也要加；
        }
    }
    return Server_sock::getinstance()->send_message(CPacket(1, (BYTE*)drive.data(), drive.size())); // 读完了磁盘的信息封装成包给客户端发回去；
 }
// 查看目录文件信息；
bool MakeDirInfo() {
    std::string filepath;
    // 获取目录路径成功：
    if (Server_sock::getinstance()->get_filepath(filepath)) {
        // 注意这里：直接chdir("C:")并不会切回C盘，而是需要看环境变量设的是什么，我这里默认为C:/，所以当切回C盘的时候，需要通过_chdir("C://")切回来；
        if (filepath.size() == 2) {
            filepath += "//";
        }
        if(_chdir(filepath.data())) {
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
// 运行文件；
bool RunFile() {
    string filepath;
    // 获取目录路径成功：
    if (Server_sock::getinstance()->get_filepath(filepath)) {
        HINSTANCE h = ShellExecuteA(NULL, NULL, filepath.data(), NULL, NULL, SW_SHOWNORMAL); // 通过当前进程打开一个文件；类似于创建子进程，但是这种方式并不是父子进程，而只是打开一个进程；
        // 打开失败；
        if((INT_PTR)h < 32){
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
// 在下载文件之前，客户端需要先发送该命令获取文件总长度，然后在下载文件，方便显示进度条，因为我们设计的客户端收包是长连接，发多个包一个全得收完了再释放连接然后读；
bool Get_File_Length() {
    string filepath;
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

// 下载文件：说白了就是读文件然后传回去；
bool DownloadFile() {
    string filepath;
    // 获取目录路径成功：
    if (Server_sock::getinstance()->get_filepath(filepath)) {
        FILE* pfile = fopen(filepath.data(), "rb");
        if (pfile == nullptr) {
            Server_sock::getinstance()->send_message(CPacket(4, (BYTE*)"download fail", 14));
            MessageBox(NULL, L"下载文件失败", L"下载失败提示", MB_OK | MB_ICONERROR);
            return false;
        }

        char buf[1024]{}; // 缓冲区；
        int len = 0; // 存储每次读的长度；
        do {
            len = fread(buf, 1, sizeof(buf), pfile); // size=1表示读文件的最小单位，count表示最多都多少个size；
            Server_sock::getinstance()->send_message(CPacket(4, (BYTE*)buf, len));
        } while (len >= sizeof(buf)); // 当最后一次读的小于1024表示读到文件尾啦；
        fclose(pfile);
        return true;
    }
    // 获取路径失败；
    MessageBox(NULL, L"命令错误，不是下载文件", L"命令错误", MB_OK | MB_ICONERROR);
    return false;
}
// 执行鼠标事件；
bool DoMouseEvent() {
    Mouse_Event mouse;
    // 获取鼠标事件结构体；
    if (Server_sock::getinstance()->get_mouse_event(mouse)) {
        // 事件标志位：
        WORD flag; // 每一个二进制位表示一个事件类型，可以或运算来叠加action和button事件类型；
        // 判断button事件类型：
        switch (mouse.button) {
            case 0: // 左键
                flag = 0b1;
                break;
            case 1: // 右键
                flag = 0b10;
                break;
            case 2: // 中键
                flag = 0b100;
                break;
            case 3: // 没有按键
                flag = 0;
            default:
                break;
        }
        switch (mouse.action) {
        case 0: // 按下
            flag |= 0b1000;
            break;
        case 1: // 抬起
            flag |= 0b10000;
            break;
        case 2: // 没有按键
            flag = 0;
        default:
            break;
        }

        // 如果鼠标事件不是移动，那么就先移动鼠标到目的位置，如果是移动就通过鼠标事件API来执行，为了能够获取鼠标事件的信息；
        if (flag != 0) {
            SetCursorPos(mouse.point.x, mouse.point.y);
        }
        

        // 前两个事件的排列组合：真正执行鼠标事件的地方；
        switch (flag) {
        case 0b1001: // 左键按下；
            mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, NULL, GetMessageExtraInfo());
            break;
        case 0b10001: // 左键抬起；
            mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, NULL, GetMessageExtraInfo());
            break;
        case 0b1010: // 右键按下；
            mouse_event(MOUSEEVENTF_RIGHTDOWN, 0, 0, NULL, GetMessageExtraInfo());
            break;
        case 0b10010: // 右键抬起；
            mouse_event(MOUSEEVENTF_RIGHTUP, 0, 0, NULL, GetMessageExtraInfo());
            break;
        case 0b1100: // 中键按下；
            mouse_event(MOUSEEVENTF_MIDDLEDOWN, 0, 0, NULL, GetMessageExtraInfo());
            break;
        case 0b10100: //中键抬起；
            mouse_event(MOUSEEVENTF_MIDDLEUP, 0, 0, NULL, GetMessageExtraInfo());
            break;
        case 0: // 没有任何鼠标按键，只是鼠标的移动；
            mouse_event(MOUSEEVENTF_MOVE | MOUSEEVENTF_ABSOLUTE, mouse.point.x, mouse.point.y, NULL, GetMessageExtraInfo());
            break;
        default:
            break;
        }
        Server_sock::getinstance()->send_message(CPacket(5, NULL, 0));
        return true;
    }
    // 获取鼠标事件结构体失败；
    MessageBox(NULL, L"命令错误，不是鼠标移动", L"命令错误", MB_OK | MB_ICONERROR);
    return false;
}
// 显示屏幕内容：就是发送截图给客户端； 使用CImage类需要引入atlimage，是存储GDI绘图相关的头文件；
bool SendScreen() {
    CImage screen; // 用于存放绘制好的图片；
    HDC dc = ::GetDC(NULL); // 获取全局屏幕dc，一定要叫全局，否则会调用该窗口类内的，因为所有窗口对象都继承该函数啦呦；
    int nbitsPerpixel = GetDeviceCaps(dc, BITSPIXEL);
    int screen_pixel_width = GetDeviceCaps(dc, HORZRES);
    int screen_pixel_height = GetDeviceCaps(dc, VERTRES);
    screen.Create(screen_pixel_width, screen_pixel_height, nbitsPerpixel); // 创建CImage控件对象；生成的图片：宽高比为8：5，2560x1600；
    if (BitBlt(screen.GetDC(), 0, 0, screen_pixel_width, screen_pixel_height, dc, 0, 0, SRCCOPY) == false) { // 把服务器屏幕截图画入CImage对象；SRCCOPY宏：将源矩形直接复制到目标矩形；
        ReleaseDC(NULL, dc);
        screen.ReleaseDC();
        Server_sock::getinstance()->send_message(CPacket(6, (BYTE*)"CImage.Create wrong", 20));
        MessageBox(NULL, L"图像对象建立失败", L"建立失败提示", MB_OK | MB_ICONERROR);
        return false;
    }
    // 这里只能通过全局申请函数申请而不能通过堆申请函数申请，因为下面的流只能保存到全局堆而不能通过Heap申请；
    HGLOBAL global = GlobalAlloc(GMEM_MOVEABLE, 0); // 申请全局堆区空间；
    IStream* stream = NULL; // IStream指向要存入cimage对象的流起始地址；
    HRESULT hr = CreateStreamOnHGlobal(global, TRUE, &stream); // 将图像保存到流；
    if (hr == S_OK) {
        screen.Save(stream, Gdiplus::ImageFormatPNG); // 存入字节流地址；
        LARGE_INTEGER begin{ 0 };
        stream->Seek(begin, SEEK_SET, NULL); // 移动回起始位置，便于后面释放；
        PBYTE pdata = (PBYTE)GlobalLock(global); // 固定全局内存，得到起始地址来发数据；
        int size = GlobalSize(global); // 得到申请的内存大小；
        Server_sock::getinstance()->send_message(CPacket(6, (BYTE*)pdata, size));
        stream->Release(); // 递减 COM 对象上接口的引用计数。
        ReleaseDC(NULL, dc);
        screen.ReleaseDC();
        GlobalFree(global);
        return true;
    }
    MessageBox(NULL, L"建立流对象失败", L"内存中建立流对象错误", MB_OK | MB_ICONERROR);
    Server_sock::getinstance()->send_message(CPacket(6, (BYTE*)"create image stream on global wrong", 36));
    return false;
    // 比较一下jpg和png谁的生成速度更快，结果表明png大小更小，但是生成速度较慢；jpg更大，速度却更快；但是速度相差不大，而内存占比更为重要，所以选png；
    //DWORD tick = GetTickCount64(); // 返回从操作系统启动以来经过多少个小的时间周期；
    //screen.Save(_T("screen.png"), Gdiplus::ImageFormatPNG); // 保存为png文件到本地查看；
    //TRACE("png:%d", GetTickCount64()-tick);
    //tick = GetTickCount64();
    // screen.Save(_T("screen.jpg"), Gdiplus::ImageFormatJPEG); // 保存为jpg文件到本地查看；
    //TRACE("png:%d", GetTickCount64() - tick);
}

// 锁机线程回调函数；
DIA_CLOSE dia;
unsigned thread_close_computer(void* arg) {
    if (dia.Create(IDD_DIALOG_CLOSE_COMPUTER, NULL)) { // 建立窗口；
        dia.ShowWindow(SW_SHOW); // 显示对话框；
        dia.SetWindowPos(&(dia.wndTopMost), 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE); // 显示在最上面；
        RECT rect;
        CWnd* h = dia.GetDesktopWindow(); // 获取桌面窗口对象，这样在充满屏幕的时候就能知道显示器的分辨率啦呦，同样可通过GetSystemMetrics来得到显示器宽高呦；
        h->GetWindowRect(&rect);
        MoveWindow(dia.m_hWnd, 0, 0, rect.right, rect.bottom, true); // 扩大窗口充满屏幕；
        ShowCursor(false); // 隐藏鼠标指针；
        GetWindowRect(dia.m_hWnd, &rect); // 得到窗口矩形坐标；
        rect.right = rect.left + 1;
        rect.bottom = rect.top + 1;
        ClipCursor(&rect); // 显示鼠标移动范围在窗口矩形内；
        MSG msg; // msg就是每次所收到的消息，包括message消息类型，和该消息携带的wparam，lparam参数；
        while (GetMessageW(&msg, NULL, 0, 0)) {
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
            if (msg.message == WM_KEYDOWN) { // 如果是键盘按下消息，那么就判断；
                if (msg.wParam == VK_ESCAPE) { // 如果是esc键的话，就退出锁机；
                    break;
                }
            }
        }
        dia.DestroyWindow(); // 使锁机全局对话框局部失效，否则第二次锁机就无法满足锁机的条件；
        ShowCursor(true);
        _endthreadex(NULL);
        return 0;
    }
    MessageBox(NULL, L"建立对话框失败", L"窗口错误", MB_OK | MB_ICONERROR);
    Server_sock::getinstance()->send_message(CPacket(7, (BYTE*)"dialog create wrong", 20));
    return 0;
}

// 弹出对话框锁机；
unsigned int tid; // 存储线程id号，用于后面向该线程发退出消息循环的消息；
bool Close_Computer() {
    // 如果没锁呢，就执行；
    if (dia.m_hWnd == NULL || dia.m_hWnd == INVALID_HANDLE_VALUE) {
        SOCKET client_socket = Server_sock::getinstance()->get_client_sock();
        _beginthreadex(NULL, 0, thread_close_computer, &client_socket, 0, &tid);
    }
    Server_sock::getinstance()->send_message(CPacket(7, (BYTE*)"computer has already been locked", 33));
    return true;
}

// 消除对话框解锁；
bool Open_Computer() {
    PostThreadMessageW(tid, WM_QUIT, NULL, NULL);
    Server_sock::getinstance()->send_message(CPacket(8, (BYTE*)"computer opened", 16));
    return true;
}
// TODO：令起线程删除文件；
bool Delete_File() {
    string filepath;
    // 获取目录路径成功：
    if (Server_sock::getinstance()->get_filepath(filepath)) {\
        // 删除成功；
        if (DeleteFileA(filepath.data())) {
            return true;
        }
        else {
            MessageBox(NULL, L"删除文件失败，文件不存在或无删除权限", L"删除操作错误", MB_OK | MB_ICONERROR);
            return false;
        }
    }
    // 获取路径失败；
    MessageBox(NULL, L"命令错误，不是删除文件", L"命令错误", MB_OK | MB_ICONERROR);
    return false;
}

// 命令处理函数：
void Process_Cmd(int cmd){
    switch (cmd) {
        case -1: // 若什么都没读到表示客户端已经断开连接，则break继续执行accept函数等待连接；
            MessageBox(NULL, _T("客户端已断开连接"), _T("接收命令失败"), MB_OK | MB_ICONERROR);
            break;
        case 1:
            MakeDriveInfo();
            break;
        case 2:
            MakeDirInfo();
            break;
        case 3:
            RunFile();
            break;
        case 4:
            DownloadFile();
            break;
        case 5:
            DoMouseEvent();
            break;
        case 6:
            SendScreen();
            break;
        case 7:
            Close_Computer();
            break;
        case 8:
            Open_Computer();
            break;
        case 9:
            Delete_File();
            break;
        case 10:
            Get_File_Length();
            break;
        default:
            break;
    }
}
// 投屏读发包线程；
void screen_thread(void* arg) {
    // 通过参数传递过来主线程的客户端套接字，如果这里不重新赋值就会变为初始化值NULL；
    Server_sock::getinstance()->get_client_sock() = *(SOCKET*)arg;
    do{
        Process_Cmd(6);
    } while (Server_sock::getinstance()->recv_message() != -1);
    Server_sock::getinstance()->close_client_socket();
    _endthread();
}

int main()
{
    int nRetCode = 0;

    HMODULE hModule = ::GetModuleHandle(nullptr);

    if (hModule != nullptr)
    {
        // 初始化 MFC 并在失败时显示错误
        if (!AfxWinInit(hModule, nullptr, ::GetCommandLine(), 0))
        {
            // TODO: 在此处为应用程序的行为编写代码。
            wprintf(L"错误: MFC 初始化失败\n");
            nRetCode = 1;
        }


        else
        {
            // TODO: 在此处为应用程序的行为编写代码。
            // 初始化一个类对象智能指针，实现对socket库的初始化；
            std::unique_ptr<Server_sock>& server_sock = Server_sock::getinstance();
            // 绑定套接字，然后监听客户端；
            if (server_sock->init_socket() == false) {
                MessageBox(NULL, _T("网阔套接字初始化错误，请检查网络"), _T("网络初始化失败"), MB_OK | MB_ICONERROR);
                exit(-1);
            }
            // 如果未连接成功，就最多重试三次；
            int count = 0; // 用于计数三次；
            while (Server_sock::getinstance()) {
                if (server_sock->accept_socket() == false) { // 之所以要每次都accept一次，就是因为客户端每次发完一个包之后就释放连接，然后发包时，会再重新连接；
                    if (count >= 3) {
                        MessageBox(NULL, _T("接入用户失败，请手动关闭重试"), _T("接入用户失败"), MB_OK | MB_ICONERROR);
                        exit(-1);
                    }
                    MessageBox(NULL, _T("接入用户失败，自动重试"), _T("接入用户失败"), MB_OK | MB_ICONERROR);
                    count++;
                }
                int cmd = server_sock->recv_message();
                // 如果是收截屏就单开一个线程去循环收消息，处理命令；
                if (cmd == 6) {
                    _beginthread(screen_thread, 0, &(server_sock->get_client_sock()));
                    continue;
                }
                if (cmd != -1) { // 如果命令有效；
                    Process_Cmd(cmd);
                }
                else {
                    TRACE("命令错误，收到的命令为");
                }
                server_sock->close_client_socket();
            }
        }
    }
    else
    {
        // TODO: 更改错误代码以符合需要
        wprintf(L"错误: GetModuleHandle 失败\n");
        nRetCode = 1;
    }

    return nRetCode;
}
