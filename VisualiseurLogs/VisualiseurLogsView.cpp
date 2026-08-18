
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

IMPLEMENT_DYNCREATE(CVisualiseurLogsView, CView)

BEGIN_MESSAGE_MAP(CVisualiseurLogsView, CView)
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, &CVisualiseurLogsView::OnFilePrintPreview)
	ON_WM_CONTEXTMENU()
	ON_WM_RBUTTONUP()
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
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CView::PreCreateWindow(cs);
}

// CVisualiseurLogsView drawing

void CVisualiseurLogsView::OnDraw(CDC* /*pDC*/)
{
	CVisualiseurLogsDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	// TODO: add draw code for native data here
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
