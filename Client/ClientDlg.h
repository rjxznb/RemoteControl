
// ClientDlg.h: 头文件
//

#pragma once
#include"Client_sock.h"
#include"Download_Process_Dialog.h"
#include"WATCH_DIALOG.h"

// CClientDlg 对话框
class CClientDlg : public CDialogEx
{
// 构造
public:
	CClientDlg(CWnd* pParent = nullptr);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_CLIENT_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	CImage screen_image; // 缓存截屏图片；
	bool success_image = false; // 记录每次都读成功了嘛，还是读错丢包啦，方便定时器检查，成功就直接拿走，不成功就啥都不干；
	int screen_image_width;
	int screen_image_height;
public:
	std::wstring ConvertToWideString(const std::string& narrowStr);
	std::string ConvertToNarrowString(const CString& wideStr);
	afx_msg void OnBnClickedButton1();
	DWORD ip;
	CString port;
	afx_msg void OnBnClickedButton2();
	std::list<std::string> send_cmd(WORD cmd, std::string data = "", int data_length = 0); // 向服务器发命令包；
	void send_cmd(FILE* download_file, long long& processed_file_len, WORD cmd, std::string data, int data_length);
	CTreeCtrl file_tree;
	CString get_tree_item_path(HTREEITEM item);
	void get_file_info();
	afx_msg void OnNMDblclkTree1(NMHDR* pNMHDR, LRESULT* pResult);
	CListCtrl file_list;
	afx_msg void OnNMClickTree1(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnNMRClickList3(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnFileoperationOpenfile();
	afx_msg void OnFileoperationDeletefile();
	afx_msg void OnFileoperationDownloadfile();
	afx_msg LRESULT OnDownLoadCMD(WPARAM wparam, LPARAM lparam);
	afx_msg LRESULT OnGetFileLength(WPARAM wparam, LPARAM lparam);
	static void download_file_thread(void* arg);
	std::string get_selected_file_path_in_file_list();
	static unsigned entry_get_screen_data_thread(void* arg); // 这是线程函数，因为无this指针所以在里面通过传递的this指针参数，来调用下面的函数，从而能变相地使用this指针；
	void get_screen_data(); // 循环读服务器发来滴数据呦；
	afx_msg void OnBnClickedButton3();
	afx_msg void OnClose();
};


