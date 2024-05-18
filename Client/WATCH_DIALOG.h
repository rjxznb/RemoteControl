#pragma once
#include "afxdialogex.h"
#include"ClientDlg.h"

// WATCH_DIALOG 对话框

class WATCH_DIALOG : public CDialogEx
{
	DECLARE_DYNAMIC(WATCH_DIALOG)

public:
	WATCH_DIALOG(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~WATCH_DIALOG();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_WATCH_DIALOG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	virtual BOOL OnInitDialog();
	CStatic picture;
	afx_msg void OnClose();
	std::string trans_pos_and_get_cursor_evnet_data(CPoint, int, int);
//	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
//	afx_msg void OnMButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnMButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMButtonUp(UINT nFlags, CPoint point);
//	afx_msg void OnRButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	virtual void OnOK();
//	afx_msg void OnStnClickedPictureWatch();
	afx_msg void OnBnClickedCloseComputerButton();
	afx_msg void OnBnClickedOpenComputerButton();
};
