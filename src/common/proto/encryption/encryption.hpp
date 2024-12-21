#ifndef BSIT_3_ENCRYPTION_HPP
#define BSIT_3_ENCRYPTION_HPP

#include <windows.h>
#include <wincrypt.h>

#include <unordered_map>
#include <memory>

#include "../../alias.hpp"

namespace proto::encryption {
class EncryptionManager {
public:
    EncryptionManager();
    ~EncryptionManager();

    void CreateAsymmetricKey();
    void CreateSymmetricKey(u32 cid);

    const u8 *ExportPublicKey(DWORD *size);
    const u8 *ExportSymmetricKey(u32 cid, DWORD *size, const u8 *pub_key_buf,
                                 DWORD pub_key_size);

    void ImportSymmetricKey(u32 cid, const u8 *buf, usize size);
    std::unique_ptr<const u8[]> Encrypt(u32 cid, const u8 *buf, usize size, DWORD *res_size);
    std::unique_ptr<const u8[]> Decrypt(u32 cid, const u8 *buf, usize size, DWORD *res_size);

    void PrintHash(HCRYPTKEY hKey) const;

private:
    std::unordered_map<u32, HCRYPTKEY> m_keys;
    HCRYPTPROV m_provider = 0;
};

void init();

inline EncryptionManager *g_instance = nullptr;
}  // namespace proto::encryption

#endif
