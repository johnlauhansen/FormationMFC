#include "pch.h"
#include "MappedFile.h"

/**
 * @brief Constructeur par défaut.
 * Initialise tous les handles Win32 à leur état inactif de sécurité.
 */
CMappedFile::CMappedFile() noexcept
    : m_hFile(INVALID_HANDLE_VALUE)
    , m_hMapping(nullptr)
    , m_pData(nullptr)
    , m_fileSize(0)
{
}

/**
 * @brief Destructeur RAII.
 * Déclenche automatiquement l'appel de Close() pour garantir la libération des ressources de bas niveau.
 */
CMappedFile::~CMappedFile() noexcept
{
    Close();
}

/**
 * @brief Constructeur de déplacement.
 * Permet de transférer efficacement la propriété d'un fichier mappé (mots-clés && et noexcept).
 */
CMappedFile::CMappedFile(CMappedFile&& other) noexcept
    : m_hFile(other.m_hFile)
    , m_hMapping(other.m_hMapping)
    , m_pData(other.m_pData)
    , m_fileSize(other.m_fileSize)
{
    other.m_hFile = INVALID_HANDLE_VALUE;
    other.m_hMapping = nullptr;
    other.m_pData = nullptr;
    other.m_fileSize = 0;
}

/**
 * @brief Opérateur d'affectation par déplacement.
 * Nettoie d'abord les ressources courantes, puis transfère la propriété des nouvelles ressources.
 */
CMappedFile& CMappedFile::operator=(CMappedFile&& other) noexcept
{
    if (this != &other)
    {
        Close();

        m_hFile = other.m_hFile;
        m_hMapping = other.m_hMapping;
        m_pData = other.m_pData;
        m_fileSize = other.m_fileSize;

        other.m_hFile = INVALID_HANDLE_VALUE;
        other.m_hMapping = nullptr;
        other.m_pData = nullptr;
        other.m_fileSize = 0;
    }
    return *this;
}

/**
 * @brief Ouvre un fichier sur disque et projette tout son contenu dans la mémoire virtuelle de l'application.
 * 
 * Cette méthode réalise 3 étapes Win32 distinctes :
 * 1. Ouverture du fichier via CreateFile (en lecture seule partagée, optimisé pour parcours séquentiel).
 * 2. Création d'un objet de mapping de fichier via CreateFileMapping (en PAGE_READONLY).
 * 3. Projection complète de la vue du fichier en mémoire vive via MapViewOfFile (en FILE_MAP_READ).
 *
 * @param lpszPathName Chemin d'accès complet au fichier.
 * @return true si le fichier a été mappé avec succès, false en cas d'erreur.
 */
bool CMappedFile::Open(LPCTSTR lpszPathName) noexcept
{
    Close(); 

    m_hFile = ::CreateFile(
        lpszPathName,
        GENERIC_READ,
        FILE_SHARE_READ,
        nullptr,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, // Optimisé pour lecture séquentielle (Cache de lecture Windows)
        nullptr
    );

    if (m_hFile == INVALID_HANDLE_VALUE)
    {
        return false;
    }

    LARGE_INTEGER size;
    if (!::GetFileSizeEx(m_hFile, &size))
    {
        Close();
        return false;
    }
    m_fileSize = static_cast<uint64_t>(size.QuadPart);

    if (m_fileSize == 0)
    {
        return true;
    }

    m_hMapping = ::CreateFileMapping(
        m_hFile,
        nullptr,
        PAGE_READONLY,
        0,
        0,
        nullptr
    );

    if (m_hMapping == nullptr)
    {
        Close();
        return false;
    }

    m_pData = static_cast<const char*>(::MapViewOfFile(
        m_hMapping,
        FILE_MAP_READ,
        0,
        0,
        0
    ));

    if (m_pData == nullptr)
    {
        Close();
        return false;
    }

    return true;
}

/**
 * @brief Libère proprement toutes les ressources système associées au fichier.
 * 
 * L'ordre de fermeture inverse l'ordre d'acquisition (Reverse Order) :
 * 1. Annulation de la projection mémoire (UnmapViewOfFile).
 * 2. Fermeture de l'objet de mapping noyau (CloseHandle).
 * 3. Fermeture du fichier physique (CloseHandle).
 */
void CMappedFile::Close() noexcept
{
    if (m_pData != nullptr)
    {
        ::UnmapViewOfFile(m_pData);
        m_pData = nullptr;
    }

    if (m_hMapping != nullptr)
    {
        ::CloseHandle(m_hMapping);
        m_hMapping = nullptr;
    }

    if (m_hFile != INVALID_HANDLE_VALUE)
    {
        ::CloseHandle(m_hFile);
        m_hFile = INVALID_HANDLE_VALUE;
    }

    m_fileSize = 0;
}
