#include "StorageManager.h"
#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/sha.h>
#include <sstream>
#include <sys/stat.h>

const int CHUNK_SIZE = 1024 * 1024; // 1 MB

StorageManager::StorageManager() {
  // Generate static key/IV for testing (In prod, these come from User Auth)
  // NOTE: This is just for MVP demonstration.
  memset(encryptionKey, 0x42, 32);
  memset(iv, 0x00, 16);

  // Create storage directory
  mkdir("storage", 0777);
}

StorageManager::~StorageManager() {}

std::string
StorageManager::calculateHash(const std::vector<unsigned char> &data) {
  unsigned char hash[SHA256_DIGEST_LENGTH];
  SHA256(data.data(), data.size(), hash);

  std::stringstream ss;
  for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
    ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
  }
  return ss.str();
}

std::vector<unsigned char>
StorageManager::encryptChunk(const std::vector<unsigned char> &data) {
  EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
  std::vector<unsigned char> ciphertext(data.size() + EVP_MAX_BLOCK_LENGTH);
  int len;
  int ciphertext_len;

  EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, encryptionKey, iv);
  EVP_EncryptUpdate(ctx, ciphertext.data(), &len, data.data(), data.size());
  ciphertext_len = len;

  EVP_EncryptFinal_ex(ctx, ciphertext.data() + len, &len);
  ciphertext_len += len;

  EVP_CIPHER_CTX_free(ctx);
  ciphertext.resize(ciphertext_len);
  return ciphertext;
}

std::vector<unsigned char>
StorageManager::decryptChunk(const std::vector<unsigned char> &data) {
  EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
  std::vector<unsigned char> plaintext(data.size() + EVP_MAX_BLOCK_LENGTH);
  int len;
  int plaintext_len;

  EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, encryptionKey, iv);
  EVP_DecryptUpdate(ctx, plaintext.data(), &len, data.data(), data.size());
  plaintext_len = len;

  EVP_DecryptFinal_ex(ctx, plaintext.data() + len, &len);
  plaintext_len += len;

  EVP_CIPHER_CTX_free(ctx);
  plaintext.resize(plaintext_len);
  return plaintext;
}

std::vector<std::string>
StorageManager::storeFile(const std::string &filepath) {
  std::ifstream file(filepath, std::ios::binary);
  std::vector<std::string> chunkIds;

  if (!file.is_open()) {
    std::cerr << "Failed to open file: " << filepath << std::endl;
    return chunkIds;
  }

  std::vector<unsigned char> buffer(CHUNK_SIZE);
  while (file.read(reinterpret_cast<char *>(buffer.data()), CHUNK_SIZE) ||
         file.gcount() > 0) {
    size_t bytesRead = file.gcount();
    std::vector<unsigned char> chunkData(buffer.begin(),
                                         buffer.begin() + bytesRead);

    // 1. Encrypt
    auto encryptedData = encryptChunk(chunkData);

    // 2. Hash (ID)
    std::string chunkId = calculateHash(encryptedData);

    // 3. Save to Disk
    std::string outPath = "storage/" + chunkId;
    std::ofstream outfile(outPath, std::ios::binary);
    outfile.write(reinterpret_cast<const char *>(encryptedData.data()),
                  encryptedData.size());
    outfile.close();

    chunkIds.push_back(chunkId);
    std::cout << "Stored chunk: " << chunkId << " (" << bytesRead << " bytes)"
              << std::endl;

    buffer.assign(CHUNK_SIZE, 0); // Clear buffer
  }
  return chunkIds;
}

bool StorageManager::retrieveFile(const std::vector<std::string> &chunkIds,
                                  const std::string &outputPath) {
  std::ofstream outfile(outputPath, std::ios::binary);
  if (!outfile.is_open())
    return false;

  for (const auto &id : chunkIds) {
    std::ifstream infile("storage/" + id, std::ios::binary);
    if (!infile.is_open()) {
      std::cerr << "Missing chunk: " << id << std::endl;
      return false;
    }

    std::vector<unsigned char> encryptedData(
        (std::istreambuf_iterator<char>(infile)),
        std::istreambuf_iterator<char>());

    // Decrypt
    auto decryptedData = decryptChunk(encryptedData);

    // Append to file
    outfile.write(reinterpret_cast<const char *>(decryptedData.data()),
                  decryptedData.size());
  }
  return true;
}
