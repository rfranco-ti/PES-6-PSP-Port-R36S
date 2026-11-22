#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cctype>
#include <cerrno>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <vector>
#include <string>
#include "third_party/pspdecrypt/PrxDecrypter.h"

static constexpr size_t sector_size = 2048;
static constexpr size_t expected_size = 1914920;
static constexpr uint64_t expected_hash = 0xd9cd847af64f63b1ULL;

static uint32_t little32(const uint8_t *p) {
    return uint32_t(p[0]) | uint32_t(p[1]) << 8 | uint32_t(p[2]) << 16 | uint32_t(p[3]) << 24;
}

static uint64_t fingerprint(const uint8_t *data, size_t size) {
    uint64_t hash = 1469598103934665603ULL;
    for (size_t i = 0; i < size; ++i) hash = (hash ^ data[i]) * 1099511628211ULL;
    return hash;
}

static bool read_at(int fd, void *buffer, size_t size, uint64_t offset) {
    size_t done = 0;
    while (done < size) {
        ssize_t count = pread(fd, static_cast<uint8_t *>(buffer) + done, size - done, offset + done);
        if (count < 0 && errno == EINTR) continue;
        if (count <= 0) return false;
        done += size_t(count);
    }
    return true;
}

static bool find_entry(int fd, uint32_t &lba, uint32_t &size, const char *name) {
    uint8_t sector[sector_size];
    const size_t wanted = std::strlen(name);
    for (uint64_t offset = 0; offset < size; offset += sector_size) {
        size_t available = size_t((size - offset < sector_size) ? size - offset : sector_size);
        if (!read_at(fd, sector, available, uint64_t(lba) * sector_size + offset)) return false;
        for (size_t pos = 0; pos < available && sector[pos];) {
            const uint8_t *entry = sector + pos;
            size_t length = entry[0];
            if (length < 34 || length > available - pos) return false;
            size_t name_size = entry[32];
            if (name_size > length - 33) return false;
            const char *entry_name = reinterpret_cast<const char *>(entry + 33);
            if (name_size >= 2 && entry_name[name_size - 2] == ';') name_size -= 2;
            bool matches = name_size == wanted;
            for (size_t i = 0; matches && i < name_size; ++i)
                matches = std::toupper(static_cast<unsigned char>(entry_name[i])) == std::toupper(static_cast<unsigned char>(name[i]));
            if (matches) {
                lba = little32(entry + 2);
                size = little32(entry + 10);
                return true;
            }
            pos += length;
        }
    }
    return false;
}

static bool valid_elf(const uint8_t *bytes, size_t size) {
    return size == expected_size && std::memcmp(bytes, "\x7f" "ELF", 4) == 0 && fingerprint(bytes, size) == expected_hash;
}

static bool existing_valid(const char *path) {
    int fd = open(path, O_RDONLY);
    if (fd < 0) return false;
    struct stat st;
    std::vector<uint8_t> bytes(expected_size);
    bool valid = fstat(fd, &st) == 0 && st.st_size == static_cast<off_t>(expected_size)
        && read_at(fd, bytes.data(), bytes.size(), 0) && valid_elf(bytes.data(), bytes.size());
    close(fd);
    return valid;
}

static bool write_elf(const char *path, const uint8_t *bytes, size_t size) {
    std::string pattern = std::string(path) + ".tmp.XXXXXX";
    std::vector<char> temp(pattern.begin(), pattern.end());
    temp.push_back('\0');
    int fd = mkstemp(temp.data());
    if (fd < 0) return false;
    bool ok = true;
    size_t done = 0;
    while (done < size) {
        ssize_t written = write(fd, bytes + done, size - done);
        if (written < 0 && errno == EINTR) continue;
        if (written <= 0) { ok = false; break; }
        done += size_t(written);
    }
    if (close(fd) != 0) ok = false;
    if (ok && rename(temp.data(), path) != 0) ok = false;
    if (!ok) unlink(temp.data());
    return ok;
}

static int extract(const char *iso_path, const char *elf_path) {
    int fd = open(iso_path, O_RDONLY);
    if (fd < 0) { std::fprintf(stderr, "Nao foi possivel abrir a ISO: %s\n", iso_path); return 1; }
    uint8_t descriptor[sector_size];
    if (!read_at(fd, descriptor, sizeof descriptor, 16 * sector_size) || descriptor[0] != 1 || std::memcmp(descriptor + 1, "CD001", 5)) {
        std::fprintf(stderr, "ISO invalida ou ilegivel: %s\n", iso_path); close(fd); return 1;
    }
    uint32_t lba = little32(descriptor + 158), size = little32(descriptor + 166);
    const char *path[] = {"PSP_GAME", "SYSDIR", "EBOOT.BIN"};
    for (const char *part : path) {
        if (!find_entry(fd, lba, size, part)) {
            std::fprintf(stderr, "EBOOT.BIN nao encontrado na ISO: %s\n", iso_path); close(fd); return 1;
        }
    }
    if (size != 1915264) {
        std::fprintf(stderr, "Executavel incompativel: use PES 6 Europeu ULES00476 v1.03.\n"); close(fd); return 1;
    }
    std::vector<uint8_t> encrypted(size);
    if (!read_at(fd, encrypted.data(), size, uint64_t(lba) * sector_size)) {
        std::fprintf(stderr, "Falha ao ler EBOOT.BIN da ISO.\n"); close(fd); return 1;
    }
    close(fd);
    if (fingerprint(encrypted.data(), encrypted.size()) != 0x79d738ce48bb2178ULL) {
        std::fprintf(stderr, "ISO incompativel: use PES 6 Europeu ULES00476 v1.03.\n"); return 1;
    }
    if (existing_valid(elf_path)) { std::puts("EBOOT.elf validado."); return 0; }
    struct stat existing;
    if (lstat(elf_path, &existing) == 0 || errno != ENOENT) {
        std::fprintf(stderr, "EBOOT.elf existente e invalido ou inacessivel; mova o arquivo antes de continuar: %s\n", elf_path); return 1;
    }
    if (std::memcmp(encrypted.data(), "~PSP", 4) != 0) {
        std::fprintf(stderr, "EBOOT.BIN nao esta no formato PSP esperado.\n"); return 1;
    }
    std::vector<uint8_t> decrypted(size * 4);
    int count = pspDecryptPRX(encrypted.data(), decrypted.data(), size, nullptr, false);
    if (count <= 0 || static_cast<size_t>(count) > decrypted.size() || !valid_elf(decrypted.data(), size_t(count))) {
        std::fprintf(stderr, "Falha ao decodificar o EBOOT da versao esperada.\n"); return 1;
    }
    if (!write_elf(elf_path, decrypted.data(), size_t(count))) {
        std::fprintf(stderr, "Falha ao gravar EBOOT.elf em %s: %s\n", elf_path, std::strerror(errno)); return 1;
    }
    std::puts("EBOOT.elf extraido e validado.");
    return 0;
}

int main(int argc, char **argv) {
    if (argc != 3) { std::fprintf(stderr, "Uso: %s <jogo.iso> <EBOOT.elf>\n", argv[0]); return 2; }
    return extract(argv[1], argv[2]);
}
