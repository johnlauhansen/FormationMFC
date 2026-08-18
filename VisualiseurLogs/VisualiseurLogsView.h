
// VisualiseurLogsView.h : interface of the CVisualiseurLogsView class
//

#pragma once

#include <afxdlgs.h> // Requis pour CFindReplaceDialog

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
	virtual BOOL PreTranslateMessage(MSG* pMsg) override;
protected:
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) override;
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
	afx_msg void OnGetDispInfo(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCustomDraw(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnEditFind();
	afx_msg LRESULT OnFindReplace(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnSearchProgress(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnSearchComplete(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()

private:
	CFindReplaceDialog* m_pFindDlg = nullptr; ///< Pointeur vers la boîte de dialogue modèle standard "Rechercher".
};

#ifndef _DEBUG  // debug version in VisualiseurLogsView.cpp
inline CVisualiseurLogsDoc* CVisualiseurLogsView::GetDocument() const
   { return static_cast<CVisualiseurLogsDoc*>(m_pDocument); }
#endif

