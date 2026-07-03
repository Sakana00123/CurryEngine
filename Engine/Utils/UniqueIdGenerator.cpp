#include "pch.h"
#include "UniqueIdGenerator.h"

#include "Engine/Editor/Console.h"


static int g_globalInstanceID = 0; // グローバルなインスタンスIDカウンタ

/**
 * @brief 新しいインスタンスIDを生成し、返す。
 * @return 生成されたインスタンスID（0から始まる連番）。
 */
int GenerateInstanceID()
{
    int newID = g_globalInstanceID++; // カウンタをインクリメントして新しいIDを取得
    Console::Log("Generated new instance ID: " + std::to_string(newID));
    return newID;
}

int GetCurrentInstanceID()
{
    int currentID = g_globalInstanceID;
    Console::Log("Current instance ID counter value: " + std::to_string(currentID));
    return currentID; // 現在のカウンタ値を取得
}

/**
 * @brief インスタンスIDカウンタをリセットする。次にGenerateInstanceIDが呼び出されたとき、0から始まるIDが生成される。
 */
void ResetInstanceID(int id)
{
    int oldID = g_globalInstanceID;// カウンタをリセット
    g_globalInstanceID = id;
    if (id < oldID) {
        Console::Log("Instance ID counter reset to " + std::to_string(id) + " (previously " + std::to_string(oldID) + ")");
    }
    else {
        Console::Log("Instance ID counter set to " + std::to_string(id) + " (previously " + std::to_string(oldID) + ")");
    }
}