#pragma once

#include <windows.h>

/**
 * @class CMappedFile
 * @brief Classe RAII encapsulant les Memory-Mapped Files (MMF) de l'API Win32.
 * 
 * Cette classe permet d'ouvrir un fichier de grande taille en lecture seule
 * et de le projeter directement dans l'espace d'adressage virtuel du processus.
 * Elle garantit la libération automatique des handles et de la projection mémoire
 * à la destruction de l'objet (RAII). La copie est interdite, mais le déplacement
 * (move semantics) est supporté.
 */
class CMappedFile
{
public:
    /**
     * @brief Constructeur par défaut. Initialise l'objet dans un état vide/fermé.
     */
    CMappedFile() noexcept;

    /**
     * @brief Destructeur. Assure la fermeture automatique du fichier et du mapping (RAII).
     */
    ~CMappedFile() noexcept;

    // Non-copiable pour éviter les doubles libérations de handles
    CMappedFile(const CMappedFile&) = delete;
    CMappedFile& operator=(const CMappedFile&) = delete;

    /**
     * @brief Constructeur de déplacement (Move Constructor).
     * @param other L'objet temporaire dont on va transférer les ressources.
     */
    CMappedFile(CMappedFile&& other) noexcept;

    /**
     * @brief Opérateur d'affectation par déplacement (Move Assignment Operator).
     * @param other L'objet temporaire dont on transfère les ressources.
     * @return Référence sur l'objet actuel.
     */
    CMappedFile& operator=(CMappedFile&& other) noexcept;

    /**
     * @brief Ouvre le fichier et le projette entièrement en mémoire virtuelle.
     * @param lpszPathName Chemin complet du fichier à ouvrir.
     * @return true si l'ouverture et le mapping ont réussi, false sinon.
     */
    bool Open(LPCTSTR lpszPathName) noexcept;

    /**
     * @brief Ferme le mapping, détruit la projection et ferme le fichier sous-jacent.
     * Libère proprement toutes les ressources Win32 associées.
     */
    void Close() noexcept;

    /**
     * @brief Obtient un pointeur brut sur le début du fichier mappé en mémoire.
     * @return const char* Pointeur sur le contenu, ou nullptr si aucun fichier n'est mappé.
     */
    const char* GetData() const noexcept { return m_pData; }

    /**
     * @brief Obtient la taille totale du fichier en octets.
     * @return uint64_t Taille du fichier.
     */
    uint64_t GetSize() const noexcept { return m_fileSize; }

    /**
     * @brief Indique si un fichier est actuellement ouvert et projeté en mémoire.
     * @return true si le mapping est actif, false sinon.
     */
    bool IsOpen() const noexcept { return m_pData != nullptr; }

private:
    HANDLE m_hFile;      ///< Handle Win32 du fichier physique ouvert.
    HANDLE m_hMapping;   ///< Handle Win32 de l'objet de mapping de fichier.
    const char* m_pData; ///< Pointeur sur la zone de mémoire virtuelle où le fichier est mappé.
    uint64_t m_fileSize; ///< Taille totale du fichier indexé.
};
