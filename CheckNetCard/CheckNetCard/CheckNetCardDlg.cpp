
// CheckNetCardDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "CheckNetCard.h"
#include "CheckNetCardDlg.h"
#include "afxdialogex.h"
#include <algorithm>
#include <Wininet.h>
#include <iphlpapi.h>
#include "ods.h"
#pragma comment(lib,"Iphlpapi.lib")
#pragma comment(lib,"Wininet.lib")
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define WM_SHOWTASK WM_USER + 32

CCheckNetCardDlg::CCheckNetCardDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CCheckNetCardDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CCheckNetCardDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO_CARDLIST, m_comboList);
}

BEGIN_MESSAGE_MAP(CCheckNetCardDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_TRACER, &CCheckNetCardDlg::OnBnClickedButtonTracer)
	ON_BN_CLICKED(IDC_BUTTON_OPEN, &CCheckNetCardDlg::OnBnClickedButtonOpen)
	ON_BN_CLICKED(IDC_BUTTON_CLOSE, &CCheckNetCardDlg::OnBnClickedButtonClose)
	ON_MESSAGE(WM_SHOWTASK,OnShowTask)
	ON_WM_SYSCOMMAND()
END_MESSAGE_MAP()


CStringArray g_VpnSpecString;
static DWORD g_yetStatus = 0;
DWORD WINAPI CheckVPNStatus(LPVOID lpParameter)
{
	CCheckNetCardDlg* dlg = (CCheckNetCardDlg*)lpParameter;
	DWORD dwConnectStatus = 0;

	GetAdaptersInfo(NULL,&dwConnectStatus);

//	DebugODS(_T("GetAdaptersInfo : %d"),dwConnectStatus);

	if (dwConnectStatus != g_yetStatus)	
	{
		WinNetCard::NetCardStateChange(&dlg->m_map[dlg->m_comboList.GetCurSel()],FALSE);
		dlg->PostMessage(WM_COMMAND,IDC_BUTTON_TRACER);
		dlg->ShowBalloonTip(_T("网卡状态改变，VPN已断开！"),_T("TracerVPN"),3000,1);
	}

	return 0;
}

LRESULT CCheckNetCardDlg::OnShowTask(WPARAM wParam,LPARAM lParam)
{
	switch(lParam)
	{
	case WM_RBUTTONUP:
 		break;
	case WM_LBUTTONDBLCLK://双击左键的处理
		ShowWindow(SW_SHOW);//简单的显示主窗口完事儿
		break; 
	default: 
		break;
	}
	return 0;
} 

void CCheckNetCardDlg::DialogToTray()
{
	nid.cbSize = (DWORD)sizeof(NOTIFYICONDATA);
	nid.hWnd = m_hWnd;
	nid.uID = IDR_MAINFRAME;
	nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP ;
	nid.uCallbackMessage = WM_SHOWTASK;
	nid.hIcon = LoadIcon(AfxGetInstanceHandle(),MAKEINTRESOURCE(IDR_MAINFRAME));
	wcscpy(nid.szTip,_T("程序名称"));
	Shell_NotifyIcon(NIM_ADD,&nid);
	ShowWindow(SW_HIDE);
}

BOOL CCheckNetCardDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	SetIcon(m_hIcon, TRUE);		
	SetIcon(m_hIcon, FALSE);	

	NetCardList* list = new NetCardList();

	WinNetCard::EnumNetCards(list);

	SetDlgItemText(IDC_EDIT_TIMEOUT,_T("500"));

	g_VpnSpecString.Add(_T("(L2TP)"));
	g_VpnSpecString.Add(_T("(SSTP)"));
	g_VpnSpecString.Add(_T("(PPTP)"));
	g_VpnSpecString.Add(_T("(PPPOE)"));	

	NetCardList::iterator it = list->begin();
	for (; it != list->end();it++)
	{
		int i = 0;
		for ( ;i < g_VpnSpecString.GetCount(); i++)
		{
			if (it->Name.Find(g_VpnSpecString[i]) > -1)
				break;
		}
		if (i == g_VpnSpecString.GetCount())
		{
			int index = m_comboList.AddString(it->Name);
			m_map[index] = *it;
		}
	}
	
	delete list;

	m_task.Init(CheckVPNStatus,this,_T("test"),0,800);

	m_comboList.SetCurSel(0);

	return TRUE; 
}

void CCheckNetCardDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this);

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

HCURSOR CCheckNetCardDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CCheckNetCardDlg::OnBnClickedButtonTracer()
{
	CString strCur = _T("");

	GetDlgItemText(IDC_BUTTON_TRACER,strCur);

	if (strCur == _T("监控"))
	{
		GetAdaptersInfo(NULL,&g_yetStatus);
		int nOutTime = GetDlgItemInt(IDC_EDIT_TIMEOUT);
		m_task.SetIntervalSeconds(nOutTime);
		m_task.Start();

		GetDlgItem(IDC_COMBO_CARDLIST)->EnableWindow(FALSE);
		GetDlgItem(IDC_EDIT_TIMEOUT)->EnableWindow(FALSE);

		SetDlgItemText(IDC_BUTTON_TRACER,_T("停止"));
	}
	else
	{
		SetDlgItemText(IDC_BUTTON_TRACER,_T("监控"));

		m_task.Stop();

		GetDlgItem(IDC_COMBO_CARDLIST)->EnableWindow(TRUE);
		GetDlgItem(IDC_EDIT_TIMEOUT)->EnableWindow(TRUE);
	}

}


void CCheckNetCardDlg::OnBnClickedButtonOpen()
{
	WinNetCard::NetCardStateChange(
		&m_map[m_comboList.GetCurSel()],true);
}

void CCheckNetCardDlg::OnBnClickedButtonClose()
{
	WinNetCard::NetCardStateChange(
		&m_map[m_comboList.GetCurSel()],false);
}


BOOL CCheckNetCardDlg::ShowBalloonTip(LPCTSTR szMsg, LPCTSTR szTitle, UINT uTimeout, DWORD dwInfoFlags)
{
	nid.cbSize=sizeof(NOTIFYICONDATA);
	nid.uFlags = NIF_INFO;
	nid.uTimeout = uTimeout;
	nid.dwInfoFlags = dwInfoFlags;
	lstrcpy(nid.szInfo,szMsg ? szMsg : _T(""));
	lstrcpy(nid.szInfoTitle,szTitle ? szTitle : _T(""));
	return Shell_NotifyIcon(NIM_MODIFY, &nid);
}

void CCheckNetCardDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	CDialogEx::OnSysCommand(nID, lParam);
	if(nID==SC_MINIMIZE)    
		DialogToTray();
}
