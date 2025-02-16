#include "windrv.h"
#include <stdio.h>
#include <string.h>

#define WINDRV_PORT 0xE9F000
#define WINDRV_BUFFER 0xE9F001

static char windrv_filename[256]; // WINDRV で開くファイル名
static FILE* windrv_fp = NULL;    // WINDRV で開いたファイルのポインタ
static BYTE windrv_command = 0;   // WINDRV から受け取るコマンド

// *********************************
// TODO: Not accurate at all!!!
// *********************************

// WINDRV からのコマンドを取得
BYTE Windrv_ReadCommand() {
    return windrv_command;
}

// WINDRV からのデータ（ファイル名やデータ）を取得
void Windrv_SetFilename() {
    // TODO:
    // strncpy(windrv_filename, (char*)(MEM + WINDRV_BUFFER), 255);
    // windrv_filename[255] = '\0';
    strcpy(windrv_filename, "hoge\0");
}

// WINDRV のリクエストを処理する
void Windrv_ProcessCommand(BYTE command) {
    windrv_command = command;
    printf("*** Windrv command: %02x\n", command);

    switch (command) {
    case 0x01: // ファイルを開く
        Windrv_SetFilename();
        windrv_fp = fopen(windrv_filename, "rb");
        break;
    case 0x02: // ファイルを閉じる
        if (windrv_fp) {
            fclose(windrv_fp);
            windrv_fp = NULL;
        }
        break;
    case 0x03: // ファイルを読み込む
        if (windrv_fp) {
            // TODO:
            // fread(MEM + WINDRV_BUFFER, 1, 256,
            //       windrv_fp); // 読み込んだデータをバッファに格納
        }
        break;
    case 0x04: // ファイルを書き込む
        if (windrv_fp) {
            // TODO:
            // fwrite(MEM + WINDRV_BUFFER, 1, 256, windrv_fp);
        }
        break;
    case 0x28: // init?
        break;
    default:
        printf("*** Unknown Windrv command: %02x\n", command);
        break;
    }
}
