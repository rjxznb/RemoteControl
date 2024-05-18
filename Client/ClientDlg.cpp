
// ClientDlg.cpp: 实现文件
//

#include "pch.h"
#include "framework.h"
#include "Client.h"
#include "ClientDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
public:
//	afx_msg void OnNMDblclkTree1(NMHDR* pNMHDR, LRESULT* pResult);
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CClientDlg 对话框



CClientDlg::CClientDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_CLIENT_DIALOG, pParent)
	, ip(0)
	, port(_T(""))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CClientDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_IPAddress(pDX, IDC_IPADDRESS1, ip);
	DDX_Text(pDX, IDC_EDIT1, port);
	DDX_Control(pDX, IDC_TREE1, file_tree);
	DDX_Control(pDX, IDC_LIST3, file_list);
}

BEGIN_MESSAGE_MAP(CClientDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON1, &CClientDlg::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON2, &CClientDlg::OnBnClickedButton2)
	ON_NOTIFY(NM_DBLCLK, IDC_TREE1, &CClientDlg::OnNMDblclkTree1)
	ON_NOTIFY(NM_CLICK, IDC_TREE1, &CClientDlg::OnNMClickTree1)
	ON_NOTIFY(NM_RCLICK, IDC_LIST3, &CClientDlg::OnNMRClickList3)
	ON_COMMAND(ID_FILEOPERATION_OPENFILE, &CClientDlg::OnFileoperationOpenfile)
	ON_COMMAND(ID_FILEOPERATION_DELETEFILE, &CClientDlg::OnFileoperationDeletefile)
	ON_COMMAND(ID_FILEOPERATION_DOWNLOADFILE, &CClientDlg::OnFileoperationDownloadfile)
	ON_MESSAGE(WM_DOWNLOAD_CMD, &CClientDlg::OnDownLoadCMD)
	ON_MESSAGE(WM_GET_FILE_LENGTH, &CClientDlg::OnGetFileLength)
	ON_BN_CLICKED(IDC_WATCH_BUTTON, &CClientDlg::OnBnClickedButton3)
	ON_WM_CLOSE()
END_MESSAGE_MAP()


// CClientDlg 消息处理程序

BOOL CClientDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CClientDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CClientDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CClientDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



// 对发命令包和读返回数据的封装：因为后面要有各种不同命令的包要重复使用；返回一个list<string>，存储一个命令所对应收到的全部包的数据部分的内容；短连接客户端只发一次命令，但是服务器可发多个包，如：读文件，每个文件返回一个包；
std::list<std::string> CClientDlg::send_cmd(WORD cmd, std::string data, int data_length)
{
	UpdateData(); // 更新控件对应的值 ip 和 port；

	// 初始化一个类对象智能指针，实现对socket库的初始化；
	std::unique_ptr<Client_sock>& client_sock = Client_sock::getinstance(); // 是引用，所以他的局部生命周期结束也不会调用析构函数；
	// 绑定套接字，然后建立连接；
	if (client_sock->init_socket(ip, _wtoi(port)) == false) {
		::MessageBoxW(NULL, _T("网络套接字初始化错误，请检查网络"), L"网络初始化失败", MB_OK | MB_ICONERROR);
		client_sock->close_socket();
		client_sock = nullptr; // 在每次发送完包之后都释放套接字并析构指针，然后重新初始化；直接把nullptr赋值给智能指针就会调用其指向类对象的析构函数，效果同unique_ptr.reset();
		return {};
	}
	CPacket pack1(cmd, (BYTE*)data.data(), data_length); // 装包，发命令；
	if (client_sock->send_message(pack1) == false) { // 发展开数据后的包；
		::MessageBoxW(NULL, _T("发送失败，请检查网络"), L"数据发送失败", MB_OK | MB_ICONERROR);
		client_sock->close_socket();
		client_sock = nullptr; // 在每次发送完包之后都释放套接字并析构指针，然后重新初始化；直接把nullptr赋值给智能指针就会调用其指向类对象的析构函数，效果同unique_ptr.reset();
		return {};
	}

	// 循环读消息，把每一个包的数据都存在一个list里累积，如果当前读到数据为空，则表示已经读完啦；
	std::string recv_data{};
	std::list<std::string> packet_data_recv_list{};

	// 对于不返回数据的命令，就不向list装入任何存储数据的string对象；
	while ((recv_data = Client_sock::getinstance()->recv_message()) != "") {
		// 如果本次读到的数据返回-1，则表示本次读取操作异常，返回已读到的内容并停止继续读；
		if (recv_data == "-1") {
			TRACE("wrong number:", WSAGetLastError());
			::MessageBoxW(NULL, _T("读取返回数据失败"), L"读取数据失败", MB_OK | MB_ICONERROR);
			break;
		}
		// 读到数据啦，插入list中来；
		packet_data_recv_list.push_back(recv_data);
	}
	
	// 读完数据，关闭套接字；
	client_sock->close_socket(); 
	// client_sock = nullptr; // 改为thread_local之后要删掉，否则会将单例类全部清零； // 在每次发送完包之后都释放套接字并析构指针，然后重新初始化；直接把nullptr赋值给智能指针就会调用其指向类对象的析构函数，效果同unique_ptr.reset();
	return packet_data_recv_list;
}

// 重载一遍send_cmd函数，对下载文件命令进行单独处理，需要传入文件指针和文件长度；
void CClientDlg::send_cmd(FILE* download_file, long long& processed_file_len, WORD cmd, std::string data, int data_length)
{
	if (cmd != 4) {
		MessageBoxW(L"将非下载命令传入为下载命令重载的send_cmd函数中来", L"命令错误", MB_OK | MB_ICONERROR);
		return;
	}

	UpdateData(); // 更新控件对应的值 ip 和 port；

	// 初始化一个类对象智能指针，实现对socket库的初始化；
	std::unique_ptr<Client_sock>& client_sock = Client_sock::getinstance(); // 是引用，所以他的局部生命周期结束也不会调用析构函数；
	// 绑定套接字，然后监听客户端；
	if (client_sock->init_socket(ip, _wtoi(port)) == false) {
		::MessageBoxW(NULL, _T("网络套接字初始化错误，请检查网络"), L"网络初始化失败", MB_OK | MB_ICONERROR);
		client_sock->close_socket();
		client_sock = nullptr; // 在每次发送完包之后都释放套接字并析构指针，然后重新初始化；直接把nullptr赋值给智能指针就会调用其指向类对象的析构函数，效果同unique_ptr.reset();
		return ;
	}
	CPacket pack1(cmd, (BYTE*)data.data(), data_length); // 装包，发命令；
	if (client_sock->send_message(pack1) == false) { // 发展开数据后的包；
		::MessageBoxW(NULL, _T("发送失败，请检查网络"), L"数据发送失败", MB_OK | MB_ICONERROR);
		client_sock->close_socket();
		client_sock = nullptr; // 在每次发送完包之后都释放套接字并析构指针，然后重新初始化；直接把nullptr赋值给智能指针就会调用其指向类对象的析构函数，效果同unique_ptr.reset();
		return ;
	}
	// 不同通过模态化显示，否则会阻塞在domodal直至关闭对话框，从而无法更新控件显示最新进度；
	Download_Process_Dialog download_process_dialog;
	download_process_dialog.BeginWaitCursor(); // 将光标置为沙漏型；
	download_process_dialog.Create(IDD_DOWNLOAD_PROGRESS_DIALOG, this);
	download_process_dialog.ShowWindow(SW_SHOW);

	// 设定进度条范围：
	// TODO:下载不显示edit；是不是没更新回来download_process_dialog.UpdateData();
	download_process_dialog.progress.SetRange32(0, (processed_file_len / 1024) + 1); // 按照包个数设定上限，注意：如果按照字节有可能会过大，32位表示不过来；
	download_process_dialog.progress_edit = CString("0 /") + std::to_wstring((processed_file_len / 1024) + 1).data();
	download_process_dialog.UpdateData(false); // 向窗口更新数据，注意：是进度条窗口，而不是主对话框；
	// 自己推动消息，否则只会在最后一个循环中更新对话框内容，这时updatedata(false)固有的问题；
	MSG   msg;
	while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	// 循环读消息，读完一个丢一个；
	std::string recv_data{};
	// 记录下载/删除文件的进度；
	int downnload_progress = 0;
	// 对于不返回数据的命令，就不向list装入任何存储数据的string对象；
	while ((recv_data = Client_sock::getinstance()->recv_message()) != "") {
		// 如果本次读到的数据返回-1，则表示本次读取操作异常，返回已读到的内容并停止继续读；
		if (recv_data == "-1") {
			TRACE("wrong number:", WSAGetLastError());
			::MessageBoxW(NULL, _T("读取返回数据失败"), L"读取数据失败", MB_OK | MB_ICONERROR);
			break;
		}

		// 如果是下载文件就写入外存，并且从list中删除读到的包，避免占用内存过大；
		CProgressCtrl& prog = download_process_dialog.progress; // 通过引用来改个名简化；
		CString& prog_edit = download_process_dialog.progress_edit;
		int low;
		int higher; // 存储进度条上限；
		prog.GetRange(low, higher);
		prog.SetPos(++downnload_progress); // 如果通过正数设定，一定要注意类型转换，有可能小于1的数直接变为0，造成一直都处于0的状态；
		prog_edit = CString(std::to_wstring(prog.GetPos()).data())+ L'/' + std::to_wstring(higher).data();
		TRACE("progress: %d // %d", prog.GetPos(), higher);
		download_process_dialog.UpdateData(false);
		while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		Sleep(1000);
		int len = fwrite(recv_data.data(), 1, recv_data.size(), download_file);
	}
	// 下载/删除完成文件，就析构进度条对象；
	MessageBox(L"Download Complete", NULL, MB_OK | MB_APPLMODAL);
	// 读完数据，关闭套接字；
	client_sock->close_socket();
	client_sock = nullptr; // 在每次发送完包之后都释放套接字并析构指针，然后重新初始化；直接把nullptr赋值给智能指针就会调用其指向类对象的析构函数，效果同unique_ptr.reset();
	return ;
}

// 窄字节string对象变宽字节wstring对象；
std::wstring CClientDlg::ConvertToWideString(const std::string& narrowStr)
{
	// TODO:窄转宽还是不对，中文更不对；

	// 计算宽字符字符串所需的缓冲区大小
	int wideStrSize = MultiByteToWideChar(CP_UTF8, 0, narrowStr.data(), -1, NULL, 0);

	// 分配缓冲区
	wchar_t* pwideStr = new wchar_t[wideStrSize];

	// 将窄字符字符串转换为宽字符字符串
	MultiByteToWideChar(CP_UTF8, 0, narrowStr.data(), -1, pwideStr, wideStrSize);

	std::wstring wideStr(pwideStr);
	// 释放缓冲区内存
	delete[] pwideStr;

	return wideStr;
}

std::string CClientDlg::ConvertToNarrowString(const CString& wideStr)
{
	int wideStrLen = wideStr.GetLength(); // 获取宽字节字符串的长度

	// 计算转换后的窄字节字符串长度
	int narrowStrLen = WideCharToMultiByte(CP_ACP, 0, wideStr, wideStrLen, NULL, 0, NULL, NULL);

	// 分配内存
	char* narrowStr = new char[narrowStrLen + 1];

	// 进行转换
	WideCharToMultiByte(CP_ACP, 0, wideStr, wideStrLen, narrowStr, narrowStrLen, NULL, NULL);

	// 添加字符串结束符
	narrowStr[narrowStrLen] = '\0';

	// 将窄字节字符串存储到std::string对象中
	std::string narrowString(narrowStr);
	// 输出转换后的窄字节字符串
	std::cout << "Narrow string: " << narrowStr << std::endl;

	// 释放内存
	delete[] narrowStr;
	return narrowString;
}

// 点击按钮，测试能否建立连接，命令虽然是无效的，但是能够判断是否连接成功；
void CClientDlg::OnBnClickedButton1()
{
	// TODO: 在此添加控件通知处理程序代码
	// send_cmd返回一个list，所以这里取返回的第一个包；
	send_cmd(1222);
}

// 点击按钮，获取磁盘信息；
void CClientDlg::OnBnClickedButton2()
{
	// TODO: 在此添加控件通知处理程序代码
	// send_cmd返回一个list，并且读磁盘只返回一个包，所以取第一个包的数据；
	std::string recv_data = send_cmd(1).front();
	if (recv_data == "-1") return;
	file_tree.DeleteAllItems(); // 删除之前所有的项；
	std::string driver{};
	for (auto i : recv_data) {
		if (i != ',') {
			driver += i;
		}
		else { // 变为宽字符串插入项；
			driver += ':';
			file_tree.InsertItem(ConvertToWideString(driver).data(), TVI_ROOT, TVI_LAST);
			driver.clear();
		}
	}
}

CString CClientDlg::get_tree_item_path(HTREEITEM item) {
	CString parent_basename, temp_path;
	temp_path = file_tree.GetItemText(item);
	// 获取树控件一个子项的父项句柄；
	HTREEITEM hParentItem = TreeView_GetParent(file_tree, item);
	while(hParentItem != NULL) {
		parent_basename = file_tree.GetItemText(hParentItem);
		temp_path = parent_basename + L"\\" + temp_path;
		hParentItem = TreeView_GetParent(file_tree, hParentItem); // 继续向上递归；
	}
	return temp_path;
}

// 删除父项的所有子项；假设 treeCtrl 是你的树控件对象，hParent 是要删除子项的父项句柄；
void DeleteAllChildren(CTreeCtrl& treeCtrl, HTREEITEM hParent)
{
	// 获取第一个子项句柄
	HTREEITEM hChildItem = treeCtrl.GetChildItem(hParent);

	// 遍历所有子项
	while (hChildItem != NULL)
	{
		// 获取下一个子项句柄
		HTREEITEM hNextItem = treeCtrl.GetNextItem(hChildItem, TVGN_NEXT);

		// 递归删除子项的所有子项
		DeleteAllChildren(treeCtrl, hChildItem);

		// 删除当前子项
		treeCtrl.DeleteItem(hChildItem);

		// 移动到下一个子项
		hChildItem = hNextItem;
	}
}

bool Is_Directory(const WIN32_FIND_DATAA& findData) {
	return (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
}

// 定义一个当前所访问目录的路径，方便后面使用；
std::string current_file_path{};
// 获取鼠标点击文件的信息结构体；
void CClientDlg::get_file_info() {
	// 获取选中的项句柄；
	CPoint mouse_pos;
	GetCursorPos(&mouse_pos);
	file_tree.ScreenToClient(&mouse_pos); // 全局到树控件局部坐标的转换；
	HTREEITEM item = file_tree.HitTest(mouse_pos); // 返回所在位置的项的句柄；
	// 啥都没点到就不发命令给服务器啦，直接退出；
	if (item == NULL) {
		return;
	}

	// 获取所点击目录的路径；转换为窄字节，因为装包时填数据采用窄字节的string类型对象；
	current_file_path = ConvertToNarrowString(get_tree_item_path(item));
	// 发包，获取目录路径下的文件信息，并以string对象作为缓冲区返回；
	std::list<std::string> recv_data_list = send_cmd(2, current_file_path, current_file_path.size());
	// 再插入项之前，先删除对应项的子项，避免重复显示；
	DeleteAllChildren(file_tree, item);
	file_list.DeleteAllItems();
	// 读出每一个包的数据，如果是目录插入tree control，如果是文件插入list control；
	for (auto recv_data : recv_data_list) {
		WIN32_FIND_DATAA file_info{}; // 因为发的时候就是结构体对象，所以收的时候同样通过文件信息结构体来挨个接收文件信息；
		file_info = *((WIN32_FIND_DATAA*)recv_data.data());
		if (Is_Directory(file_info)) {
			file_tree.InsertItem(ConvertToWideString(file_info.cFileName).data(), item, TVI_LAST);
		}
		else {
			file_list.InsertItem(0, ConvertToWideString(file_info.cFileName).data());
		}
	}
}

// 树控件被双击事件，有可能点到空白处，所以要加判断；
void CClientDlg::OnNMDblclkTree1(NMHDR* pNMHDR, LRESULT* pResult)
{
	// TODO: 在此添加控件通知处理程序代码
	get_file_info();
	*pResult = 0;
}


void CClientDlg::OnNMClickTree1(NMHDR* pNMHDR, LRESULT* pResult)
{
	// TODO: 在此添加控件通知处理程序代码
	get_file_info();
	*pResult = 0;
}


void CClientDlg::OnNMRClickList3(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	// 获取选中的项号；
	CPoint mouse_pos, client_pos;
	GetCursorPos(&mouse_pos);
	client_pos = mouse_pos;
	file_list.ScreenToClient(&client_pos); // 全局到列表控件局部坐标的转换；
	int item = file_list.HitTest(client_pos); // 返回点击的是列表中的第几个项；
	// 啥都没点到就不发命令给服务器啦，直接退出；
	if (item < 0) {
		return;
	}
	// 加载file operation menu；
	CMenu file_menu;
	file_menu.LoadMenuW(IDR_FILE_MENU);
	CMenu* pop_up_menu = file_menu.GetSubMenu(0); // 获得菜单第一栏的句柄；
	pop_up_menu->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, mouse_pos.x, mouse_pos.y, this); // 弹出菜单并跟踪；
	*pResult = 0;
}

// 获得选中文件的绝对路径；
std::string CClientDlg::get_selected_file_path_in_file_list() {
	int selected_item = file_list.GetSelectionMark(); // 获得第几个项被选择，如果没有选择标记，则为 -1；
	CString wide_file_name = file_list.GetItemText(selected_item, 0);
	std::string file_name = ConvertToNarrowString(wide_file_name);
	// 获取选中文件的绝对路径，current_file_path是一个全局变量，因为只有先获取tree control中的目录路径之后才能访问该路径下的文件，所以他就是当前文件所在的目录路径；这里改file_name，不要改current路径变量，那么变量在其他的文件命令函数中还要用滴呦；
	file_name = current_file_path + "\\" + file_name;
	return file_name;
}

// 打开/运行选中的文件；
void CClientDlg::OnFileoperationOpenfile()
{
	// TODO: 在此添加命令处理程序代码
	std::string file_name = get_selected_file_path_in_file_list();
	send_cmd(3, file_name.data(), file_name.size());
}

// 删除选中的文件：需要等待服务器删除完毕再进行其他操作，否则就算客户端可以进行，但是因为服务器是单线程的，所以他还在删除着呢；并且再删除玩之后还要在列表控件中删除掉这个项；我们这样就能够实现，因为在send_cmd里会有recv阻塞着，如果服务器没执行完命令是不会释放连接滴呦，所以直到服务器释放连接，客户端才会继续执行recv后面的语句呦；
void CClientDlg::OnFileoperationDeletefile()
{
	// TODO: 在此添加命令处理程序代码
	// 获取右键列表项选中文件路径
	int selected_item = file_list.GetSelectionMark(); // 获得第几个项被选择，如果没有选择标记，则为 -1；
	CString wide_file_name = file_list.GetItemText(selected_item, 0);
	std::string file_name = ConvertToNarrowString(wide_file_name);
	// 获取选中文件的绝对路径，current_file_path是一个全局变量，因为只有先获取tree control中的目录路径之后才能访问该路径下的文件，所以他就是当前文件所在的目录路径；这里改file_name，不要改current路径变量，那么变量在其他的文件命令函数中还要用滴呦；
	file_name = current_file_path + "\\" + file_name;
	// 发送删除命令和文件路径，如果没删完就一直卡在recv里面；
	send_cmd(9, file_name.data(), file_name.size());
	// 在列表控件中也删除该项；
	file_list.DeleteItem(selected_item);

	// TODO:删除文件显示进度，我们只通过DeleteFile系统自带的API来删除，无法显示进度；
	//// 先获取文件长度，为了在删除时显示进度条；
	//std::string recv_file_len_data = send_cmd(10, file_name.data(), file_name.size()).front();
	//processed_file_len = *(long long*)(recv_file_len_data.data());
}

// 主线程要执行的响应函数：wparam和lparam要携带processed_file_len, file_name_len 和file_name三个数据，但是空间并不足，所以我们传一个指向堆区数据流缓冲区的指针，因为都是在一个进程中，所以即使来自不同线程也可以访问到同一个进程的堆区变量呦；不能跨线程传递文件指针，因为通过open打开文件他返回的是一个指向本线程的结构体变量，而不是堆区空间，所以他们可能不能共享；
// 我们将主要的逻辑还是放到这里，因为文件指针不能跨线程共享，所以我们干脆还是在主线程的响应函数中再打开文件吧，所以这就相当多线程了个寂寞；流程：主线程开启子线程下载文件->子线程发消息给主线程，让其发命令给服务器->主线程执行相应函数，并且sendmessage发消息会阻塞到目标线程执行完响应函数之后返回函数的返回值；
// wparam传指向存储file_name_len 和file_name 的堆区地址指针，lparam传文件总长度；
LRESULT CClientDlg::OnDownLoadCMD(WPARAM wparam, LPARAM lparam) {
	// 逐位读数据：wparam内容中存储着一个指针，我们将其转换为想要的位指针类型就阔以滴啦；
	//BYTE* p = (BYTE*)wparam;
	//FILE* download_file = (FILE*)p; // 有点绕，前八个字节存放着FILE*变量指向的文件地址，所以该语句就相当于用指针赋值指针，先将p转换为FILE*，该指针中存储着文件的地址，然后用这个指针赋值给新的局部指针，让其也指向他；
	size_t file_name_len = *(size_t*)wparam;
	std::string file_name{};
	file_name.resize(file_name_len);
	memcpy(file_name.data(), (BYTE*)wparam+sizeof(file_name_len), file_name_len);
	CFileDialog file_dialog(FALSE, L"*", ConvertToWideString(file_name).data(), OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, NULL, this);
	// 如果创建文件对话框正确，并且能够打开文件有权限写入才发送命令，否则就白忙活；
	if (file_dialog.DoModal() == IDOK) {
		// 打开文件：这里是调用文件对话框类中的获取选中文件路径函数，写入时会从文件头覆盖全部内容；
		FILE* download_file = fopen(ConvertToNarrowString(file_dialog.GetPathName()).data(), "wb+"); // 打开要存入数据的文件；
		if (download_file == NULL) {
			MessageBoxW(L"打开文件错误，无权限写入", NULL, MB_OK | MB_ICONERROR);
			delete [](BYTE*)wparam;
			return 0;
		}

		long long processed_file_len = (long long)lparam;
		// 下载文件时，服务器分多次发来一个文件的数据，每次发1024个字节，每读完一个包就将其写入外存，然后将其丢掉；
		send_cmd(download_file, processed_file_len, 4, file_name.data(), file_name.size());
		fclose(download_file);
		delete [](BYTE*)wparam; // 释放掉堆区的内存；
		return 1;
	}
	MessageBoxW(L"存入文件路径错误", NULL, MB_OK | MB_ICONERROR);
	delete[](BYTE*)wparam;
	return 0;
}

// 对于获取长度命令也需要通过自定义消息和响应函数来实现；wparam传递堆区字符串指针，lparam传递缓冲区的总长度；
LRESULT CClientDlg::OnGetFileLength(WPARAM wparam, LPARAM lparam) {

	long long processed_file_len = *(long long*)(send_cmd(10, std::string((const char*)wparam), (int)lparam).front().data());
	delete [] (BYTE*)wparam;
	return processed_file_len;
}


// 单独开启线程下载文件，因为他是通过向主线程发消息实现的下载，所以真正连接还是建立在主线程，所以无需在子线程释放任何套接字；
void CClientDlg::download_file_thread(void* arg) {
	CClientDlg* dlg = (CClientDlg*)arg;
	// 通过弹出文件对话框窗口，让用户选文件存入；
	std::string file_name = dlg->get_selected_file_path_in_file_list(); // 获取右键选中要下载的文件路径；
	// 先发送一个命令读取文件长度用来显示进度条；
	BYTE* length_wparam = new BYTE[file_name.size()];
	memcpy(length_wparam, file_name.data(), file_name.size());
	// 响应函数返回本次下载文件的字节数；
	long long processed_file_len = dlg->SendMessageW(WM_GET_FILE_LENGTH, (WPARAM)length_wparam, file_name.size());

	// 通过new申请文件名字符串指针和文件名长度变量两个参数字节长度的堆区缓冲区进行作为消息参数wparam传递，因为wparam为8字节，指针也为8字节，所以刚好可以传递指针类型，从而得到堆区缓冲区的起始地址；
	BYTE* download_wparam = new BYTE[sizeof(size_t) + file_name.size()];
	size_t file_name_len = file_name.size();
	memcpy(download_wparam, &file_name_len, sizeof(file_name_len)); // 为什么可以传递指针，因为传递的是堆区空间，虽然指针是局部变量，但是他所指向的内存是不会被释放的，我们传递的是指向的地址FILE*，而不是指针本身&FILE*（这种才是传递指针本身）；
	memcpy(download_wparam + sizeof(file_name_len), file_name.data(), file_name.size()); // 传文件名字符串的长度，不然在响应函数中不知道初始化多长；
	// 发消息回主线程，让主线程调用响应函数去执行；
	dlg->SendMessage(WM_DOWNLOAD_CMD, (WPARAM)download_wparam, (LPARAM)processed_file_len);
	_endthread();
	return;
}

// 下载选中的文件；前面先定义一个全局变量存储下载文件的总长度，用于在后面send_cmd函数中读数据显示进度条；
void CClientDlg::OnFileoperationDownloadfile()
{
	// TODO: 在此添加命令处理程序代码
	// 在线程的传入参数中传入对话框对象的this指针,只能线程回调函数定义为静态变量或者全局变量，不能定义为类的普通成员函数，因为他不能有this指针，采用的__cdecl函数约定；
	_beginthread(CClientDlg::download_file_thread, 0, this);
	Sleep(50); // 避免用户在主线程中点击了不同的文件，这样在下载线程中获取到的文件就可能会改变；
}
// 主要作用是执行下面的函数，为了能用this指针，从而访问非静态地成员变量和函数；
unsigned CClientDlg::entry_get_screen_data_thread(void* arg)
{
	Sleep(50); // 因为线程先于投屏对话框运行，所以要等他窗口加载出来再读图片；
	CClientDlg* dlg = (CClientDlg*)arg;
	dlg->get_screen_data();
	return 0;
}

// 循环读数据并缓存：在这里我们并不采用之前的send_cmd，因为他是发一次就释放而无法一直长连接发和读，并且里面有updatedata，还需要sendmessage回主线在中断响应返回，这样就失去了多线程的意义；所以这里通过定义client_sock类智能指针的方式从头建立连接，然后调用其定义的send_message函数来发包；
// 原则：发数据和读数据不成功不断连，扔掉继续发和读就好，这个命令主打一个持久而不中断；
SOCKET screen_thread_socket; // 用来访问子线程里的套接字，不然在外面关不了套接字；
void CClientDlg::get_screen_data() {
	// 初始化socket库；
	std::unique_ptr<Client_sock>& client_sock = Client_sock::getinstance();
	// 在获取ip和port时，可以不在这里updatedata，在主线程的显示函数中先updatedata，然后这里因为已经更新完啦，所以直接用他们俩就行啦；
	// 先要确保建立起连接，因为这里可能是广域网，所以建立连接时间比较长，一定要确保建立起再继续；
	while(client_sock->init_socket(ip, _wtoi(port)) == false) {
		::MessageBoxW(NULL, _T("网络套接字初始化错误，请检查网络，点击确认重试"), L"网络初始化失败", MB_OK | MB_ICONERROR);
		client_sock->close_socket();
		client_sock = nullptr; // 在每次发送完包之后都释放套接字并析构指针，然后重新初始化；直接把nullptr赋值给智能指针就会调用其指向类对象的析构函数，效果同unique_ptr.reset();
	}
	screen_thread_socket = client_sock->get_socket();
	// 一直发命令包，然后读数据包；
	CPacket pack1(6, NULL, 0); // 装包，发命令，因为是一种命令，所以在循环外定义一次就够啦；
	std::string recv_data{};
	
	
	// 采用长连接，一直不释放连接地send和recv；
	while (true) {
		// 获取当前时间：避免一次while循环过快导致白获取截图让cpu占用率过高，因为ontimer是50ms读一次，所以要让一轮的时间间隔50ms以上；
		ULONGLONG tick = GetTickCount64();

		if (client_sock->send_message(pack1) == false) { // 发展开数据后的包，如果不成功就停会儿再发而不释放；
			Sleep(1);
			TRACE("client send message fail");
		}
		else { // 发成功就等着读返回来滴数据包,recv_data存的是CImage对象存入的IStream数据流，所以我们先要采用同样的方式在全局堆开辟数据流空间，然后存入读来的数据，然后再将这个数据流Load加载入CImage对象；
			// 如果读完图片或者图片解包失败才会继续发命令包，不然没办法destroy这个图片的bitmap；
			if (success_image == false) {
				if ((recv_data = client_sock->recv_message()) != "-1") { // 如果收到的数据正确那就显示，否则错误就不显示，扔包就好啦，然后继续发包读包；
					HGLOBAL hg = GlobalAlloc(GMEM_MOVEABLE, 0);
					if (hg == NULL) {
						TRACE(L"内存不足");
						Sleep(1); // 防止cpu卡死，连程序关都关不了啦呦；
						continue;
					}
					IStream* pistream = nullptr;
					// 开辟内存成功则继续；CreateStreamOnHGlobal 函数创建一个流对象，也就是指向开辟存储流数据的内存首地址，该对象使用 HGLOBAL 内存句柄来存储流内容。
					if (CreateStreamOnHGlobal(hg, true, &pistream) == S_OK) {
						ULONG size; // 存储返回的实际写入长度；
						pistream->Write(recv_data.data(), recv_data.size(), &size);
						LARGE_INTEGER begin{ 0 };
						pistream->Seek(begin, SEEK_SET, NULL);
						if (screen_image.Load(pistream) < 0) { // 缓存数据到成员变量；
							TRACE("load fail number: %d", WSAGetLastError());
							if (screen_image.IsNull() == false) {
								screen_image.Destroy();
							}
							success_image = false;
							pistream->Release();
							GlobalFree(hg);
							Sleep(1); // 防止cpu卡死，连程序关都关不了啦呦；
							continue;
						}
						screen_image_width = screen_image.GetWidth();
						screen_image_height = screen_image.GetHeight();
						success_image = true; // 读成功，定时器可以取走；
						TRACE("success_image = true");
					}
					else {
						success_image = false; // 开辟失败，定时器啥都不干；
						if (screen_image.IsNull() == false) {
							screen_image.Destroy();
						}
						TRACE("success_image = false");
					}
					pistream->Release();
					GlobalFree(hg);
				}
			}
		}
		if (GetTickCount64() - tick < 40) {
			Sleep(40 + tick - GetTickCount64());
		}
	}
}
// 主线程：按下按钮实现投屏；
void CClientDlg::OnBnClickedButton3()
{
	// TODO: 在此添加控件通知处理程序代码
	// 在此先更新控件值，为了让后面在子线程中能使用更新后的ip和port，因为其他线程不能调用该对话框的upadtedata函数；
	UpdateData();
	unsigned int tid;
	WATCH_DIALOG watch_dialog;
	_beginthreadex(NULL, 0, entry_get_screen_data_thread, this, 0, &tid);
	
	// 为了防止按多次按钮，在此将其禁用，在后面退出函数里再解除；
	watch_dialog.DoModal(); // 阻塞在这里，直到点叉关闭对话框；
	closesocket(screen_thread_socket); // 关闭子线程投屏套接字连接；因为他在主线程，所以无法直接访问thread_local声明的套接字滴呦；
	HANDLE th = OpenThread(THREAD_TERMINATE, false, tid); // 获取线程句柄；
	if(TerminateThread(th, 0) == false){ // 退出线程，关闭套接字，从而让服务器也停止发送继续循环accept；
		TRACE("关闭线程失败");
	}
}

// 在主对话框点叉就强行退出本进程；
void CClientDlg::OnClose()
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	exit(0);
	CDialogEx::OnClose();
}
