// WATCH_DIALOG.cpp: 实现文件
//

#include "pch.h"
#include "Client.h"
#include "afxdialogex.h"
#include "WATCH_DIALOG.h"


// WATCH_DIALOG 对话框

IMPLEMENT_DYNAMIC(WATCH_DIALOG, CDialogEx)

WATCH_DIALOG::WATCH_DIALOG(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_WATCH_DIALOG, pParent)
{

}

WATCH_DIALOG::~WATCH_DIALOG()
{
}

void WATCH_DIALOG::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PICTURE_WATCH, picture);
}


BEGIN_MESSAGE_MAP(WATCH_DIALOG, CDialogEx)
	ON_WM_TIMER()
	ON_WM_CLOSE()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MBUTTONDOWN()
	ON_WM_MBUTTONUP()
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
	ON_WM_MOUSEMOVE()
//	ON_STN_CLICKED(IDC_PICTURE_WATCH, &WATCH_DIALOG::OnStnClickedPictureWatch)
ON_BN_CLICKED(IDC_CLOSE_COMPUTER_BUTTON, &WATCH_DIALOG::OnBnClickedCloseComputerButton)
ON_BN_CLICKED(IDC_OPEN_COMPUTER_BUTTON, &WATCH_DIALOG::OnBnClickedOpenComputerButton)
END_MESSAGE_MAP()


// WATCH_DIALOG 消息处理程序
// TODO:显示时无法填充满，并且无法显示全部屏幕；
void WATCH_DIALOG::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if (nIDEvent == 0) {
		// 获取主对话框对象，来读图片成员变量；
		CClientDlg* client_dialog = (CClientDlg*)GetParent();
		if (client_dialog->success_image == true) {
			// 将图片通过picture control控件的dc按像素位绘制到控件中来；
			CRect rect;
			picture.GetWindowRect(&rect); // 我们在资源文件中添加的picture control一定要和目标屏幕等比例；
			client_dialog->screen_image.StretchBlt(picture.GetDC()->GetSafeHdc(), 0, 0, rect.Width(), rect.Height(), SRCCOPY);
			// 绘制完要记得重绘static，否则永远不显示，并且不是对话框而是picture control控件；
			picture.InvalidateRect(NULL);
			client_dialog->screen_image.Destroy(); // 销毁CImage加载的位图对象；
			client_dialog->success_image = false; // 设为无效；
		}
	}
	CDialogEx::OnTimer(nIDEvent);
}


BOOL WATCH_DIALOG::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  在此添加额外的初始化
	SetTimer(0, 50, NULL); // 第一个参数表示发送 0 作为响应函数的形参，也就是用来识别是哪个定时器发的消息；
	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}

// 重写右上角的关闭响应函数，删除最后一句的传递消息，避免将主对话框也关啦呦；
void WATCH_DIALOG::OnClose()
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	// 结束模态对话框只能通过enddialog函数，而不能通过destroy/closewindow；
	KillTimer(0);
	EndDialog(0);
}

// 封装起坐标转换，并返回存储着鼠标结构体数据的缓冲区，默认为移动鼠标事件，传入的第一个参数是鼠标在对话框窗口客户区的局部坐标；
std::string WATCH_DIALOG::trans_pos_and_get_cursor_evnet_data(CPoint client_cursor, int action=4, int button=3) {
	CClientDlg* client_dialog = (CClientDlg*)GetParent();

	CRect rect;
	picture.GetWindowRect(&rect);
	client_cursor.x = client_cursor.x * client_dialog->screen_image_width / rect.Width();
	client_cursor.y = client_cursor.y * client_dialog->screen_image_height / rect.Height();

	// 将转换完的坐标作为数据发回去，因为这是消息，所以能够中断主线程来响应他，此时我们就相当于又和对方建立一个连接，即使sock是单例模式，但是客户端是可以有多个连接的，因为他是通过connect来连接服务器，不需要提供自身的ip和端口，所以他会自动再分配一个端口给他们通信，当结束本响应函数再继续刷新投屏；
	// 这里采用短连接；
	Mouse_Event mouse_event;
	mouse_event.action = action;
	mouse_event.button = button;
	mouse_event.point = client_cursor;

	// 通过单例对象在建立一条短连接的链路来发包，因为他是在主线程，所以可以执行主对话框的updatedata函数；
	std::string send_data;
	send_data.resize(sizeof(mouse_event));
	memcpy(send_data.data(), (BYTE*)&mouse_event, sizeof(mouse_event));
	return send_data;
}

// 在主线程新建立一条短连接链路，左键按下命令；参数有鼠标点击位置的客户区局部坐标；
void WATCH_DIALOG::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	// 获取主对话框对象，来读图片成员变量的宽高，从而确定服务器屏幕的大小来等比例缩放；
	CClientDlg* client_dialog = (CClientDlg*)GetParent();
	std::string send_data = trans_pos_and_get_cursor_evnet_data(point, 0, 0);
	client_dialog->send_cmd(5, send_data, send_data.size());
	CDialogEx::OnLButtonDown(nFlags, point);
}

void WATCH_DIALOG::OnLButtonUp(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	// 获取主对话框对象，来读图片成员变量的宽高，从而确定服务器屏幕的大小来等比例缩放；
	CClientDlg* client_dialog = (CClientDlg*)GetParent();
	std::string send_data = trans_pos_and_get_cursor_evnet_data(point, 1, 0);
	client_dialog->send_cmd(5, send_data, send_data.size());
	CDialogEx::OnLButtonUp(nFlags, point);
}

void WATCH_DIALOG::OnMButtonDown(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	// 获取主对话框对象，来读图片成员变量的宽高，从而确定服务器屏幕的大小来等比例缩放；
	CClientDlg* client_dialog = (CClientDlg*)GetParent();
	std::string send_data = trans_pos_and_get_cursor_evnet_data(point, 0, 2);
	client_dialog->send_cmd(5, send_data, send_data.size());
	CDialogEx::OnMButtonDown(nFlags, point);
}

void WATCH_DIALOG::OnMButtonUp(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	// 获取主对话框对象，来读图片成员变量的宽高，从而确定服务器屏幕的大小来等比例缩放；
	CClientDlg* client_dialog = (CClientDlg*)GetParent();
	std::string send_data = trans_pos_and_get_cursor_evnet_data(point, 1, 2);
	client_dialog->send_cmd(5, send_data, send_data.size());
	CDialogEx::OnMButtonUp(nFlags, point);
}

void WATCH_DIALOG::OnRButtonDown(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	// 获取主对话框对象，来读图片成员变量的宽高，从而确定服务器屏幕的大小来等比例缩放；
	CClientDlg* client_dialog = (CClientDlg*)GetParent();
	std::string send_data = trans_pos_and_get_cursor_evnet_data(point, 0, 1);
	client_dialog->send_cmd(5, send_data, send_data.size());
	CDialogEx::OnRButtonDown(nFlags, point);
}

void WATCH_DIALOG::OnRButtonUp(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	// 获取主对话框对象，来读图片成员变量的宽高，从而确定服务器屏幕的大小来等比例缩放；
	CClientDlg* client_dialog = (CClientDlg*)GetParent();
	std::string send_data = trans_pos_and_get_cursor_evnet_data(point, 1, 1);
	client_dialog->send_cmd(5, send_data, send_data.size());
	CDialogEx::OnRButtonUp(nFlags, point);
}

void WATCH_DIALOG::OnMouseMove(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	// 获取主对话框对象，来读图片成员变量的宽高，从而确定服务器屏幕的大小来等比例缩放；
	/*CClientDlg* client_dialog = (CClientDlg*)GetParent();
	std::string send_data = trans_pos_and_get_cursor_evnet_data(point, 2, 3);
	client_dialog->send_cmd(5, send_data, send_data.size());*/
	CDialogEx::OnMouseMove(nFlags, point);
}

// 避免按回车推出；
void WATCH_DIALOG::OnOK()
{
	// TODO: 在此添加专用代码和/或调用基类

	// CDialogEx::OnOK();
}


void WATCH_DIALOG::OnBnClickedCloseComputerButton()
{
	// TODO: 在此添加控件通知处理程序代码
	CClientDlg* client_dialog = (CClientDlg*)GetParent();
	client_dialog->send_cmd(7);
}


void WATCH_DIALOG::OnBnClickedOpenComputerButton()
{
	// TODO: 在此添加控件通知处理程序代码
	CClientDlg* client_dialog = (CClientDlg*)GetParent();
	client_dialog->send_cmd(8);
}
