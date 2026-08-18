
// VisualiseurLogsDoc.cpp : implementation of the CVisualiseurLogsDoc class
//

#include "pch.h"
#include "framework.h"
// SHARED_HANDLERS can be defined in an ATL project implementing preview, thumbnail
// and search filter handlers and allows sharing of document code with that project.
#ifndef SHARED_HANDLERS
#include "VisualiseurLogs.h"
#endif

#include "VisualiseurLogsDoc.h"

#include <propkey.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CVisualiseurLogsDoc

IMPLEMENT_DYNCREATE(CVisualiseurLogsDoc, CDocument)

BEGIN_MESSAGE_MAP(CVisualiseurLogsDoc, CDocument)
END_MESSAGE_MAP()


// CVisualiseurLogsDoc construction/destruction

CVisualiseurLogsDoc::CVisualiseurLogsDoc() noexcept
{
	// TODO: add one-time construction code here

}

CVisualiseurLogsDoc::~CVisualiseurLogsDoc()
{
}

BOOL CVisualiseurLogsDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: add reinitialization code here
	// (SDI documents will reuse this document)

	return TRUE;
}




// CVisualiseurLogsDoc serialization

void CVisualiseurLogsDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: add storing code here
	}
	else
	{
		// TODO: add loading code here
	}
}

#ifdef SHARED_HANDLERS

// Support for thumbnails
void CVisualiseurLogsDoc::OnDrawThumbnail(CDC& dc, LPRECT lprcBounds)
{
	// Modify this code to draw the document's data
	dc.FillSolidRect(lprcBounds, RGB(255, 255, 255));

	CString strText = _T("TODO: implement thumbnail drawing here");
	LOGFONT lf;

	CFont* pDefaultGUIFont = CFont::FromHandle((HFONT) GetStockObject(DEFAULT_GUI_FONT));
	pDefaultGUIFont->GetLogFont(&lf);
	lf.lfHeight = 36;

	CFont fontDraw;
	fontDraw.CreateFontIndirect(&lf);

	CFont* pOldFont = dc.SelectObject(&fontDraw);
	dc.DrawText(strText, lprcBounds, DT_CENTER | DT_WORDBREAK);
	dc.SelectObject(pOldFont);
}

// Support for Search Handlers
void CVisualiseurLogsDoc::InitializeSearchContent()
{
	CString strSearchContent;
	// Set search contents from document's data.
	// The content parts should be separated by ";"

	// For example:  strSearchContent = _T("point;rectangle;circle;ole object;");
	SetSearchContent(strSearchContent);
}

void CVisualiseurLogsDoc::SetSearchContent(const CString& value)
{
	if (value.IsEmpty())
	{
		RemoveChunk(PKEY_Search_Contents.fmtid, PKEY_Search_Contents.pid);
	}
	else
	{
		CMFCFilterChunkValueImpl *pChunk = nullptr;
		ATLTRY(pChunk = new CMFCFilterChunkValueImpl);
		if (pChunk != nullptr)
		{
			pChunk->SetTextValue(PKEY_Search_Contents, value, CHUNK_TEXT);
			SetChunkValue(pChunk);
		}
	}
}

#endif // SHARED_HANDLERS

// CVisualiseurLogsDoc diagnostics

#ifdef _DEBUG
void CVisualiseurLogsDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CVisualiseurLogsDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG


// CVisualiseurLogsDoc commands

/**
 * @brief Surcharge MFC appelée lors de l'ouverture d'un fichier (ex: Fichier ➔ Ouvrir).
 * 
 * Cette méthode ouvre le fichier via notre moteur CMappedFile, puis réalise une
 * indexation ultra-rapide en un seul passage (single-scan) des positions (offsets) de
 * chaque début de ligne (\n). Elle notifie ensuite les vues pour mettre à jour l'affichage.
 *
 * @param lpszPathName Chemin complet du fichier sélectionné par l'utilisateur.
 * @return BOOL TRUE si le document a été ouvert et indexé avec succès, FALSE sinon.
 */
BOOL CVisualiseurLogsDoc::OnOpenDocument(LPCTSTR lpszPathName)
{
	DeleteContents();

	if (!m_mappedFile.Open(lpszPathName))
	{
		CString strError;
		strError.Format(_T("Impossible d'ouvrir et de projeter le fichier : %s"), lpszPathName);
		AfxMessageBox(strError, MB_ICONERROR);
		return FALSE;
	}

	const char* pData = m_mappedFile.GetData();
	uint64_t fileSize = m_mappedFile.GetSize();

	if (fileSize > 0 && pData != nullptr)
	{
		m_lineOffsets.reserve(fileSize / 100);

		m_lineOffsets.push_back(0);

		for (uint64_t i = 0; i < fileSize; ++i)
		{
			if (pData[i] == '\n' && i + 1 < fileSize)
			{
				m_lineOffsets.push_back(i + 1);
			}
		}
	}

	SetModifiedFlag(FALSE);

	UpdateAllViews(nullptr);

	return TRUE;
}

/**
 * @brief Nettoie le contenu actuel du document et réinitialise tous les membres.
 * 
 * Appelée automatiquement par MFC avant de réutiliser ou de détruire le document.
 * Elle assure la fermeture du mapping mémoire et libère la capacité allouée par
 * le vecteur d'offsets (shrink_to_fit).
 */
void CVisualiseurLogsDoc::DeleteContents()
{
	m_mappedFile.Close();
	m_lineOffsets.clear();
	m_lineOffsets.shrink_to_fit(); // Libère réellement la RAM système du vecteur d'offsets

	CDocument::DeleteContents();
}

/**
 * @brief Récupère, convertit et renvoie une ligne spécifique du fichier log sous forme de CString Unicode.
 * 
 * Cette méthode est optimisée pour l'On-Demand Rendering (pas de copie) :
 * 1. Elle délimite la ligne demandée à l'aide de l'index d'offsets (m_lineOffsets[index] et suivant).
 * 2. Elle retire les caractères de contrôle (\r, \n) de fin de ligne.
 * 3. Elle convertit le buffer brut (UTF-8 ou ANSI) en UTF-16 Unicode via MultiByteToWideChar.
 *
 * @param index Index de la ligne demandée (0-based).
 * @return CString La chaîne décodée et prête à être affichée. Retourne une chaîne vide si index invalide.
 */
CString CVisualiseurLogsDoc::GetLine(size_t index) const
{
	// Sécurité d'index
	if (index >= m_lineOffsets.size() || !m_mappedFile.IsOpen())
	{
		return CString();
	}

	const char* pData = m_mappedFile.GetData();
	uint64_t fileSize = m_mappedFile.GetSize();
	uint64_t startOffset = m_lineOffsets[index];
	uint64_t endOffset = 0;

	if (index + 1 < m_lineOffsets.size())
	{
		endOffset = m_lineOffsets[index + 1];
	}
	else
	{
		endOffset = fileSize;
	}

	size_t length = endOffset - startOffset;
	if (length == 0)
	{
		return CString();
	}

	const char* pLineStart = pData + startOffset;

	while (length > 0 && (pLineStart[length - 1] == '\r' || pLineStart[length - 1] == '\n'))
	{
		length--;
	}

	if (length == 0)
	{
		return CString();
	}

	// Étape 1 : Tentative de conversion depuis UTF-8 (Code Page standard moderne)
	int requiredCharCount = ::MultiByteToWideChar(CP_UTF8, 0, pLineStart, static_cast<int>(length), nullptr, 0);
	if (requiredCharCount > 0)
	{
		CString strResult;
		wchar_t* pBuffer = strResult.GetBuffer(requiredCharCount);
		::MultiByteToWideChar(CP_UTF8, 0, pLineStart, static_cast<int>(length), pBuffer, requiredCharCount);
		strResult.ReleaseBufferSetLength(requiredCharCount);
		return strResult;
	}
	else
	{
		// Étape 2 : Fallback sur l'ANSI local (CP_ACP) si le fichier contient des octets UTF-8 invalides
		requiredCharCount = ::MultiByteToWideChar(CP_ACP, 0, pLineStart, static_cast<int>(length), nullptr, 0);
		if (requiredCharCount > 0)
		{
			CString strResult;
			wchar_t* pBuffer = strResult.GetBuffer(requiredCharCount);
			::MultiByteToWideChar(CP_ACP, 0, pLineStart, static_cast<int>(length), pBuffer, requiredCharCount);
			strResult.ReleaseBufferSetLength(requiredCharCount);
			return strResult;
		}
	}

	return CString();
}
