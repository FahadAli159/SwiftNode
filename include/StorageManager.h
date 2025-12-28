#pragma once
#include <string>
#include <vector>

class StorageManager {
public:
  StorageManager();
  ~StorageManager();

  // Splits file into encrypted chunks and saves them.
  // Returns a list of chunk IDs (hashes) representing the file.
  std::vector<std::string> storeFile(const std::string &filepath);

  // Reassembles a file from local chunks given a list of IDs.
  bool retrieveFile(const std::vector<std::string> &chunkIds,
                    const std::string &outputPath);

private:
  std::string calculateHash(const std::vector<unsigned char> &data);
  std::vector<unsigned char>
  encryptChunk(const std::vector<unsigned char> &data);
  std::vector<unsigned char>
  decryptChunk(const std::vector<unsigned char> &data);

  // In a real app, keys would be managed securely per user/file.
  // For MVP, we use a hardcoded key.
  unsigned char encryptionKey[32];
  unsigned char iv[16];
};
