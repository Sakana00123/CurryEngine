#pragma once
#include <objbase.h>
#include <string>

/**
 * @file
 * @brief GUIDおよびインスタンスIDを生成するユーティリティ関数群。
 * @details GUIDはCOMのCoCreateGuidを利用して生成します。
 * @note GUIDはグローバルに一意なID、インスタンスIDはアプリケーション実行中に一意なIDです。
 * @warning インスタンスID生成関数はスレッドセーフではありません。必要に応じて呼び出し元で同期を行ってください。
 * @warning インスタンスIDはアプリケーションの実行中にのみ一意であり、再起動後にはリセットされます。
 * @warning インスタンスIDの範囲はint型の範囲内に制限されます。非常に多くのIDを生成するとオーバーフローする可能性があります。
 * @see https://learn.microsoft.com/en-us/windows/win32/api/combaseapi/nf-combaseapi-cocreateguid
 * @see https://en.cppreference.com/w/cpp/language/storage_duration
 * @see https://en.cppreference.com/w/cpp/language/types
 * @note 例外はスローしません。
 */

/**
 * @brief 新しいGUIDを生成し、文字列として返す。
 * @return 生成されたGUID文字列（例: "3F2504E0-4F89-11D3-9A0C-0305E82C3301"）。
 */
inline std::string GenerateGUID() {
    GUID guid;
    HRESULT hr = CoCreateGuid(&guid);
    char buffer[64];
    snprintf(buffer, sizeof(buffer),
        "%08lX-%04X-%04X-%04X-%02X%02X%02X%02X%02X%02X",
        guid.Data1, guid.Data2, guid.Data3,
        (guid.Data4[0] << 8) | guid.Data4[1],
        guid.Data4[2], guid.Data4[3], guid.Data4[4],
        guid.Data4[5], guid.Data4[6], guid.Data4[7]);
    return std::string(buffer);
}

/**
 * @brief 新しいインスタンスIDを生成し、返す。
 * @return 生成されたインスタンスID（0から始まる連番）。
 */
int GenerateInstanceID();

int GetCurrentInstanceID();

/**
 * @brief インスタンスIDカウンタをリセットする。次にGenerateInstanceIDが呼び出されたとき、0から始まるIDが生成される。
 */
void ResetInstanceID(int id);