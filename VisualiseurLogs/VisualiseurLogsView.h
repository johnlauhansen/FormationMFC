
// VisualiseurLogsView.h : interface of the CVisualiseurLogsView class
//

#pragma once


class CVisualiseurLogsView : public CListView
{
protected: // create from serialization only
	CVisualiseurLogsView() noexcept;
	DECLARE_DYNCREATE(CVisualiseurLogsView)

// Attributes
public:
	CVisualiseurLogsDoc* GetDocument() const;

// Operations
public:

// Overrides
public:

	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual void OnInitialUpdate();
protected:
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);

// Implementation
public:
	virtual ~CVisualiseurLogsView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	afx_msg void OnFilePrintPreview();
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnGetDispInfo(NMHDR* pNMHDR, LRESULT* pResult) const;
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // debug version in VisualiseurLogsView.cpp
inline CVisualiseurLogsDoc* CVisualiseurLogsView::GetDocument() const
   { return static_cast<CVisualiseurLogsDoc*>(m_pDocument); }
#endif

