// MainDlg.cpp : implementation of the CMainDlg class
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MainDlg.h"
#include "controls/ScintillaWnd.h"
#include "scintilla.h"
	
#ifdef DWMBLUR	//win7毛玻璃开关
#include <dwmapi.h>
#pragma comment(lib,"dwmapi.lib")
#endif

CMainDlg::CMainDlg() : SHostWnd(_T("LAYOUT:XML_MAINWND"))
{
	m_bLayoutInited = FALSE;
}

CMainDlg::~CMainDlg()
{
}

int CMainDlg::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	#ifdef DWMBLUR	//win7毛玻璃开关
	MARGINS mar = {5,5,30,5};
	DwmExtendFrameIntoClientArea ( m_hWnd, &mar );
	#endif

	SetMsgHandled(FALSE);
	return 0;
}

BOOL CMainDlg::OnInitDialog(HWND hWnd, LPARAM lParam)
{
	m_bLayoutInited = TRUE;
	return 0;
}

// 按钮：解析 xml 代码
HRESULT CMainDlg::OnButtonParseXmlCode()
{
	// 获取：用户输入的 xml 代码
	std::string strXmlCode;
	_GetScintillaText(&strXmlCode);
	if (strXmlCode.length() == 0) {
		SMessageBox(GetActiveWindow(), _T("请先输入 xml 代码"), _T("提示"), MB_OK);
		return S_OK;
	}
	// 清理：\r\n 字符去掉
	_ClearUnusefulChar(strXmlCode);
	// 解析：xml 代码
	std::string strStatusMessage;
	CWangYingXmlParser wangYingXmlParser;
	if (wangYingXmlParser.ParseXml(strXmlCode)) {
		strStatusMessage = "Parse: Success.";
	} else {
		wangYingXmlParser.GetErrorMessage(&strStatusMessage);
	}
	// 输出：解析状态
	SOUI::SEdit *pEditStatus = FindChildByName2<SOUI::SEdit>(L"edit_parse_status");
	assert(pEditStatus != NULL);
	// --转换：char 转 wchar_t
	wchar_t  ws[512];
	swprintf(ws, 512, L"%hs", strStatusMessage.c_str());
	pEditStatus->SetWindowText(ws);
	// 清理：xml 树
	SOUI::STreeCtrl *pTreeCtrl = FindChildByName2<SOUI::STreeCtrl>(L"tree_ctrl_xml");
	assert(pTreeCtrl != NULL);
	pTreeCtrl->RemoveAllItems();
	// 输出：xml 树
	if (strStatusMessage == "Parse: Success.") {
		WangYingXmlParser::CDocument xmlDocument;
		wangYingXmlParser.GetXmlDocument(&xmlDocument);
		_ShowParseXmlTree(xmlDocument);
	}
	return S_OK;
}

// Scintilla：获取文本
HRESULT CMainDlg::_GetScintillaText(std::string *pstrText)
{
	assert(pstrText != NULL);
	SOUI::SRealWnd *pRealWnd = FindChildByName2<SOUI::SRealWnd>(L"realwnd_xml_edit");
	assert(pRealWnd != NULL);
	CScintillaWnd *pXmlEdit = (CScintillaWnd*)pRealWnd->GetUserData();
	assert(pXmlEdit != NULL);
	// 获取：文本长度
	int n = pXmlEdit->SendMessage(SCI_GETTEXT, 0, 0);
	if (n == 0) return S_OK;
	char *chText = new char[n];
	// 获取：通过发送宏定义消息获取当前指定长度的文本信息
	pXmlEdit->SendMessage(SCI_GETTEXT, n, (LPARAM)chText);
	*pstrText = chText;
	delete chText;
	return S_OK;
}

// Scintilla：设置文本
HRESULT CMainDlg::_SetScintillaText(std::string strText)
{
	SOUI::SRealWnd *pRealWnd = FindChildByName2<SOUI::SRealWnd>(L"realwnd_xml_edit");
	assert(pRealWnd != NULL);
	CScintillaWnd *pXmlEdit = (CScintillaWnd*)pRealWnd->GetUserData();
	assert(pXmlEdit != NULL);
	// 设置：清空文本
	pXmlEdit->SendMessage(SCI_CLEARALL, 0, 0);
	// 设置：显示文本
	pXmlEdit->SendMessage(SCI_ADDTEXT, strText.length(), (LPARAM)(LPCWSTR)strText.c_str());
	return S_OK;
}

// 显示：解析树
HRESULT CMainDlg::_ShowParseXmlTree(WangYingXmlParser::CDocument xmlDocument)
{
	SOUI::STreeCtrl *pTreeCtrl = FindChildByName2<SOUI::STreeCtrl>(L"tree_ctrl_xml");
	assert(pTreeCtrl != NULL);
	for (auto item : xmlDocument.items) {
		wchar_t  ws[512];
		swprintf(ws, 512, L"%hs", item.name.c_str());
		SOUI::HTREEITEM treeItem = pTreeCtrl->InsertItem(ws);
		for (auto subItem : item.subitems) {
			_ParseSubItem(subItem, treeItem, pTreeCtrl);
		}
	}
	return S_OK;
}

// 显示：解析项目
HRESULT CMainDlg::_ParseSubItem(WangYingXmlParser::CItem subItem, SOUI::HTREEITEM treeItem, SOUI::STreeCtrl *pTreeCtrl)
{
	assert(pTreeCtrl != NULL); 
	wchar_t  ws[512];
	swprintf(ws, 512, L"%hs", subItem.name.c_str());
	SOUI::HTREEITEM subTreeItem = pTreeCtrl->InsertItem(ws, treeItem);
	for (auto subsubItem : subItem.subitems) {
		_ParseSubItem(subsubItem, subTreeItem, pTreeCtrl);
	}
	return S_OK;
}

// 清除：清除字符串中的 \r\n 空白字符
HRESULT CMainDlg::_ClearUnusefulChar(std::string &strXmlCode)
{
	for (auto it = strXmlCode.begin(); it != strXmlCode.end();) {
		if (*it == '\r' || *it == '\n') {
			it = strXmlCode.erase(it);
		} else {
			it++;
		}
	}
	return S_OK;
}

void CMainDlg::OnClose()
{
	CSimpleWnd::DestroyWindow();
}

void CMainDlg::OnMaximize()
{
	SendMessage(WM_SYSCOMMAND, SC_MAXIMIZE);
}
void CMainDlg::OnRestore()
{
	SendMessage(WM_SYSCOMMAND, SC_RESTORE);
}
void CMainDlg::OnMinimize()
{
	SendMessage(WM_SYSCOMMAND, SC_MINIMIZE);
}

void CMainDlg::OnSize(UINT nType, CSize size)
{
	SetMsgHandled(FALSE);
	if (!m_bLayoutInited) return;
	
	SWindow *pBtnMax = FindChildByName(L"btn_max");
	SWindow *pBtnRestore = FindChildByName(L"btn_restore");
	if(!pBtnMax || !pBtnRestore) return;
	
	if (nType == SIZE_MAXIMIZED)
	{
		pBtnRestore->SetVisible(TRUE);
		pBtnMax->SetVisible(FALSE);
	}
	else if (nType == SIZE_RESTORED)
	{
		pBtnRestore->SetVisible(FALSE);
		pBtnMax->SetVisible(TRUE);
	}
}

