
// CheckNetCardDlg.h : Í·ÎÄ¼þ
//

#pragma once
#include <map>
#include "WinNetCard.h"
#include "afxwin.h"
#include "RepeatTask.h"


class CCheckNetCardDlg : public CDialogEx
{

public:
	CCheckNetCardDlg(CWnd* pParent = NULL);
	std::map<int,TNetCardStruct> m_map;
	RepeatTask m_task;

	enum { IDD = IDD_CHECKNETCARD_DIALOG };

	BOOL ShowBalloonTip(LPCTSTR szMsg, LPCTSTR szTitle, UINT uTimeout, DWORD dwInfoFlags);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);

protected:
	HICON m_hIcon;
	NOTIFYICONDATA nid;

	void DialogToTray();
	virtual BOOL OnInitDialog();

	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	CComboBox m_comboList;
	afx_msg void OnBnClickedButtonTracer();
	afx_msg void OnBnClickedButtonOpen();
	afx_msg void OnBnClickedButtonClose();
	afx_msg LRESULT OnShowTask(WPARAM wParam,LPARAM lParam) ;
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
};
