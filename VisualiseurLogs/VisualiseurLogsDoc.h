
// VisualiseurLogsDoc.h : interface of the CVisualiseurLogsDoc class
//


#pragma once

#include "MappedFile.h"
#include <vector>

class CVisualiseurLogsDoc : public CDocument
{
protected: // create from serialization only
	CVisualiseurLogsDoc() noexcept;
	DECLARE_DYNCREATE(CVisualiseurLogsDoc)

// Attributes
public:
	size_t GetLineCount() const noexcept { return m_lineOffsets.size(); }
	CString GetLine(size_t index) const;

// Operations
public:

// Overrides
public:
	virtual BOOL OnNewDocument();
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName) override;
	virtual void DeleteContents() override;
	virtual void Serialize(CArchive& ar);
#ifdef SHARED_HANDLERS
	virtual void InitializeSearchContent();
	virtual void OnDrawThumbnail(CDC& dc, LPRECT lprcBounds);
#endif // SHARED_HANDLERS

// Implementation
public:
	virtual ~CVisualiseurLogsDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()

#ifdef SHARED_HANDLERS
	// Helper function that sets search content for a Search Handler
	void SetSearchContent(const CString& value);
#endif // SHARED_HANDLERS

private:
	CMappedFile m_mappedFile;
	std::vector<uint64_t> m_lineOffsets;
};
