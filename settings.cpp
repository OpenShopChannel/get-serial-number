#include "settings.h"
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <algorithm>

static fstats stats ATTRIBUTE_ALIGN(32);

Settings::Settings(const u8* encrypted_data)
{
  for (u32 i = 0; i < ENCRYPTED_SIZE; i++) {
    m_decrypted.push_back(static_cast<char>(encrypted_data[i] ^ m_key));
    m_key = (m_key << 1) | (m_key >> 31);
  }

  m_decrypted.erase(std::remove(m_decrypted.begin(), m_decrypted.end(), '\x0d'), m_decrypted.end());
}

std::string Settings::GetValue(std::string_view key) const
{
  // This code was taken from Dolphin Emulator's SettingsHandler.cpp.
  constexpr char delim[] = "\n";
  std::string toFind = std::string(delim).append(key).append("=");
  size_t found = m_decrypted.find(toFind);

  if (found != std::string_view::npos)
  {
    size_t delimFound = m_decrypted.find(delim, found + toFind.length());
    if (delimFound == std::string_view::npos)
      delimFound = m_decrypted.length() - 1;
    return m_decrypted.substr(found + toFind.length(), delimFound - (found + toFind.length()));
  }
  else
  {
    toFind = std::string(key).append("=");
    found = m_decrypted.find(toFind);
    if (found == 0)
    {
      size_t delimFound = m_decrypted.find(delim, found + toFind.length());
      if (delimFound == std::string_view::npos)
        delimFound = m_decrypted.length() - 1;
      return m_decrypted.substr(found + toFind.length(), delimFound - (found + toFind.length()));
    }
  }

  return {};
}

u8* Settings::GetEncryptedSettings()
{
  s32 fd = ISFS_Open(SETTINGS_PATH, ISFS_OPEN_READ);
  if (fd < 0) {
    printf("ISFS_GetFile: unable to open file (error %d)\n", fd);
    return nullptr;
  }

  void *buf = nullptr;
  std::memset(&stats, 0, sizeof(fstats));

  s32 ret = ISFS_GetFileStats(fd, &stats);
  if (ret >= 0) {
    u32 length = stats.file_length;

    // We must align our length by 32.
    // memalign itself is dreadfully broken for unknown reasons.
    u32 aligned_length = length;
    u32 remainder = aligned_length % 32;
    if (remainder != 0) {
      aligned_length += 32 - remainder;
    }

    buf = aligned_alloc(32, aligned_length);

    if (buf != nullptr) {
      s32 tmp_size = ISFS_Read(fd, buf, length);

      if (tmp_size != static_cast<s32>(length)) {
        // If positive, the file could not be fully read.
        // If negative, it is most likely an underlying /dev/fs
        // error.
        if (tmp_size >= 0) {
          printf("ISFS_GetFile: only able to read %d out of "
                 "%d bytes!\n",
                 tmp_size, length);
          ISFS_Close(fd);
          return nullptr;
        } else if (tmp_size == ISFS_ENOENT) {
          // This should never ever happen as this is an essential system file.
        } else {
          printf("ISFS_GetFile: ISFS_Open failed! (error %d)\n",
                 tmp_size);
          ISFS_Close(fd);
          return nullptr;
        }

        free(buf);
      }
    } else {
      printf("ISFS_GetFile: failed to allocate buffer!\n");
      ISFS_Close(fd);
      return nullptr;
    }
  } else {
    printf("ISFS_GetFile: unable to retrieve file stats (error %d)\n", ret);
    ISFS_Close(fd);
    return nullptr;
  }

  ISFS_Close(fd);
  return reinterpret_cast<u8*>(buf);
}
