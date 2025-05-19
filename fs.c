#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include "fs.h"

#define DISK_FILE "disk.sim"

static FileEntry file_table[MAX_FILES];

static int load_metadata() {
    int fd = open(DISK_FILE, O_RDWR);
    if (fd < 0) {
        perror("[DEBUG] load_metadata: Disk dosyası açılamadı");
        return -1;
    }
    
    off_t seek_result = lseek(fd, 0, SEEK_SET);
    if (seek_result == -1) {
        perror("[DEBUG] load_metadata: lseek hatası");
        close(fd);
        return -1;
    }
    
    ssize_t bytes_read = read(fd, file_table, sizeof(file_table));
    if (bytes_read != sizeof(file_table)) {
        printf("[DEBUG] load_metadata: Metadata tam okunamadı. İstenen: %ld, Okunan: %ld\n", 
               (long)sizeof(file_table), (long)bytes_read);
        close(fd);
        return -1;
    }
    
    close(fd);
    printf("[DEBUG] load_metadata: Metadata başarıyla yüklendi (%ld byte)\n", (long)bytes_read);
    return 0;
}

static int save_metadata() {
    int fd = open(DISK_FILE, O_RDWR);
    if (fd < 0) {
        perror("[DEBUG] save_metadata: Disk dosyası açılamadı");
        return -1;
    }
    
    off_t seek_result = lseek(fd, 0, SEEK_SET);
    if (seek_result == -1) {
        perror("[DEBUG] save_metadata: lseek hatası");
        close(fd);
        return -1;
    }
    
    ssize_t bytes_written = write(fd, file_table, sizeof(file_table));
    if (bytes_written != sizeof(file_table)) {
        printf("[DEBUG] save_metadata: Metadata tam yazılamadı. İstenen: %ld, Yazılan: %ld\n", 
               (long)sizeof(file_table), (long)bytes_written);
        close(fd);
        return -1;
    }
    
    close(fd);
    printf("[DEBUG] save_metadata: Metadata başarıyla kaydedildi (%ld byte)\n", (long)bytes_written);
    return 0;
}

void fs_format() {
    int fd = open(DISK_FILE, O_RDWR | O_CREAT | O_TRUNC, 0666);
    ftruncate(fd, DISK_SIZE);
    memset(file_table, 0, sizeof(file_table));
    write(fd, file_table, sizeof(file_table));
    close(fd);
}
//ddosya oluştur yeni
int fs_create(const char* filename) {
    load_metadata();
    printf("[DEBUG] fs_create: Dosya adı: %s\n", filename);
    
    for (int i = 0; i < MAX_FILES; i++) {
        if (file_table[i].used && strcmp(file_table[i].name, filename) == 0) {
            printf("[DEBUG] Dosya zaten var: i=%d\n", i);
            return -1; // Zaten var
        }
        
        if (!file_table[i].used) {
            strncpy(file_table[i].name, filename, MAX_FILENAME);
            file_table[i].size = 0;
            file_table[i].start_block = i; // Sadece blok numarası
            file_table[i].created_at = time(NULL);
            file_table[i].used = 1;
            
            printf("[DEBUG] Dosya oluşturuldu: i=%d, başlangıç blok=%d\n", 
                   i, file_table[i].start_block);
            
            save_metadata();
            return 0;
        }
    }
    
    printf("[DEBUG] Yer yok: Maksimum dosya sayısına ulaşıldı\n");
    return -1; // Yer yok
}

int fs_delete(const char* filename) {
    load_metadata();
    for (int i = 0; i < MAX_FILES; i++) {
        if (file_table[i].used && strcmp(file_table[i].name, filename) == 0) {
            file_table[i].used = 0;
            save_metadata();
            return 0;
        }
    }
    return -1;
}
// Dosyaya veri yaz yeni
int fs_write(const char* filename, const char* data, int size) {
    load_metadata();
    printf("[DEBUG] fs_write: Dosya: %s, Veri boyutu: %d\n", filename, size);
    
    // Kaydedilecek veriyi hex olarak göster
    printf("[DEBUG] Yazılacak veri (hex): ");
    for (int i = 0; i < size; i++) {
        printf("%02X ", (unsigned char)data[i]);
    }
    printf("\n");
    
    int fd = open(DISK_FILE, O_RDWR);
    if (fd < 0) {
        perror("Disk dosyası açılamadı");
        return -1;
    }

    for (int i = 0; i < MAX_FILES; i++) {
        if (file_table[i].used && strcmp(file_table[i].name, filename) == 0) {
            // Dosya başlangıç pozisyonunu hesapla
            int file_position = DATA_START + file_table[i].start_block * BLOCK_SIZE;
            printf("[DEBUG] Dosya bulundu: i=%d, başlangıç blok=%d, hesaplanan konum=%d\n", 
                   i, file_table[i].start_block, file_position);
            
            off_t seek_result = lseek(fd, file_position, SEEK_SET);
            if (seek_result == -1) {
                perror("lseek hatası");
                close(fd);
                return -1;
            }
            printf("[DEBUG] lseek başarılı: konum=%ld\n", (long)seek_result);

            // Yazma işlemi sonrasında veriyi doğrula
            int written = write(fd, data, size);
            if (written != size) {
                printf("[DEBUG] Yazma hatası: istenen=%d, yazılan=%d\n", size, written);
                perror("write hatası");
                close(fd);
                return -1;
            }
            printf("[DEBUG] Yazma başarılı: %d byte yazıldı\n", written);
            
            // Yazılan veriyi kontrol etmek için tekrar oku
            char* verify_buffer = malloc(size);
            if (verify_buffer != NULL) {
                lseek(fd, file_position, SEEK_SET);
                int read_check = read(fd, verify_buffer, size);
                printf("[DEBUG] Yazma doğrulama: %d byte okundu\n", read_check);
                
                printf("[DEBUG] Doğrulama verisi (hex): ");
                for (int j = 0; j < read_check; j++) {
                    printf("%02X ", (unsigned char)verify_buffer[j]);
                }
                printf("\n");
                
                free(verify_buffer);
            }

            file_table[i].size = size;
            save_metadata();
            close(fd);
            return written;
        }
    }

    printf("[DEBUG] Dosya bulunamadı: %s\n", filename);
    close(fd);
    return -1;
}
// Dosyadan veri oku yeni
int fs_read(const char* filename, int offset, int size, char* buffer) {
    load_metadata();
    printf("[DEBUG] fs_read: Dosya: %s, Offset: %d, Boyut: %d\n", filename, offset, size);
    
    int fd = open(DISK_FILE, O_RDONLY);
    if (fd < 0) {
        perror("Disk dosyası açılamadı");
        return -1;
    }

    for (int i = 0; i < MAX_FILES; i++) {
        if (file_table[i].used && strcmp(file_table[i].name, filename) == 0) {
            printf("[DEBUG] Dosya bulundu: i=%d, boyut=%d, başlangıç blok=%d\n", 
                   i, file_table[i].size, file_table[i].start_block);
            
            if (offset < 0 || size < 0 || offset + size > file_table[i].size) {
                printf("[DEBUG] Hata: Geçersiz offset veya boyut. offset=%d, size=%d, dosya boyutu=%d\n", 
                       offset, size, file_table[i].size);
                close(fd);
                return -1;
            }

            // İlk olarak buffer'ı benzersiz bir değerle dolduralım
            memset(buffer, '_', size);
            
            // Dosya başlangıç pozisyonunu ve okuma konumunu hesapla
            int file_position = DATA_START + file_table[i].start_block * BLOCK_SIZE + offset;
            printf("[DEBUG] Dosya konumu hesaplandı: DATA_START=%d, BLOCK_SIZE=%d, file_position=%d\n", 
                   DATA_START, BLOCK_SIZE, file_position);
            
            // Dosya konumuna git
            off_t seek_result = lseek(fd, file_position, SEEK_SET);
            if (seek_result == -1) {
                perror("lseek hatası");
                close(fd);
                return -1;
            }
            printf("[DEBUG] lseek başarılı: konum=%ld\n", (long)seek_result);

            // Veriyi oku
            int bytes_read = read(fd, buffer, size);
            if (bytes_read != size) {
                printf("[DEBUG] Okuma hatası: istenen=%d, okunan=%d\n", size, bytes_read);
                perror("read hatası");
                close(fd);
                return -1;
            }
            
            printf("[DEBUG] Okuma başarılı: %d byte okundu\n", bytes_read);
            
            // Okunan veriyi hex olarak göster
            printf("[DEBUG] Okunan veri (hex): ");
            for (int j = 0; j < bytes_read; j++) {
                printf("%02X ", (unsigned char)buffer[j]);
            }
            printf("\n");

            close(fd);
            return 0;
        }
    }

    printf("[DEBUG] Dosya bulunamadı: %s\n", filename);
    close(fd);
    return -1;
}

void fs_ls() {
    load_metadata();
    printf("\n=== Dosya Listesi ===\n");
    printf("%-20s %-10s %-20s\n", "Dosya Adı", "Boyut", "Oluşturma Zamanı");
    printf("-----------------------------------------------------\n");
    
    int sayac = 0;
    for (int i = 0; i < MAX_FILES; i++) {
        if (file_table[i].used) {
            char zaman[30];
            struct tm* timeinfo = localtime(&file_table[i].created_at);
            strftime(zaman, sizeof(zaman), "%Y-%m-%d %H:%M:%S", timeinfo);
            
            // Dosya adını hex olarak da göster (özellikle boşluk vb. karakterler için)
            printf("%-20s %-10d %-20s", file_table[i].name, file_table[i].size, zaman);
            
            printf(" [hex: ");
            for (int j = 0; j < strlen(file_table[i].name); j++) {
                printf("%02X ", (unsigned char)file_table[i].name[j]);
            }
            printf("]\n");
            
            sayac++;
        }
    }
    
    if (sayac == 0) {
        printf("Hiç dosya bulunamadı.\n");
    } else {
        printf("\nToplam %d dosya listelendi.\n", sayac);
    }
}

int fs_rename(const char* old_name, const char* new_name) {
    load_metadata();
    for (int i = 0; i < MAX_FILES; i++) {
        if (file_table[i].used && strcmp(file_table[i].name, old_name) == 0) {
            strncpy(file_table[i].name, new_name, MAX_FILENAME);
            save_metadata();
            return 0;
        }
    }
    return -1;
}

int fs_exists(const char* filename) {
    load_metadata();
    for (int i = 0; i < MAX_FILES; i++) {
        if (file_table[i].used && strcmp(file_table[i].name, filename) == 0)
            return 1;
    }
    return 0;
}

int fs_size(const char* filename) {
    load_metadata();
    for (int i = 0; i < MAX_FILES; i++) {
        if (file_table[i].used && strcmp(file_table[i].name, filename) == 0)
            return file_table[i].size;
    }
    return -1;
}

int fs_append(const char* filename, const char* data, int size) {
    load_metadata();
    int fd = open(DISK_FILE, O_RDWR);
    for (int i = 0; i < MAX_FILES; i++) {
        if (file_table[i].used && strcmp(file_table[i].name, filename) == 0) {
            lseek(fd, file_table[i].start_block + file_table[i].size, SEEK_SET);
            write(fd, data, size);
            file_table[i].size += size;
            save_metadata();
            close(fd);
            return 0;
        }
    }
    close(fd);
    return -1;
}

int fs_truncate(const char* filename, int new_size) {
    load_metadata();
    for (int i = 0; i < MAX_FILES; i++) {
        if (file_table[i].used && strcmp(file_table[i].name, filename) == 0) {
            if (new_size > file_table[i].size)
                return -1;
            file_table[i].size = new_size;
            save_metadata();
            return 0;
        }
    }
    return -1;
}

int fs_copy(const char* src_filename, const char* dest_filename) {
    char buffer[1024];
    int size = fs_size(src_filename);
    if (size < 0) return -1;

    char* data = malloc(size);
    if (!data) return -1;

    if (fs_read(src_filename, 0, size, data) != size) {
        free(data);
        return -1;
    }

    if (fs_create(dest_filename) != 0) {
        free(data);
        return -1;
    }

    int result = fs_write(dest_filename, data, size);
    free(data);
    return result;
}

int fs_mv(const char* old_name, const char* new_name) {
    return fs_rename(old_name, new_name);
}

void fs_defragment() {
    int fd = open("disk.sim", O_RDWR);
    if (fd < 0) return;

    FileEntry entries[MAX_FILES];
    read(fd, entries, sizeof(entries));

    int data_ptr = DATA_START;
    for (int i = 0; i < MAX_FILES; i++) {
        if (!entries[i].used || entries[i].size == 0) continue;

        if (entries[i].start_block != data_ptr) {
            char* temp = malloc(entries[i].size);
            lseek(fd, entries[i].start_block, SEEK_SET);
            read(fd, temp, entries[i].size);

            lseek(fd, data_ptr, SEEK_SET);
            write(fd, temp, entries[i].size);
            entries[i].start_block = data_ptr;

            free(temp);
        }

        data_ptr += entries[i].size;
    }

    lseek(fd, 0, SEEK_SET);
    write(fd, entries, sizeof(entries));
    close(fd);
}

void fs_check_integrity() {
    int fd = open("disk.sim", O_RDONLY);
    if (fd < 0) return;

    FileEntry entries[MAX_FILES];
    read(fd, entries, sizeof(entries));

    for (int i = 0; i < MAX_FILES; i++) {
        if (!entries[i].used) continue;

        if (entries[i].start_block < DATA_START || 
            entries[i].start_block + entries[i].size > DISK_SIZE) {
            printf("Integrity error in file: %s\n", entries[i].name);
        }
    }

    close(fd);
}

int fs_backup(const char* backup_filename) {
    int src = open("disk.sim", O_RDONLY);
    int dst = open(backup_filename, O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if (src < 0 || dst < 0) return -1;

    char buffer[1024];
    ssize_t bytes;
    while ((bytes = read(src, buffer, sizeof(buffer))) > 0) {
        write(dst, buffer, bytes);
    }

    close(src);
    close(dst);
    return 0;
}

int fs_restore(const char* backup_filename) {
    int src = open(backup_filename, O_RDONLY);
    int dst = open("disk.sim", O_WRONLY | O_TRUNC);
    if (src < 0 || dst < 0) return -1;

    char buffer[1024];
    ssize_t bytes;
    while ((bytes = read(src, buffer, sizeof(buffer))) > 0) {
        write(dst, buffer, bytes);
    }

    close(src);
    close(dst);
    return 0;
}

void fs_cat(const char* filename) {
    int size = fs_size(filename);
    if (size <= 0) return;

    char* data = malloc(size + 1);
    if (!data) return;

    fs_read(filename, 0, size, data);
    data[size] = '\0';
    printf("%s\n", data);
    free(data);
}

int fs_diff(const char* file1, const char* file2) {
    int size1 = fs_size(file1);
    int size2 = fs_size(file2);
    if (size1 != size2) return 1;

    char* buf1 = malloc(size1);
    char* buf2 = malloc(size2);
    fs_read(file1, 0, size1, buf1);
    fs_read(file2, 0, size2, buf2);

    int result = memcmp(buf1, buf2, size1);
    free(buf1);
    free(buf2);
    return result;
}

#include <time.h>

void fs_log(const char* message) {
    FILE* logf = fopen("fs.log", "a");
    if (!logf) return;

    time_t now = time(NULL);
    fprintf(logf, "[%s] %s\n", ctime(&now), message);
    fclose(logf);
}
