
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
#include <string_view>
#include <string>
#include <thread>

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
	CancelSearch();
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
 * Cette méthode ouvre le fichier via le CMappedFile, puis réalise une
 * indexation rapide en un seul passage (single-scan) des positions (offsets) de
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
 * @brief Obtient le nombre de lignes à afficher (le nombre filtré si un filtre est actif, le nombre total sinon).
 * @return size_t Nombre de lignes de log affichables.
 */
size_t CVisualiseurLogsDoc::GetLineCount() const noexcept
{
	return m_hasActiveFilter ? m_filteredLineIndices.size() : m_lineOffsets.size();
}

/**
 * @brief Nettoie le contenu actuel du document et réinitialise tous les membres.
 * 
 * Appelée automatiquement par MFC avant de réutiliser ou de détruire le document.
 * Elle assure la fermeture du mapping mémoire, annule une recherche en cours et libère la capacité allouée.
 */
void CVisualiseurLogsDoc::DeleteContents()
{
	CancelSearch();

	m_mappedFile.Close();
	m_lineOffsets.clear();
	m_lineOffsets.shrink_to_fit(); // Libère réellement la RAM système du vecteur d'offsets

	m_filteredLineIndices.clear();
	m_filteredLineIndices.shrink_to_fit();
	m_tempMatches.clear();
	m_tempMatches.shrink_to_fit();
	m_hasActiveFilter = false;

	CDocument::DeleteContents();
}

/**
 * @brief Récupère, convertit et renvoie une ligne spécifique du fichier log sous forme de CString Unicode.
 * 
 * Surchargé pour prendre en compte le filtrage dynamique si un filtre est actif.
 *
 * @param index Index de la ligne demandée (0-based dans le référentiel affiché).
 * @return CString La chaîne décodée et prête à être affichée.
 */
CString CVisualiseurLogsDoc::GetLine(size_t index) const
{
	// Sécurité d'index
	if (!m_mappedFile.IsOpen())
	{
		return CString();
	}

	size_t originalIndex = index;
	if (m_hasActiveFilter)
	{
		if (index >= m_filteredLineIndices.size())
		{
			return CString();
		}
		originalIndex = m_filteredLineIndices[index]; // Résolution de l'index réel à partir de la ligne filtrée
	}

	if (originalIndex >= m_lineOffsets.size())
	{
		return CString();
	}

	const char* pData = m_mappedFile.GetData();
	uint64_t fileSize = m_mappedFile.GetSize();
	uint64_t startOffset = m_lineOffsets[originalIndex];
	uint64_t endOffset = 0;

	if (originalIndex + 1 < m_lineOffsets.size())
	{
		endOffset = m_lineOffsets[originalIndex + 1];
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


/**
 * @brief Démarre une recherche textuelle asynchrone dans un thread secondaire de manière ultra-rapide.
 * 
 * Pour éviter toute corruption de données ou de conversion de CString à travers les threads (les macros de conversion
 * ATL utilisent un cache local au thread principal), le mot-clé de recherche est converti en std::string (UTF-8 et ANSI)
 * sur le thread principal (UI) où l'environnement est garanti sain, puis ces objets std::string sont passés par valeur au thread.
 *
 * @param strSearchText Texte recherché (mot-clé).
 * @param hViewWnd HWND de la vue réceptrice des notifications.
 */
void CVisualiseurLogsDoc::StartSearch(const CString& strSearchText, HWND hViewWnd)
{
	CancelSearch(); // Sécurité : on arrête et rejoint une recherche en cours

	if (strSearchText.IsEmpty())
	{
		ClearFilter();
		return;
	}

	m_isSearching = true;
	m_cancelSearch = false;
	m_tempMatches.clear();

	// Conversion de sécurité absolue sur le thread principal (UI) pour éviter les bugs de cache d'ATL sur thread secondaire
	std::string targetUTF8 = CW2A(strSearchText, CP_UTF8);
	std::string targetANSI = CW2A(strSearchText, CP_ACP);

	// Lancement conforme du thread secondaire en lui passant les chaînes std::string copiées par valeur
	m_searchThread = std::thread(&CVisualiseurLogsDoc::PerformSearchInternal, this, targetUTF8, targetANSI, hViewWnd);
}

/**
 * @brief Méthode interne exécutée sur le thread de recherche secondaire (conforme cpp:S1188).
 * Scanne la mémoire brute projetée en utilisant std::string_view::find (zéro allocation de chaîne).
 *
 * @param targetUTF8 Terme de recherche pré-converti en UTF-8.
 * @param targetANSI Terme de recherche pré-converti en ANSI.
 * @param hViewWnd Fenêtre recevant les messages d'UI de progression.
 */
void CVisualiseurLogsDoc::PerformSearchInternal(std::string targetUTF8, std::string targetANSI, HWND hViewWnd)
{
	std::vector<size_t> localMatches;
	const char* pData = m_mappedFile.GetData();
	uint64_t fileSize = m_mappedFile.GetSize();
	size_t totalLines = m_lineOffsets.size();

	for (size_t i = 0; i < totalLines; ++i)
	{
		// Interruption rapide demandée par le thread principal
		if (m_cancelSearch)
		{
			break;
		}

		uint64_t start = m_lineOffsets[i];
		uint64_t end = (i + 1 < totalLines) ? m_lineOffsets[i + 1] : fileSize;

		size_t length = static_cast<size_t>(end - start);
		while (length > 0 && (pData[start + length - 1] == '\r' || pData[start + length - 1] == '\n'))
		{
			length--;
		}

		if (length > 0)
		{
			std::string_view lineView(pData + start, length);
			
			// Recherche de sous-chaîne brute ultra-rapide (sans allocation mémoire)
			if (lineView.find(targetUTF8) != std::string_view::npos ||
				(targetUTF8 != targetANSI && lineView.find(targetANSI) != std::string_view::npos))
			{
				localMatches.push_back(i);
			}
		}

		// Notification d'avancement de manière régulière à la boucle de messages UI (toutes les 50k lignes ou fin)
		if (i > 0 && (i % 50000 == 0 || i == totalLines - 1))
		{
			int progress = static_cast<int>((i * 100) / totalLines);
			::PostMessage(hViewWnd, WM_USER + 100, static_cast<WPARAM>(progress), static_cast<LPARAM>(localMatches.size()));
		}
	}

	if (!m_cancelSearch)
	{
		// Sauvegarde sécurisée des résultats temporaires
		m_tempMatches = std::move(localMatches);

		// Envoi du message final (WM_USER + 101) indiquant le nombre de résultats trouvés
		::PostMessage(hViewWnd, WM_USER + 101, static_cast<WPARAM>(m_tempMatches.size()), 0);
	}

	m_isSearching = false;
}

/**
 * @brief Annule immédiatement la recherche asynchrone en cours.
 * 
 * SÉCURITÉ MULTITHREAD MAJEURE :
 * Un objet std::thread reste "joinable" tant qu'on n'a pas appelé join() dessus, même si le thread a fini
 * de s'exécuter naturellement. Si on tente d'affecter un nouveau thread à un objet std::thread encore "joinable",
 * le runtime C++ déclenche immédiatement un appel à std::terminate() / abort().
 * Nous devons donc impérativement appeler join() dès que le thread est joinable(), peu importe l'état de m_isSearching.
 */
void CVisualiseurLogsDoc::CancelSearch()
{
	// On signale la demande d'interruption au cas où le thread est encore actif
	m_cancelSearch = true;
	
	// Si un thread est actif ou s'est terminé naturellement, on libère ses ressources systèmes
	if (m_searchThread.joinable())
	{
		m_searchThread.join();
	}

	m_isSearching = false;
}

/**
 * @brief Applique le filtre de recherche final sur le thread principal (UI).
 * Cette transition est 100% thread-safe.
 */
void CVisualiseurLogsDoc::ApplySearchFilter()
{
	m_filteredLineIndices = std::move(m_tempMatches);
	m_hasActiveFilter = true;
	UpdateAllViews(nullptr);
}

/**
 * @brief Désactive le filtrage de recherche actif et actualise toutes les vues.
 */
void CVisualiseurLogsDoc::ClearFilter()
{
	CancelSearch();
	m_filteredLineIndices.clear();
	m_filteredLineIndices.shrink_to_fit();
	m_hasActiveFilter = false;
	UpdateAllViews(nullptr);
}
