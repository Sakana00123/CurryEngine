#include "pch.h"
#include "EffekseerEffectManager.h"
#include "Engine/Rendering/Pipeline/Graphics.h"
#include <Engine\Resources\AssetDatabase.h>

void EffekseerEffectManager::Initialize()
{
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

void EffekseerEffectManager::Finalize()
{
	effectMap.clear();
	handleMap.clear();
	effekseerRenderer.Reset();
	effekseerManager.Reset();
}

void EffekseerEffectManager::Update(float deltaTime)
{
	//エフェクトをロード
	while (!loadQueue.empty()) {
		auto& [userHandle, filePath] = loadQueue.front();
		effectMap[userHandle] = LoadEffect(filePath);
		loadQueue.pop();
	}
	//エフェクトを再生
	while (!playQueue.empty()) {
		const PlayRequestParameter& param = playQueue.front();
		UserHandle userHandle = param.handle;
		Effekseer::Vector3D position(param.position.x, param.position.y, param.position.z);
		playQueue.pop();
		if (effectMap.count(userHandle)) {
			Effekseer::Handle handle = effekseerManager->Play(effectMap[userHandle], position);
			handleMap[userHandle] = handle;//ユーザーハンドルとEffekseer::Handleを紐づけ
			// 回転、スケールを設定
			SetRotation(userHandle, param.rotation);
			SetScale(userHandle, param.scale);
		}
	}
	//エフェクトを停止
	while (!stopQueue.empty()) {
		UserHandle userHandle = stopQueue.front();
		stopQueue.pop();
		if (handleMap.count(userHandle)) {
			effekseerManager->StopEffect(handleMap[userHandle]);
			handleMap.erase(userHandle);
		}
	}
	//エフェクト更新処理
	effekseerManager->Update(deltaTime * 60.f);
}

void EffekseerEffectManager::Render(const DirectX::XMFLOAT4X4& view, const DirectX::XMFLOAT4X4& projection)
{
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

EffekseerEffectManager::UserHandle EffekseerEffectManager::LoadRequest(const CurryEngine::Resources::AssetId& assetId)
{
	UserHandle userHandle = nextUserHandle++;
	// アセットIDからファイルパスを取得
	auto* meta = CurryEngine::Resources::AssetDatabase::Find(assetId);
	if (!meta) {
		LOG_ERROR(u8"[EffekseerEffectManager] アセットIDに対応するメタデータが見つかりません: " + std::u8string(assetId.ToString().begin(), assetId.ToString().end()));
		return -1; // エラーコードを返す
	}
	std::string filePath = meta->path.string();
	loadQueue.push(std::make_pair(userHandle, filePath));
	return userHandle;
}

void EffekseerEffectManager::PlayRequest(const PlayRequestParameter& param)
{
	playQueue.push(param);
}

void EffekseerEffectManager::StopRequest(UserHandle handle)
{
	stopQueue.push(handle);
}

void EffekseerEffectManager::SetPosition(UserHandle handle, const Vector3& position)
{
	if (handleMap.count(handle)) {
		effekseerManager->SetLocation(handleMap[handle], reinterpret_cast<const Effekseer::Vector3D&>(position));
	}
}

void EffekseerEffectManager::SetRotation(UserHandle handle, const Quaternion& rotation)
{
	if (handleMap.count(handle))
	{
		XMVECTOR Q = XMLoadFloat4(&rotation);
		XMVECTOR Axis;
		float angle;
		XMQuaternionToAxisAngle(&Axis, &angle, Q);
		XMFLOAT3 axis;
		XMStoreFloat3(&axis, Axis);
		effekseerManager->SetRotation(handleMap[handle], reinterpret_cast<const Effekseer::Vector3D&>(axis), angle);
	}
}

void EffekseerEffectManager::SetScale(UserHandle handle, const Vector3& scale)
{
	if (handleMap.count(handle)) {
		effekseerManager->SetScale(handleMap[handle], scale.x, scale.y, scale.z);
	}
}

Effekseer::EffectRef EffekseerEffectManager::LoadEffect(const std::string& filePath)
{
	char16_t utf16FilePath[256];
	Effekseer::ConvertUtf8ToUtf16(utf16FilePath, 256, filePath.c_str());
	return Effekseer::Effect::Create(effekseerManager, (EFK_CHAR*)utf16FilePath);
}
