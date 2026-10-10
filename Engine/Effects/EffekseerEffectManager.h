#pragma once
#include <DirectXMath.h>
#include <Effekseer.h>
#include <EffekseerRendererDX11.h>
#include <queue>
#include <Engine\Core\Math\Quaternion.h>
#include "Engine/Resources/AssetId.h"

class EffekseerEffectManager
{
public:
	using UserHandle = int;//ユーザー側のエフェクト識別用ハンドル
	struct EffectInstance
	{
		Effekseer::EffectRef effect;
		Effekseer::Handle handle;
	};

	struct EffectInstanceData
	{
		Effekseer::EffectRef effect;
		Effekseer::Handle handle;
		Vector3 position;
		Quaternion rotation;
		Vector3 scale;
	};

	struct PlayRequestParameter
	{
		UserHandle handle;
		Vector3 position;
		Quaternion rotation;
		Vector3 scale;
	};

	static void Initialize();

	static void Finalize();

	static void Update(float deltaTime);

	static void Render(const DirectX::XMFLOAT4X4& view, const DirectX::XMFLOAT4X4& projection);

	static Effekseer::ManagerRef GetEffekseerManager() {
		return effekseerManager;
	}


	static UserHandle LoadRequest(const CurryEngine::Resources::AssetId& assetId);

	static void PlayRequest(const PlayRequestParameter& param);
	static void StopRequest(UserHandle handle);
	static void SetPosition(UserHandle handle, const Vector3& position);
	static void SetRotation(UserHandle handle, const Quaternion& rotation);
	static void SetScale(UserHandle handle, const Vector3& scale);
	static Effekseer::Handle GetHandle(UserHandle handle) { return handleMap[handle]; }
private:
	static Effekseer::EffectRef LoadEffect(const std::string& filePath);
private:
	static inline Effekseer::ManagerRef effekseerManager;
	static inline EffekseerRenderer::RendererRef effekseerRenderer;

	static inline std::queue<std::pair<UserHandle, std::string>> loadQueue;   // **エフェクトのロードリクエストキュー**
	static inline std::queue<PlayRequestParameter> playQueue;   // **エフェクトの再生リクエストキュー**
	static inline std::queue<UserHandle> stopQueue;   // **エフェクトの停止リクエストキュー**

	static inline std::unordered_map<UserHandle, Effekseer::EffectRef> effectMap; //エフェクトID管理
	static inline std::unordered_map<UserHandle, Effekseer::Handle> handleMap; //Effekseer::Handleと同期

	static inline UserHandle nextUserHandle = 1;
};
