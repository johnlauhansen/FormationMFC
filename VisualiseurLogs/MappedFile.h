#pragma once

#include <windows.h>

class CMappedFile
{
public:
    CMappedFile() noexcept;
    ~CMappedFile() noexcept;

    // Non-copiable pour éviter les doubles libérations de handles
    CMappedFile(const CMappedFile&) = delete;
    CMappedFile& operator=(const CMappedFile&) = delete;

    // Déplaçable (Move semantics)
    CMappedFile(CMappedFile&& other) noexcept;
    CMappedFile& operator=(CMappedFile&& other) noexcept;

    // Méthodes principales
    bool Open(LPCTSTR lpszPathName) noexcept;
    void Close() noexcept;

    // Accesseurs
    const char* GetData() const noexcept { return m_pData; }
    uint64_t GetSize() const noexcept { return m_fileSize; }
    bool IsOpen() const noexcept { return m_pData != nullptr; }

private:
    HANDLE m_hFile;
    HANDLE m_hMapping;
    const char* m_pData;
    uint64_t m_fileSize;
};
