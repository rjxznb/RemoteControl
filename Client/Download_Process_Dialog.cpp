// Download_Process_Dialog.cpp: 实现文件
//

#include "pch.h"
#include "Client.h"
#include "afxdialogex.h"
#include "Download_Process_Dialog.h"


// Download_Process_Dialog 对话框

IMPLEMENT_DYNAMIC(Download_Process_Dialog, CDialogEx)

Download_Process_Dialog::Download_Process_Dialog(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG1, pParent)
	, progress_edit(_T(""))
{

}

Download_Process_Dialog::~Download_Process_Dialog()
{
}

void Download_Process_Dialog::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_PROCESS_EDIT, progress_edit);
	DDX_Control(pDX, IDC_DOWNLOAD_PROGRESS, progress);
}


BEGIN_MESSAGE_MAP(Download_Process_Dialog, CDialogEx)
	ON_WM_TIMER()
END_MESSAGE_MAP()


// Download_Process_Dialog 消息处理程序


void Download_Process_Dialog::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	UpdateData(false);
	CDialogEx::OnTimer(nIDEvent);
}
