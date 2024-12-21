#include "encryption.hpp"
#include "../../logging.hpp"

namespace proto::encryption {
EncryptionManager::EncryptionManager() {
    CryptAcquireContext(&m_provider, nullptr, nullptr, PROV_RSA_AES,
                        CRYPT_VERIFYCONTEXT);
}
EncryptionManager::~EncryptionManager() {
    CryptReleaseContext(m_provider, 0);
    for (auto &[_, key] : m_keys) {
        CryptDestroyKey(key);
    }
}
void EncryptionManager::CreateAsymmetricKey() {
    HCRYPTKEY key;
    CryptGenKey(m_provider, AT_KEYEXCHANGE, (2048 << 16) | CRYPT_EXPORTABLE,
                &key);
    m_keys[0] = key;
}
void EncryptionManager::CreateSymmetricKey(u32 cid) {
    HCRYPTKEY key;
    if (m_keys.contains(cid) && m_keys[cid]) {
        CryptDestroyKey(m_keys[cid]);
    }

    CryptGenKey(m_provider, CALG_AES_256, CRYPT_EXPORTABLE, &key);
    m_keys[cid] = key;
}
const u8 *EncryptionManager::ExportPublicKey(DWORD *size) {
    CryptExportKey(m_keys[0], 0, PUBLICKEYBLOB, 0, nullptr, size);
    auto buf = new u8[*size];
    CryptExportKey(m_keys[0], 0, PUBLICKEYBLOB, 0, buf,
                   reinterpret_cast<DWORD *>(size));

    return buf;
}
void EncryptionManager::ImportSymmetricKey(u32 cid, const u8 *buf, usize size) {
    HCRYPTKEY sym_key;
    CryptImportKey(m_provider, buf, size, m_keys[0], CRYPT_EXPORTABLE,
                   &sym_key);
    m_keys[cid] = sym_key;
}
const u8 *EncryptionManager::ExportSymmetricKey(u32 cid, DWORD *size,
                                                const u8 *pub_key_buf,
                                                DWORD pub_key_size) {
    HCRYPTKEY pub_key;
    CryptImportKey(m_provider, pub_key_buf, pub_key_size, 0, 0, &pub_key);
    CryptExportKey(m_keys[cid], pub_key, SIMPLEBLOB, 0, nullptr,
                   reinterpret_cast<DWORD *>(size));
    auto buf = new u8[*size];
    CryptExportKey(m_keys[cid], pub_key, SIMPLEBLOB, 0, buf,
                   reinterpret_cast<DWORD *>(size));

    CryptDestroyKey(pub_key);

    return buf;
}

std::unique_ptr<const u8[]> EncryptionManager::Encrypt(u32 cid, const u8 *buf,
                                                       usize size,
                                                       DWORD *res_size) {
    INFO("Encrypt called for key id %d.", cid);
    PrintHash(m_keys[cid]);
    usize content_size = size;
    usize encrypted_size = content_size + 16;
    auto encrypted_buf = std::make_unique<u8[]>(encrypted_size);
    std::memset(encrypted_buf.get(), 0, encrypted_size);
    std::memcpy(encrypted_buf.get(), buf, size);
    *res_size = content_size;
    CryptEncrypt(m_keys[cid], 0, true, 0, encrypted_buf.get(), res_size,
                 encrypted_size);
    auto res = std::make_unique<u8[]>(*res_size);
    std::memcpy(res.get(), encrypted_buf.get(), *res_size);
    return std::move(res);
}

std::unique_ptr<const u8[]> EncryptionManager::Decrypt(u32 cid, const u8 *buf,
                                                       usize size,
                                                       DWORD *res_size) {
    INFO("Decrypt called for key id %d.", cid);
    PrintHash(m_keys[cid]);
    usize content_size = size;
    usize decrypted_size = content_size;
    auto decrypted_buf = std::make_unique<u8[]>(decrypted_size);
    std::memset(decrypted_buf.get(), 0, decrypted_size);
    std::memcpy(decrypted_buf.get(), buf, size);
    *res_size = decrypted_size;
    CryptDecrypt(m_keys[cid], 0, true, 0, decrypted_buf.get(), res_size);
    auto res = std::make_unique<u8[]>(*res_size);
    std::memcpy(res.get(), decrypted_buf.get(), *res_size);
    return std::move(res);
}

void EncryptionManager::PrintHash(HCRYPTKEY hKey) const {
#ifndef NDEBUG
    DWORD blobLen = 0;

    if (!CryptExportKey(hKey, 0, PLAINTEXTKEYBLOB, 0, NULL, &blobLen)) {
        printf("Error in CryptExportKey: %d\n", GetLastError());
        return;
    }

    BYTE *keyBlob = (BYTE *)malloc(blobLen);

    if (!CryptExportKey(hKey, 0, PLAINTEXTKEYBLOB, 0, keyBlob, &blobLen)) {
        printf("Error in CryptExportKey: %d\n", GetLastError());
        free(keyBlob);
        return;
    }

    HCRYPTHASH hHash;
    if (!CryptCreateHash(m_provider, CALG_SHA_256, 0, 0, &hHash)) {
        printf("Error in CryptCreateHash: %d\n", GetLastError());
        free(keyBlob);
        return;
    }

    if (!CryptHashData(hHash, keyBlob, blobLen, 0)) {
        printf("Error in CryptHashData: %d\n", GetLastError());
        CryptDestroyHash(hHash);
        free(keyBlob);
        return;
    }

    DWORD hashLen = 0;
    DWORD hashLenSize = sizeof(DWORD);
    if (!CryptGetHashParam(hHash, HP_HASHSIZE, (BYTE *)&hashLen, &hashLenSize,
                           0)) {
        printf("Error in CryptGetHashParam: %d\n", GetLastError());
        CryptDestroyHash(hHash);
        free(keyBlob);
        return;
    }

    BYTE *hashValue = (BYTE *)malloc(hashLen);

    if (!CryptGetHashParam(hHash, HP_HASHVAL, hashValue, &hashLen, 0)) {
        printf("Error in CryptGetHashParam: %d\n", GetLastError());
        CryptDestroyHash(hHash);
        free(keyBlob);
        free(hashValue);
        return;
    }

    printf("Key Hash: ");
    for (DWORD i = 0; i < hashLen; i++) {
        printf("%02X", hashValue[i]);
    }
    printf("\n");

    CryptDestroyHash(hHash);
    free(keyBlob);
    free(hashValue);
#endif
}

void init() {
    if (!g_instance) {
        g_instance = new EncryptionManager();
    }
}
}  // namespace proto::encryption
