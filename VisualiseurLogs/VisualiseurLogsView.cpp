
// VisualiseurLogsView.cpp : implementation of the CVisualiseurLogsView class
//

#include "pch.h"
#include "framework.h"
// SHARED_HANDLERS can be defined in an ATL project implementing preview, thumbnail
// and search filter handlers and allows sharing of document code with that project.
#ifndef SHARED_HANDLERS
#include "VisualiseurLogs.h"
#endif

#include "VisualiseurLogsDoc.h"
#include "VisualiseurLogsView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CVisualiseurLogsView

IMPLEMENT_DYNCREATE(CVisualiseurLogsView, CListView)

BEGIN_MESSAGE_MAP(CVisualiseurLogsView, CListView)
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, &CListView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, &CListView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, &CVisualiseurLogsView::OnFilePrintPreview)
	ON_WM_CONTEXTMENU()
	ON_WM_RBUTTONUP()
	ON_NOTIFY_REFLECT(LVN_GETDISPINFO, &CVisualiseurLogsView::OnGetDispInfo)
END_MESSAGE_MAP()

// CVisualiseurLogsView construction/destruction

CVisualiseurLogsView::CVisualiseurLogsView() noexcept
{
	// TODO: add construction code here

}

CVisualiseurLogsView::~CVisualiseurLogsView()
{
}

BOOL CVisualiseurLogsView::PreCreateWindow(CREATESTRUCT& cs)
{
	cs.style &= ~LVS_TYPEMASK;
	cs.style |= LVS_REPORT | LVS_OWNERDATA | LVS_SINGLESEL;

	return CView::PreCreateWindow(cs);
}

void CVisualiseurLogsView::OnInitialUpdate()
{
	CListView::OnInitialUpdate();

	CListCtrl& listCtrl = GetListCtrl();


	listCtrl.SetExtendedStyle(listCtrl.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_DOUBLEBUFFER);

	listCtrl.InsertColumn(0, _T("Ligne"), LVCFMT_LEFT, 80);
	listCtrl.InsertColumn(1, _T("Message de Log"), LVCFMT_LEFT, 800);

	listCtrl.SetItemCountEx(1000000, LVSICF_NOSCROLL | LVSICF_NOINVALIDATEALL);
}

void CVisualiseurLogsView::OnGetDispInfo(NMHDR* pNMHDR, LRESULT* pResult) const
{
	auto pDispInfo = static_cast<NMLVDISPINFO*>(static_cast<void*>(pNMHDR));
	LVITEM* pItem = &pDispInfo->item;

	if (pItem->mask & LVIF_TEXT)
	{
		int index = pItem->iItem; // Index de la ligne demandée

		if (pItem->iSubItem == 0)
		{
			CString strLineNum;
			strLineNum.Format(_T("%d"), index + 1);
			_tcscpy_s(pItem->pszText, pItem->cchTextMax, strLineNum);
		}
		else if (pItem->iSubItem == 1)
		{
			CString strLog;
			strLog.Format(_T("Log exemple numéro %d - [INFO] Application démarrée"), index + 1);
			_tcscpy_s(pItem->pszText, pItem->cchTextMax, strLog);
		}
	}

	*pResult = 0;
}

// CVisualiseurLogsView printing


void CVisualiseurLogsView::OnFilePrintPreview()
{
#ifndef SHARED_HANDLERS
	AFXPrintPreview(this);
#endif
}

BOOL CVisualiseurLogsView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default preparation
	return DoPreparePrinting(pInfo);
}

void CVisualiseurLogsView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add extra initialization before printing
}

void CVisualiseurLogsView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add cleanup after printing
}

void CVisualiseurLogsView::OnRButtonUp(UINT /* nFlags */, CPoint point)
{
	ClientToScreen(&point);
	OnContextMenu(this, point);
}

void CVisualiseurLogsView::OnContextMenu(CWnd* /* pWnd */, CPoint point)
{
#ifndef SHARED_HANDLERS
	theApp.GetContextMenuManager()->ShowPopupMenu(IDR_POPUP_EDIT, point.x, point.y, this, TRUE);
#endif
}


// CVisualiseurLogsView diagnostics

#ifdef _DEBUG
void CVisualiseurLogsView::AssertValid() const
{
	CView::AssertValid();
}

void CVisualiseurLogsView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CVisualiseurLogsDoc* CVisualiseurLogsView::GetDocument() const // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CVisualiseurLogsDoc)));
	return (CVisualiseurLogsDoc*)m_pDocument;
}
#endif //_DEBUG


// CVisualiseurLogsView message handlers
