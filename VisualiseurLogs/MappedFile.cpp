#include "pch.h"
#include "MappedFile.h"

CMappedFile::CMappedFile() noexcept
    : m_hFile(INVALID_HANDLE_VALUE)
    , m_hMapping(nullptr)
    , m_pData(nullptr)
    , m_fileSize(0)
{
}

CMappedFile::~CMappedFile() noexcept
{
    Close();
}

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

bool CMappedFile::Open(LPCTSTR lpszPathName) noexcept
{
    Close();

    m_hFile = ::CreateFile(
        lpszPathName,
        GENERIC_READ,
        FILE_SHARE_READ,
        nullptr,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,
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
        // Un fichier de 0 octet ne peut pas être mappé.
        // On le considère ouvert mais sans mapping mémoire.
        return true;
    }

    m_hMapping = ::CreateFileMapping(m_hFile, nullptr, PAGE_READONLY, 0, 0, nullptr);

    if (m_hMapping == nullptr)
    {
        Close();
        return false;
    }

    m_pData = static_cast<const char*>(::MapViewOfFile(m_hMapping, FILE_MAP_READ, 0, 0, 0));

    if (m_pData == nullptr)
    {
        Close();
        return false;
    }

    return true;
}

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
