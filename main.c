#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fs.h"

#define MAX_FILENAME 32
#define MAX_DATA 1024

void show_menu() {
    printf("\n=== SimpleFS Menü ===\n");
    printf("1. Dosya oluştur\n");
    printf("2. Dosya sil\n");
    printf("3. Dosyaya veri yaz\n");
    printf("4. Dosyadan veri oku\n");
    printf("5. Dosyaları listele\n");
    printf("6. Format at\n");
    printf("7. Dosya yeniden adlandır\n");
    printf("8. Dosya mevcut mu?\n");
    printf("9. Dosya boyutu\n");
    printf("10. Dosyaya veri ekle\n");
    printf("11. Dosya boyutunu küçült\n");
    printf("12. Dosya kopyala\n");
    printf("13. Dosya taşı (rename gibi)\n");
    printf("14. Diski birleştir (defragment)\n");
    printf("15. Disk bütünlüğünü kontrol et\n");
    printf("16. Disk yedeği al\n");
    printf("17. Disk yedeğini geri yükle\n");
    printf("18. Dosya içeriğini göster (cat)\n");
    printf("19. Dosyaları karşılaştır (diff)\n");
    printf("0. Çıkış\n");
    printf("======================\n");
}

int main() {
    int choice;
    char filename[MAX_FILENAME], newname[MAX_FILENAME];
    char data[MAX_DATA];
    int offset, size;
    
    while (1) {
        show_menu();
        printf("Seçiminiz: ");
        scanf("%d", &choice);
        getchar();  // Enter karakterini tüket

        switch (choice) {
            case 1:
                printf("Dosya adı: ");
                fgets(filename, MAX_FILENAME, stdin); strtok(filename, "\n");
                if (fs_create(filename) == 0) printf("Dosya oluşturuldu.\n");
                else printf("Hata: Dosya oluşturulamadı.\n");
                break;

            case 2:
                printf("Silinecek dosya adı: ");
                fgets(filename, MAX_FILENAME, stdin); strtok(filename, "\n");
                if (fs_delete(filename) == 0) printf("Dosya silindi.\n");
                else printf("Hata: Silinemedi.\n");
                break;

            case 3:
                printf("Dosya adı: ");
                fgets(filename, MAX_FILENAME, stdin); strtok(filename, "\n");
                printf("Veri girin: ");
                fgets(data, MAX_DATA, stdin); strtok(data, "\n");
                size = strlen(data);
                if (fs_write(filename, data, size) == size)
                    printf("Yazma başarılı.\n");
                else
                    printf("Hata: Yazılamadı.\n");
                break;

            case 4:
                printf("Dosya adı: ");
                fgets(filename, MAX_FILENAME, stdin); strtok(filename, "\n");
                printf("Offset: "); scanf("%d", &offset);
                printf("Boyut: "); scanf("%d", &size); getchar();
                char* buffer = malloc(size + 1);
                if (fs_read(filename, offset, size, buffer) > 0) {
                    buffer[size] = '\0';
                    printf("Okunan veri: %s\n", buffer);
                } else {
                    printf("Hata: Okuma başarısız.\n");
                }
                free(buffer);
                break;

            case 5:
                fs_ls();
                break;

            case 6:
                fs_format();
                printf("Disk formatlandı.\n");
                break;

            case 7:
                printf("Eski ad: ");
                fgets(filename, MAX_FILENAME, stdin); strtok(filename, "\n");
                printf("Yeni ad: ");
                fgets(newname, MAX_FILENAME, stdin); strtok(newname, "\n");
                if (fs_rename(filename, newname) == 0)
                    printf("Ad değiştirildi.\n");
                else
                    printf("Hata: Değiştirilemedi.\n");
                break;

            case 8:
                printf("Dosya adı: ");
                fgets(filename, MAX_FILENAME, stdin); strtok(filename, "\n");
                printf("Durum: %s\n", fs_exists(filename) ? "Var" : "Yok");
                break;

            case 9:
                printf("Dosya adı: ");
                fgets(filename, MAX_FILENAME, stdin); strtok(filename, "\n");
                printf("Boyut: %d\n", fs_size(filename));
                break;

            case 10:
                printf("Dosya adı: ");
                fgets(filename, MAX_FILENAME, stdin); strtok(filename, "\n");
                printf("Eklemek istediğiniz veri: ");
                fgets(data, MAX_DATA, stdin); strtok(data, "\n");
                size = strlen(data);
                if (fs_append(filename, data, size) == size)
                    printf("Ekleme başarılı.\n");
                else
                    printf("Hata: Ekleme başarısız.\n");
                break;

            case 11:
                printf("Dosya adı: ");
                fgets(filename, MAX_FILENAME, stdin); strtok(filename, "\n");
                printf("Yeni boyut: ");
                scanf("%d", &size); getchar();
                if (fs_truncate(filename, size) == 0)
                    printf("Dosya kesildi.\n");
                else
                    printf("Hata: Truncate başarısız.\n");
                break;

            case 12:
                printf("Kaynak dosya: ");
                fgets(filename, MAX_FILENAME, stdin); strtok(filename, "\n");
                printf("Hedef dosya: ");
                fgets(newname, MAX_FILENAME, stdin); strtok(newname, "\n");
                if (fs_copy(filename, newname) == 0)
                    printf("Kopyalama başarılı.\n");
                else
                    printf("Hata: Kopyalanamadı.\n");
                break;

            case 13:
                printf("Taşınacak dosya (eski ad): ");
                fgets(filename, MAX_FILENAME, stdin); strtok(filename, "\n");
                printf("Yeni ad: ");
                fgets(newname, MAX_FILENAME, stdin); strtok(newname, "\n");
                if (fs_mv(filename, newname) == 0)
                    printf("Taşıma başarılı.\n");
                else
                    printf("Hata: Taşıma başarısız.\n");
                break;

            case 14:
                fs_defragment();
                printf("Disk birleştirildi.\n");
                break;

            case 15:
                fs_check_integrity();
                break;

            case 16:
                printf("Yedek dosya adı: ");
                fgets(filename, MAX_FILENAME, stdin); strtok(filename, "\n");
                if (fs_backup(filename) == 0)
                    printf("Yedekleme tamamlandı.\n");
                else
                    printf("Hata: Yedek alınamadı.\n");
                break;

            case 17:
                printf("Geri yüklenecek yedek adı: ");
                fgets(filename, MAX_FILENAME, stdin); strtok(filename, "\n");
                if (fs_restore(filename) == 0)
                    printf("Geri yükleme başarılı.\n");
                else
                    printf("Hata: Geri yüklenemedi.\n");
                break;

            case 18:
                printf("Dosya adı: ");
                fgets(filename, MAX_FILENAME, stdin); strtok(filename, "\n");
                fs_cat(filename);
                break;

            case 19:
                printf("1. dosya adı: ");
                fgets(filename, MAX_FILENAME, stdin); strtok(filename, "\n");
                printf("2. dosya adı: ");
                fgets(newname, MAX_FILENAME, stdin); strtok(newname, "\n");
                if (fs_diff(filename, newname) == 0)
                    printf("Dosyalar aynı.\n");
                else
                    printf("Dosyalar farklı.\n");
                break;

            case 0:
                printf("Çıkılıyor...\n");
                exit(0);

            default:
                printf("Geçersiz seçim.\n");
        }
    }

    return 0;
}
