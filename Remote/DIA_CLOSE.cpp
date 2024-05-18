// DIA_CLOSE.cpp: 实现文件
//

#include "pch.h"
#include "RemoteCtrl.h"
#include "afxdialogex.h"
#include "DIA_CLOSE.h"


// DIA_CLOSE 对话框

IMPLEMENT_DYNAMIC(DIA_CLOSE, CDialogEx)

DIA_CLOSE::DIA_CLOSE(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG_CLOSE_COMPUTER, pParent)
{

}

DIA_CLOSE::~DIA_CLOSE()
{
}

void DIA_CLOSE::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(DIA_CLOSE, CDialogEx)
END_MESSAGE_MAP()


// DIA_CLOSE 消息处理程序


BOOL DIA_CLOSE::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  在此添加额外的初始化

	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}
