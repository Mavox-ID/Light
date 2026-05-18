#pragma once
#include <shared/crc.h>

struct LppBundleHeader {
	uint32_t signature;
	uint32_t version;
	uint32_t fileCount;
	uint32_t _unused;
	uint64_t mapAddress;
};

struct LppBundleFile {
	uint64_t nameCRC64;
	uint64_t bytes;
	uint64_t offset;
};

static const void *LppFindFile(const void *data, size_t dataSize,
                                const char *key, size_t keyLen, size_t *outBytes) {
	if (!data || dataSize < sizeof(LppBundleHeader)) return nullptr;
	const LppBundleHeader *header = (const LppBundleHeader *) data;
	if (header->signature != 0x63BDAF45 || header->version != 1) return nullptr;
	size_t filesSize = (size_t) header->fileCount * sizeof(LppBundleFile);
	if (dataSize < sizeof(LppBundleHeader) + filesSize) return nullptr;
	const LppBundleFile *files = (const LppBundleFile *)(header + 1);
	uint64_t targetCRC = CalculateCRC64(key, keyLen, 0);
	for (uint32_t i = 0; i < header->fileCount; i++) {
		if (files[i].nameCRC64 == targetCRC) {
			if (files[i].offset + files[i].bytes > dataSize) return nullptr;
			if (outBytes) *outBytes = (size_t) files[i].bytes;
			return (const uint8_t *) data + files[i].offset;
		}
	}
	return nullptr;
}

static const void *LppFindIcon32(const void *data, size_t dataSize, size_t *outBytes) {
	return LppFindFile(data, dataSize, "$Icons/32", 9, outBytes);
}
