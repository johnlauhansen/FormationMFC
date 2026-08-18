
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

// Enregistrement du message système standard de communication avec la boîte "Rechercher"
static const UINT wm_FindReplaceMsg = ::RegisterWindowMessage(FINDMSGSTRING);

IMPLEMENT_DYNCREATE(CVisualiseurLogsView, CListView)

BEGIN_MESSAGE_MAP(CVisualiseurLogsView, CListView)
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, &CListView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, &CListView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, &CVisualiseurLogsView::OnFilePrintPreview)
	ON_WM_CONTEXTMENU()
	ON_WM_RBUTTONUP()
	ON_NOTIFY_REFLECT(LVN_GETDISPINFO, &CVisualiseurLogsView::OnGetDispInfo)
	ON_COMMAND(ID_EDIT_FIND, &CVisualiseurLogsView::OnEditFind)
	ON_REGISTERED_MESSAGE(wm_FindReplaceMsg, &CVisualiseurLogsView::OnFindReplace)
	ON_MESSAGE(WM_USER + 100, &CVisualiseurLogsView::OnSearchProgress)
	ON_MESSAGE(WM_USER + 101, &CVisualiseurLogsView::OnSearchComplete)
END_MESSAGE_MAP()

// CVisualiseurLogsView construction/destruction

CVisualiseurLogsView::CVisualiseurLogsView() noexcept
{
	// TODO: add construction code here

}

CVisualiseurLogsView::~CVisualiseurLogsView()
{
	if (m_pFindDlg != nullptr)
	{
		m_pFindDlg->DestroyWindow();
		m_pFindDlg = nullptr;
	}
}

/**
 * @brief Configure les styles de la fenêtre d'affichage de liste avant sa création physique.
 * 
 * Cette surcharge applique :
 * 1. Le style de rapport complet (LVS_REPORT) pour gérer les colonnes.
 * 2. Le style virtuel (LVS_OWNERDATA) : empêche le stockage interne des données et active les notifications LVN_GETDISPINFO.
 * 3. La sélection simple (LVS_SINGLESEL).
 *
 * @param cs Structure CREATESTRUCT contenant les paramètres de création de la fenêtre.
 * @return BOOL TRUE si la pré-création est validée, FALSE sinon.
 */
BOOL CVisualiseurLogsView::PreCreateWindow(CREATESTRUCT& cs)
{
	cs.style &= ~LVS_TYPEMASK;
	cs.style |= LVS_REPORT | LVS_OWNERDATA | LVS_SINGLESEL;

	return CListView::PreCreateWindow(cs);
}

/**
 * @brief Initialise le composant graphique de liste après sa création.
 * 
 * Configure les styles étendus (LVS_EX_FULLROWSELECT pour sélectionner toute la ligne,
 * LVS_EX_GRIDLINES pour afficher le quadrillage, LVS_EX_DOUBLEBUFFER contre le scintillement).
 * Insère également les deux colonnes principales ("Ligne" et "Message de Log").
 */
void CVisualiseurLogsView::OnInitialUpdate()
{
	CListView::OnInitialUpdate();

	CListCtrl& listCtrl = GetListCtrl();

	listCtrl.SetExtendedStyle(listCtrl.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_DOUBLEBUFFER);

	listCtrl.InsertColumn(0, _T("Ligne"), LVCFMT_LEFT, 80);
	listCtrl.InsertColumn(1, _T("Message de Log"), LVCFMT_LEFT, 800);

	OnUpdate(nullptr, 0, nullptr);
}

/**
 * @brief Intercepte les messages clavier avant qu'ils ne soient distribués au contrôle SysListView32.
 * 
 * Cette surcharge est essentielle car elle intercepte directement la combinaison Ctrl + F au niveau clavier,
 * de manière totalement indépendante des tables de ressources d'accélérateurs (qui peuvent être désactivées ou
 * surchargées par le moteur MDI de MFC lors de l'activation des documents). En détectant l'appui sur 'F' avec la
 * touche CTRL enfoncée, on déclenche directement l'ouverture de notre boîte de dialogue de recherche OnEditFind.
 *
 * @param pMsg Pointeur vers le message système Windows reçu.
 * @return BOOL TRUE si le message a été traduit et traité (consommé), FALSE s'il doit suivre son cours normal.
 */
BOOL CVisualiseurLogsView::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN)
	{
		// CTRL + F
		if (pMsg->wParam == 'F' && (::GetKeyState(VK_CONTROL) & 0x8000))
		{
			OnEditFind();
			return TRUE;
		}
	}

	return CListView::PreTranslateMessage(pMsg);
}

/**
 * @brief Surchargé pour synchroniser et rafraîchir l'affichage lorsque les données du document changent.
 * 
 * Récupère le nombre total de lignes indexées dans le document, l'applique au contrôle de liste virtuelle
 * via SetItemCountEx (avec optimisations NOSCROLL/NOINVALIDATEALL), puis force le réaffichage avec Invalidate.
 *
 * @param pSender Pointeur vers la vue qui a initié l'action (nullptr par défaut).
 * @param lHint Paramètre d'information spécifique (0 par défaut).
 * @param pHint Objet d'information spécifique (nullptr par défaut).
 */
void CVisualiseurLogsView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint)
{
	CVisualiseurLogsDoc* pDoc = GetDocument();
	if (pDoc != nullptr)
	{
		CListCtrl& listCtrl = GetListCtrl();
		size_t lineCount = pDoc->GetLineCount();
		
		listCtrl.SetItemCountEx(static_cast<int>(lineCount), LVSICF_NOSCROLL | LVSICF_NOINVALIDATEALL);
		
		listCtrl.Invalidate();
	}
}

/**
 * @brief Intercepte la notification de dessin Win32 (LVN_GETDISPINFO) émise par le contrôle de liste virtuelle.
 * 
 * Cette méthode ultra-rapide n'est appelée par Windows que pour les éléments physiques visibles à l'écran :
 * 1. Pour la colonne 0 : elle formate et injecte le numéro de ligne physique (index + 1).
 * 2. Pour la colonne 1 : elle récupère le texte du log décodé depuis le document.
 * 3. SÉCURITÉ : elle tronque dynamiquement à la taille maximale autorisée par le tampon Windows (pItem->cchTextMax)
 *    pour éviter tout plantage (invalid parameter crash) avec des lignes géantes (> 10 000 car.).
 *
 * @param pNMHDR Pointeur vers l'entête du message de notification de contrôle standard.
 * @param pResult Pointeur vers le résultat renvoyé par la procédure de fenêtre (0 par défaut).
 */
void CVisualiseurLogsView::OnGetDispInfo(NMHDR* pNMHDR, LRESULT* pResult)
{
	auto pDispInfo = static_cast<NMLVDISPINFO*>(static_cast<void*>(pNMHDR));
	LVITEM* pItem = &pDispInfo->item;

	const CVisualiseurLogsDoc* pDoc = GetDocument();
	if (pDoc != nullptr && (pItem->mask & LVIF_TEXT))
	{
		int index = pItem->iItem; 

		if (pItem->iSubItem == 0) // Colonne 0 : Affichage du numéro de ligne
		{
			CString strLineNum;
			strLineNum.Format(_T("%u"), index + 1);
			_tcscpy_s(pItem->pszText, pItem->cchTextMax, strLineNum);
		}
		else if (pItem->iSubItem == 1) // Colonne 1 : Affichage du texte de log extrait à la demande
		{
			CString strLog = pDoc->GetLine(static_cast<size_t>(index));

			// Contrôle de la taille du tampon fournie par Windows (pItem->cchTextMax) pour les grandes lignes
			if (strLog.GetLength() >= pItem->cchTextMax)
			{
				if (pItem->cchTextMax > 4)
				{
					// Troncature et ajout du suffixe "..." pour signifier un message tronqué
					strLog = strLog.Left(pItem->cchTextMax - 4) + _T("...");
				}
				else
				{
					strLog = strLog.Left(pItem->cchTextMax - 1);
				}
			}

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

/**
 * @brief Commande appelée pour déclencher la recherche (ex: Ctrl + F ou Menu Edition ➔ Rechercher).
 * Alloue dynamiquement et affiche la boîte de dialogue standard modeless "Rechercher" de Windows.
 */
void CVisualiseurLogsView::OnEditFind()
{
	if (m_pFindDlg != nullptr)
	{
		m_pFindDlg->SetActiveWindow();
		return;
	}

	m_pFindDlg = new CFindReplaceDialog();
	m_pFindDlg->Create(TRUE, _T(""), nullptr, FR_DOWN, this);
}

/**
 * @brief Intercepte les notifications envoyées par la boîte de dialogue standard "Rechercher".
 * 
 * Ce gestionnaire reçoit les messages enregistrés lorsque l'utilisateur :
 * 1. Clique sur "Suivant" (FindNext) ➔ Déclenche la recherche asynchrone dans le document.
 * 2. Ferme le dialogue (IsTerminating) ➔ Réinitialise notre pointeur m_pFindDlg.
 *
 * @param wParam Paramètre non utilisé dans cette notification.
 * @param lParam Pointeur vers la structure FINDREPLACE standard.
 * @return LRESULT Résultat d'exécution (0 par défaut).
 */
LRESULT CVisualiseurLogsView::OnFindReplace(WPARAM wParam, LPARAM lParam)
{
	if (m_pFindDlg == nullptr)
	{
		return 0;
	}

	if (m_pFindDlg->IsTerminating())
	{
		m_pFindDlg = nullptr;
		return 0;
	}

	if (m_pFindDlg->FindNext())
	{
		CString strFind = m_pFindDlg->GetFindString();
		CVisualiseurLogsDoc* pDoc = GetDocument();
		if (pDoc != nullptr)
		{
			CFrameWnd* pMainWnd = static_cast<CFrameWnd*>(AfxGetMainWnd());
			if (pMainWnd != nullptr)
			{
				pMainWnd->SetMessageText(_T("Recherche lancée..."));
			}
			
			pDoc->StartSearch(strFind, GetSafeHwnd());
		}
	}

	return 0;
}

/**
 * @brief Reçoit les rapports de progression émis périodiquement par le thread de recherche asynchrone.
 * 
 * Met à jour dynamiquement le statut dans la barre d'état principale de l'application :
 * ex: "Recherche en cours... 45% (3 241 correspondances trouvées)".
 *
 * @param wParam Pourcentage d'avancement de la recherche (0-100).
 * @param lParam Nombre de correspondances identifiées jusqu'à présent.
 * @return LRESULT 0
 */
LRESULT CVisualiseurLogsView::OnSearchProgress(WPARAM wParam, LPARAM lParam)
{
	auto progress = static_cast<int>(wParam);
	auto matchCount = static_cast<size_t>(lParam);

	CString strStatus;
	strStatus.Format(_T("Recherche en cours... %d%% (%u correspondances trouvées)"), progress, static_cast<unsigned int>(matchCount));
	
	auto pMainWnd = static_cast<CFrameWnd*>(AfxGetMainWnd());
	if (pMainWnd != nullptr)
	{
		pMainWnd->SetMessageText(strStatus);
	}

	return 0;
}

/**
 * @brief Appelé sur le thread UI principal lorsque la recherche en tâche de fond est terminée.
 * 
 * Applique le filtre final sur le document, force l'actualisation complète de la liste virtuelle
 * et affiche le bilan final dans la barre d'état principale de l'application.
 *
 * @param wParam Nombre total de correspondances trouvées.
 * @param lParam Non utilisé.
 * @return LRESULT 0
 */
LRESULT CVisualiseurLogsView::OnSearchComplete(WPARAM wParam, LPARAM lParam)
{
	auto totalMatches = static_cast<size_t>(wParam);

	CVisualiseurLogsDoc* pDoc = GetDocument();
	if (pDoc != nullptr)
	{
		pDoc->ApplySearchFilter(); // Transfert des résultats et rafraîchissement complet (Invalidate)
	}

	CString strStatus;
	if (totalMatches > 0)
	{
		strStatus.Format(_T("Recherche terminée ! %u lignes correspondantes affichées."), static_cast<unsigned int>(totalMatches));
	}
	else
	{
		strStatus = _T("Recherche terminée : aucune ligne correspondante trouvée.");
	}

	CFrameWnd* pMainWnd = static_cast<CFrameWnd*>(AfxGetMainWnd());
	if (pMainWnd != nullptr)
	{
		pMainWnd->SetMessageText(strStatus);
	}

	return 0;
}
