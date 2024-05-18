#pragma once
#include "afxdialogex.h"


// Download_Process_Dialog 对话框

class Download_Process_Dialog : public CDialogEx
{
	DECLARE_DYNAMIC(Download_Process_Dialog)

public:
	Download_Process_Dialog(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~Download_Process_Dialog();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG1 };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	// 显示数字进度 a / b
	CString progress_edit;
	CProgressCtrl progress;
	afx_msg void OnTimer(UINT_PTR nIDEvent);
};
