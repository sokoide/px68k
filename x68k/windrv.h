#ifndef __WINDRV_H__
#define __WINDRV_H__

#include "common.h" // 共通ヘッダ

#define WINDRV_PORT 0xE9F000   // WINDRV の I/O ポート
#define WINDRV_BUFFER 0xE9F001 // WINDRV のデータバッファアドレス

// WINDRV のコマンドを処理する関数
void Windrv_ProcessCommand(BYTE command);

// WINDRV からのリクエストを取得する関数
BYTE Windrv_ReadCommand();

// WINDRV に渡されたファイル名を取得
void Windrv_SetFilename();

#endif // __WINDRV_H__
