#pragma once
#include <DirectXMath.h>
#include <Effekseer.h>
#include <EffekseerRendererDX11.h>
#include "Engine/Rendering/Pipeline/Graphics.h"
#include <queue>
class EffectManager
{
public:
	using UserHandle = int;//ユーザー側のエフェクト識別用ハンドル

	static void Initialize() {
		ID3D11Device* device = Graphics::GetDevice();
		ID3D11DeviceContext* immediateContext = Graphics::GetDeviceContext();
		//Effekseerレンダラ生成
		effekseerRenderer = EffekseerRendererDX11::Renderer::Create(device, immediateContext, 2048);

		//Effekseerマネージャー生成
		effekseerManager = Effekseer::Manager::Create(2048);

		//Effekseerレンダラの各種設定
		effekseerManager->SetSpriteRenderer(effekseerRenderer->CreateSpriteRenderer());
		effekseerManager->SetRibbonRenderer(effekseerRenderer->CreateRibbonRenderer());
		effekseerManager->SetRingRenderer(effekseerRenderer->CreateRingRenderer());
		effekseerManager->SetTrackRenderer(effekseerRenderer->CreateTrackRenderer());
		effekseerManager->SetModelRenderer(effekseerRenderer->CreateModelRenderer());
		//Effekseer内でのローダーの設定
		effekseerManager->SetTextureLoader(effekseerRenderer->CreateTextureLoader());
		effekseerManager->SetModelLoader(effekseerRenderer->CreateModelLoader());
		effekseerManager->SetMaterialLoader(effekseerRenderer->CreateMaterialLoader());

		//Effekseerを左手座標系で計算する
		effekseerManager->SetCoordinateSystem(Effekseer::CoordinateSystem::LH);
	}

	static void Finalize() {
		effectMap.clear();
		handleMap.clear();
		effekseerRenderer.Reset();
		effekseerManager.Reset();
	}

	static void Update(float elapsedTime) {
		//エフェクトをロード
		while (!loadQueue.empty()) {
			auto& [userHandle, filePath] = loadQueue.front();
			effectMap[userHandle] = LoadEffect(filePath);
			loadQueue.pop();
		}
		//エフェクトを再生
		while (!playQueue.empty()) {
			UserHandle userHandle = playQueue.front();
			playQueue.pop();
			if (effectMap.count(userHandle)) {
				Effekseer::Handle handle = effekseerManager->Play(effectMap[userHandle], Effekseer::Vector3D());
				handleMap[userHandle] = handle;//ユーザーハンドルとEffekseer::Handleを紐づけ
			}
		}
		//エフェクトを停止
		while (!stopQueue.empty()) {
			UserHandle userHandle = stopQueue.front();
			stopQueue.pop();
			if (effectMap.count(userHandle)) {
				effekseerManager->StopEffect(handleMap[userHandle]);
			}
		}
		//エフェクト更新処理
		effekseerManager->Update(elapsedTime * 60.f);
	}

	static void Render(const DirectX::XMFLOAT4X4& view, const DirectX::XMFLOAT4X4& projection) {
		//ビュー＆プロジェクション行列をEffekseerレンダラに設定
		effekseerRenderer->SetCameraMatrix(*reinterpret_cast<const Effekseer::Matrix44*>(&view));
		effekseerRenderer->SetProjectionMatrix(*reinterpret_cast<const Effekseer::Matrix44*>(&projection));

		//Effekseer描画開始
		effekseerRenderer->BeginRendering();

		// Effekseer描画実行
		effekseerManager->Draw();

		//Effekseer描画終了
		effekseerRenderer->EndRendering();
	}

	static Effekseer::ManagerRef GetEffekseerManager() {
		return effekseerManager; 
	}


	static UserHandle LoadRequest(const std::string& filePath) {
		UserHandle userHandle = nextUserHandle++;
		loadQueue.push(std::make_pair(userHandle, filePath));
		return userHandle;
	}
	static void PlayRequest(UserHandle handle) {
		playQueue.push(handle);
	}
	static void StopRequest(UserHandle handle) {
		stopQueue.push(handle);
	}
	static void SetPosition(UserHandle handle, const XMFLOAT3& position) {
		effekseerManager->SetLocation(handleMap[handle], reinterpret_cast<const Effekseer::Vector3D&>(position));
	}
	static void SetRotation(UserHandle handle, const Quaternion& rotation) {
		XMVECTOR Q = XMLoadFloat4(&rotation);
		XMVECTOR Axis;
		float angle;
		XMQuaternionToAxisAngle(&Axis, &angle, Q);
		XMFLOAT3 axis;
		XMStoreFloat3(&axis, Axis);
		effekseerManager->SetRotation(handleMap[handle], reinterpret_cast<const Effekseer::Vector3D&>(axis), angle);
	}
	static void SetScale(UserHandle handle, const XMFLOAT3& scale) {
		effekseerManager->SetScale(handleMap[handle], scale.x, scale.y, scale.z);
	}
	static Effekseer::Handle GetHandle(UserHandle handle) { return handleMap[handle]; }
private:
	static Effekseer::EffectRef LoadEffect(const std::string& filePath) {
		char16_t utf16FilePath[256];
		Effekseer::ConvertUtf8ToUtf16(utf16FilePath, 256, filePath.c_str());
		return Effekseer::Effect::Create(effekseerManager, (EFK_CHAR*)utf16FilePath);
	}
private:
	static inline Effekseer::ManagerRef effekseerManager;
	static inline EffekseerRenderer::RendererRef effekseerRenderer;

	static inline std::queue<std::pair<UserHandle, std::string>> loadQueue;   // **エフェクトのロードリクエストキュー**
	static inline std::queue<UserHandle> playQueue;   // **エフェクトの再生リクエストキュー**
	static inline std::queue<UserHandle> stopQueue;   // **エフェクトの停止リクエストキュー**

	static inline std::unordered_map<UserHandle, Effekseer::EffectRef> effectMap; //エフェクトID管理
	static inline std::unordered_map<UserHandle, Effekseer::Handle> handleMap; //Effekseer::Handleと同期

	static inline UserHandle nextUserHandle = 1;
};