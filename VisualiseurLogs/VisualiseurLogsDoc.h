
// VisualiseurLogsDoc.h : interface of the CVisualiseurLogsDoc class
//


#pragma once

#include "MappedFile.h"
#include <vector>
#include <atomic>
#include <thread>
#include <string>

class CVisualiseurLogsDoc : public CDocument
{
protected: // create from serialization only
	CVisualiseurLogsDoc() noexcept;
	DECLARE_DYNCREATE(CVisualiseurLogsDoc)

// Attributes
public:
	/**
	 * @brief Obtient le nombre de lignes à afficher (le nombre filtré si un filtre est actif, le nombre total sinon).
	 * @return size_t Nombre de lignes de log affichables.
	 */
	size_t GetLineCount() const noexcept;

	/**
	 * @brief Extrait, décode et nettoie une ligne de log spécifique par son index de ligne (0-based).
	 * @param index Index de la ligne demandée.
	 * @return CString Le contenu textuel Unicode de la ligne de log.
	 */
	CString GetLine(size_t index) const;

	/**
	 * @brief Démarre une recherche textuelle asynchrone dans un thread secondaire.
	 * @param strSearchText Texte recherché (mot-clé).
	 * @param hViewWnd HWND de la vue qui recevra les messages de notification d'avancement.
	 */
	void StartSearch(const CString& strSearchText, HWND hViewWnd);

	/**
	 * @brief Annule immédiatement la recherche asynchrone en cours si elle existe.
	 */
	void CancelSearch();

	/**
	 * @brief Applique les résultats de recherche mémorisés dans le document (exécuté sur le thread UI principal).
	 */
	void ApplySearchFilter();

	/**
	 * @brief Désactive le filtre de recherche actif et restaure l'affichage de l'intégralité du fichier.
	 */
	void ClearFilter();

	/**
	 * @brief Indique si une recherche en tâche de fond est actuellement en cours d'exécution.
	 * @return true si une recherche tourne, false sinon.
	 */
	bool IsSearching() const noexcept { return m_isSearching; }

	/**
	 * @brief Indique si un filtre de recherche est actuellement appliqué sur l'affichage.
	 * @return true si l'affichage est filtré, false sinon.
	 */
	bool HasActiveFilter() const noexcept { return m_hasActiveFilter; }

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
	/**
	 * @brief Méthode interne exécutée sur le thread de recherche secondaire (conforme cpp:S1188).
	 * Elle réalise le scan brut de la mémoire virtuelle du fichier.
	 *
	 * @param targetUTF8 Terme de recherche pré-converti en UTF-8.
	 * @param targetANSI Terme de recherche pré-converti en ANSI.
	 * @param hViewWnd Fenêtre recevant les messages d'UI de progression.
	 */
	void PerformSearchInternal(std::string targetUTF8, std::string targetANSI, HWND hViewWnd);

private:
	CMappedFile m_mappedFile;
	std::vector<uint64_t> m_lineOffsets;

	// Membres liés au moteur de recherche et filtrage asynchrone (Phase 4)
	std::vector<size_t> m_filteredLineIndices; ///< Liste des indices originaux correspondant au filtre actif.
	std::vector<size_t> m_tempMatches;          ///< Vecteur temporaire utilisé par le thread de recherche en tâche de fond.
	bool m_hasActiveFilter = false;            ///< Indique si un filtre est actuellement actif sur l'affichage.
	std::atomic<bool> m_isSearching = false;    ///< Flag atomique indiquant si le thread de recherche est actif.
	std::atomic<bool> m_cancelSearch = false;  ///< Flag atomique servant à demander l'interruption immédiate de la recherche.
	std::thread m_searchThread;                 ///< Objet thread pour gérer l'exécution asynchrone (Phase 4, conforme cpp:S5962).
};
