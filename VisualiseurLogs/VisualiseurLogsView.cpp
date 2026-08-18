
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

/**
 * @brief Configure les styles de la fenêtre d'affichage de liste avant sa création physique.
 * 
 * Cette surcharge est cruciale car elle applique :
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
