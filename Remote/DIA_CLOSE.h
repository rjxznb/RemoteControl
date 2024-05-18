#pragma once
#include "afxdialogex.h"


// DIA_CLOSE 对话框

class DIA_CLOSE : public CDialogEx
{
	DECLARE_DYNAMIC(DIA_CLOSE)

public:
	DIA_CLOSE(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~DIA_CLOSE();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_CLOSE_COMPUTER };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
};
