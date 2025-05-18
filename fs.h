#ifndef FS_H
#define FS_H

#include <time.h>

#define MAX_FILENAME 20
#define MAX_FILES 128
#define DISK_SIZE (1024 * 1024) // 1MB
#define METADATA_SIZE 4096
#define BLOCK_SIZE 512
#define DATA_START (METADATA_SIZE)

typedef struct {
    char name[MAX_FILENAME];
    int size;
    int start_block;
    time_t created_at;
    int used; // 1 = aktif, 0 = silinmiş
} FileEntry;

void fs_format();
int fs_create(const char* filename);
int fs_delete(const char* filename);
int fs_write(const char* filename, const char* data, int size);
int fs_read(const char* filename, int offset, int size, char* buffer);
void fs_ls();
int fs_rename(const char* old_name, const char* new_name);
int fs_exists(const char* filename);
int fs_size(const char* filename);
int fs_append(const char* filename, const char* data, int size);
int fs_truncate(const char* filename, int new_size);
int fs_copy(const char* src_filename, const char* dest_filename);
int fs_mv(const char* old_name, const char* new_name); // Klasör desteği yoksa rename ile aynı
///
///
void fs_defragment();
void fs_check_integrity();

int fs_backup(const char* backup_filename);
int fs_restore(const char* backup_filename);

void fs_cat(const char* filename);
int fs_diff(const char* file1, const char* file2);

void fs_log(const char* message);

#endif
