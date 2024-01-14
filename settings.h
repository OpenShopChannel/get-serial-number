#pragma once

#include <ogcsys.h>
#include <string>

class Settings
{
public:
    Settings(const u8* encrypted_data);
    std::string GetValue(std::string_view key) const;
    static u8* GetEncryptedSettings();

private:
    static constexpr char SETTINGS_PATH[] = "/title/00000001/00000002/data/setting.txt";
    static constexpr u32 ENCRYPTED_SIZE = 256;
    static constexpr s32 ISFS_ENOENT = -105;

    std::string m_decrypted{};
    u32 m_key = 0x73B5DBFA;
};
