/*

  EFS example
  -----------

  file        : main.c
  author      : Noda
  description : libnds example of EFS use

  history : 

    13/05/2007 - v1.0
      = Original release
      
    28/09/2007 - v1.1
      - removed EFS_Flush() call, as it's now automatic
      + added sequential reading test
      
    25/05/2008 - v2.0
      + updated example to EFSlib v2.0
      + added a few tests

*/

#include <nds.h>
#include <stdio.h>
#include <malloc.h>
#include <fat.h>
#include <unistd.h>


#include "efs_lib.h"    // include EFS lib

int main(void) {

    // init nds
    irqInit();
    irqEnable(IRQ_VBLANK);
    videoSetMode(0);	// not using the main screen
    videoSetModeSub(MODE_0_2D | DISPLAY_BG0_ACTIVE);	// sub bg 0 will be used to print text
    vramSetBankC(VRAM_C_SUB_BG); 

    SUB_BG0_CR = BG_MAP_BASE(31);
    
    BG_PALETTE_SUB[255] = RGB15(31,31,31);	// by default font will be rendered with color 255
	
    //consoleInit() is a lot more flexible but this gets you up and running quick
    consoleInitDefault((u16*)SCREEN_BASE_BLOCK_SUB(31), (u16*)CHAR_BASE_BLOCK_SUB(0), 16);

    // init EFSlib & libfat
    if(EFS_Init(EFS_AND_FAT | EFS_DEFAULT_DEVICE, NULL)) {
    
        iprintf("EFS init OK!\n");
        iprintf("found NDS path: %s\n", efs_path);
        
        DIR_ITER* dir;
        struct stat st;
        s8 nb;
        FILE* file;
        u8* buffer;
        int i, size;

        // open a text file and read its contents
        file = fopen("/test.txt", "rb");
        if(file != NULL) {
            // get file size using stat            
            stat("/test.txt", &st);
            size = st.st_size;
            
            buffer = (u8*)malloc(size);
            fread(buffer, 1, size, file);
            buffer[size-1] = '\0';
            iprintf("\n/test.txt content: '%s'\nsize: %d bytes\n", buffer, size);
            free(buffer);
            fclose(file); 
        }

        // open another file, read its content
        file = fopen("/folder/test.txt", "rb");
        if(file != NULL) {
            // get file size
            fseek(file, 0, SEEK_END);
            size = ftell(file); 
            fseek(file, 0, SEEK_SET);

            buffer = (u8*)malloc(size);
            fread(buffer, 1, size, file);
            buffer[size-1] = '\0';
            iprintf("\n/folder/test.txt content:\n%s\n", buffer);
            free(buffer); 
            fclose(file);
        }
            
        // reopen the file, modify its content
        file = fopen("/folder/test.txt", "rb+");
        if(file != NULL) {
            nb = fwrite("16b Written OK!", 1, 16, file);
            iprintf("\nwrite test done! : %d bytes\n", nb);
            fclose(file);
        }

        // reopen another file, read its content again, 1 byte at a time
        file = fopen("/folder/dummy/.././test.txt", "rb");  // funky path to test relative path parsing
        if(file != NULL) {
            // get file size
            fseek(file, 0, SEEK_END);
            size = ftell(file); 
            fseek(file, 0, SEEK_SET);

            buffer = (u8*)malloc(size);
            
            i = 0;
            while(i < size) {
                fread(&buffer[i], 1, 1, file);
                i++;
            }
            
            buffer[size-1] = '\0';
            iprintf("\n/folder/test.txt new content:\n%s\n", buffer);
            free(buffer); 
            fclose(file);
        }

        iprintf("\nPress A for directory tests.\n");
        do {
            scanKeys();
        } while(!(keysDown() & KEY_A));
        consoleClear();

        // open root directory then list its content
        iprintf("Listing '/' directory:\n");
        dir = diropen(".");
        
        if(dir != NULL) {
            buffer = (u8*)malloc(EFS_MAXNAMELEN);
            
            while(!(dirnext(dir, (char*)buffer, &st))) {
                if(st.st_mode & S_IFDIR)
                    iprintf("DIR : %s\n", buffer);
                else
                    iprintf("FILE: %s, size: %d bytes\n", buffer, st.st_size);
            }
            
            iprintf("end of directory.\n");
            iprintf("\ndirectory reset, first file is:\n");
            
            dirreset(dir);
            dirnext(dir, (char*)buffer, &st);
            if(st.st_mode & S_IFDIR)
                iprintf("DIR : %s\n", buffer);
            else
                iprintf("FILE: %s, size: %d bytes\n", buffer, st.st_size);
            
            dirclose(dir);
            free(buffer);
        }

        // chdir to a directory then list its content
        iprintf("\nListing '/list/' directory:\n");
        chdir("/list/");
        dir = diropen("./");
        
        if(dir != NULL) {
            buffer = (u8*)malloc(EFS_MAXNAMELEN);
            
            while(!(dirnext(dir, (char*)buffer, &st))) {
                if(st.st_mode & S_IFDIR)
                    iprintf("DIR : %s\n", buffer);
                else
                    iprintf("FILE: %s, size: %d bytes\n", buffer, st.st_size);
            }
            
            iprintf("end of directory.\n");
            dirclose(dir);
            free(buffer);
        }

    } else {
        iprintf("EFS init error!\n");
    }

    while(1) {
        swiWaitForVBlank();
    }

    return 0;
}
