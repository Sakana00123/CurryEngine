#pragma once
#include "Engine/Physics/Physics.h"

struct CollisionInfoDto
{
	uint64_t selfId; // 自身のGameObjectのID
	uint64_t selfColliderId; // 自身のColliderのID(衝突イベントが発生したColliderのID)
	uint64_t otherId; // 衝突相手のGameObjectのID
	uint64_t otherColliderId; // 衝突相手のColliderのID(衝突イベントが発生したColliderのID)
	float impulseX; // 衝突を解決するために互いのコライダに加えられた合計の衝撃量（ベクトル）のX成分
	float impulseY; // 衝突を解決するために互いのコライダに加えられた合計の衝撃量（ベクトル）のY成分
	float impulseZ; // 衝突を解決するために互いのコライダに加えられた合計の衝撃量（ベクトル）のZ成分
	uint32_t contactCount; // 接触点の数
	// 接触点の情報を格納する配列。最大数は MAX_CONTACTS_PER_PAIR で定義されている。
	struct ContactPointDto
	{
		float pointX; // 接触点の位置のX成分
		float pointY; // 接触点の位置のY成分
		float pointZ; // 接触点の位置のZ成分
		float normalX; // 接触点の法線ベクトルのX成分
		float normalY; // 接触点の法線ベクトルのY成分
		float normalZ; // 接触点の法線ベクトルのZ成分
		float separation; // 接触点でのコライダーの距離
		uint64_t thisId; // 自身のGameObjectのID
		uint64_t thisColliderId; // 自身のコライダのID
		uint64_t otherId; // 衝突相手のGameObjectのID
		uint64_t otherColliderId; // 衝突相手のコライダのID
	} contacts[MAX_CONTACTS_PER_PAIR];
};

struct TriggerInfoDto
{
	uint64_t selfId; // 自身のGameObjectのID
	uint64_t selfColliderId; // 自身のColliderのID(トリガーイベントが発生したColliderのID)
	uint64_t otherId; // トリガー相手のGameObjectのID
	uint64_t otherColliderId; // トリガー相手のColliderのID(トリガーイベントが発生したColliderのID)
};

// スクリプトクラスのプロパティの説明
struct ScriptPropertyDesc
{
	const char* name;
	const char* type;
};

struct ScriptClassDesc
{
	const char* name;
	const ScriptClassDesc* baseClass; // 継承元クラスの説明（nullptr なら継承なし）
	const ScriptPropertyDesc* properties; // プロパティの配列（nullptr ならプロパティなし）
	int propertyCount; // プロパティの数
};

// C#から呼ばれる関数ポインタ型 (UnmanagedCallersOnly でエクスポートされる関数のシグネチャに合わせる)
using RegisterScriptClassFunc = void(__stdcall*)(const ScriptClassDesc* desc);