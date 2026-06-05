#include "pch.h"
#include "EffectManager.h"
#include "Engine/Rendering/Pipeline/Graphics.h"
#include "Engine/Utils/JsonFileHandler.h"
#include "Engine/Editor/Dialog.h"
#include "Engine/Utils/JsonUtils.h"

void EffectManager::ClearAll()
{
	ClearEffectData();
	StopAll();
}

EffectHandle EffectManager::CreateEffectData()
{
	// 新しいエフェクトデータ追加用のハンドル
	//static EffectHandle nextHandle = 0;
	//EffectHandle handle = nextHandle++; // ハンドルはインクリメントしていく
	EffectHandle handle = static_cast<EffectHandle>(effectData.size());
	effectData.emplace(); // 新しいエフェクトデータを追加
	return handle;
}

EffectHandle EffectManager::LoadEffectData(const std::string& filePath)
{
	// エフェクトデータ読み込み
	if (std::filesystem::exists(filePath))
	{
		// すでに同じデータが存在する場合はそれを返す
		for (size_t i = 0; i < effectData.size(); ++i)
		{
			if (effectData[i].filePath == filePath)
			{
				return effectData[i].handle;
			}
		}

		json j;
		if (JsonFileHandler::LoadJsonFromFile(j, filePath))
		{
			// 新しいエフェクトデータ追加用のハンドル
			EffectHandle handle = CreateEffectData();
			effectData[handle].filePath = filePath;
			effectData[handle].handle = handle;

			// エフェクト名を反映
			effectData[handle].name = j.value("name", "Effect" + std::to_string(handle));

			// 読み込みデータをエミッターデータに反映
			auto& emitterDataList = effectData[handle].emitters;
			auto emitterJsonList = j.value("emitterDataList", json::array());
			for (const auto& emitterJson : emitterJsonList)
			{
				ParticleEmitterData emitterData;
				emitterData.name = emitterJson.value("name", "Emitter");
				emitterData.isEnabled = emitterJson.value("isEnabled", true);

				// エミット設定
				{
					emitterData.emitData.maxParticles = emitterJson.value("maxParticles", 1000);
					if (emitterJson.contains("emitCount"))
						emitterData.emitData.emitCount = emitterJson["emitCount"].get<Range<int>>();
					else
						emitterData.emitData.emitCount = { 10, 10 };
					if (emitterJson.contains("initialDelay"))
						emitterData.emitData.initialDelay = emitterJson["initialDelay"].get<Range<float>>();
					else
						emitterData.emitData.initialDelay = { 0.0f, 0.0f };
					if (emitterJson.contains("emitInterval"))
						emitterData.emitData.emitInterval = emitterJson["emitInterval"].get<Range<float>>();
					else
						emitterData.emitData.emitInterval = { 0.1f, 0.1f };
					if (emitterJson.contains("positionOffset"))
						emitterData.emitData.positionOffset = emitterJson["positionOffset"].get<Vector3>();
					if (emitterJson.contains("rotationEuler"))
						emitterData.emitData.rotationEuler = emitterJson["rotationEuler"].get<Range<Vector3>>();
					if (emitterJson.contains("endRotationEuler"))
						emitterData.emitData.endRotationEuler = emitterJson["endRotationEuler"].get<Range<Vector3>>();
					if (emitterJson.contains("rotationEasingTime"))
						emitterData.emitData.rotationEasingTime = emitterJson["rotationEasingTime"].get<Range<float>>();
					if (emitterJson.contains("rotationEasingType"))
						emitterData.emitData.rotationEasingType = emitterJson["rotationEasingType"].get<int>();
					emitterData.emitData.loop = emitterJson.value("loop", false);
					emitterData.emitData.duration = emitterJson.value("duration", 1.0f);
				}
				// 形状設定
				{
					emitterData.shapeData.shape = static_cast<ShapeType>(emitterJson.value("shapeType", 0));
					emitterData.shapeData.directionMode = static_cast<DirectionMode>(emitterJson.value("directionMode", 0));
					if (emitterJson.contains("directionAxis"))
						emitterData.shapeData.directionAxis = emitterJson["directionAxis"].get<Vector3>();
					emitterData.shapeData.speed = emitterJson.value("speed", Range<float>{ 1.0f, 1.0f });
					emitterData.shapeData.endSpeed = emitterJson.value("endSpeed", Range<float>{ 1.0f, 1.0f });
					emitterData.shapeData.speedEasingTime = emitterJson.value("speedEasingTime", Range<float>{ 0.0f, 0.0f });
					emitterData.shapeData.speedEasingType = emitterJson.value("speedEasingType", 0);
					emitterData.shapeData.radius = emitterJson.value("radius", 1.0f);
					emitterData.shapeData.height = emitterJson.value("height", 1.0f);
				}
				// 動作設定
				{
					if (emitterJson.contains("velocity"))
						emitterData.motionData.velocity = emitterJson["velocity"].get<Range<Vector3>>();
					if (emitterJson.contains("acceleration"))
						emitterData.motionData.acceleration = emitterJson["acceleration"].get<Range<Vector3>>();
					if (emitterJson.contains("lifeTime"))
						emitterData.motionData.lifeTime = emitterJson["lifeTime"].get<Range<float>>();
					emitterData.motionData.useGravity = emitterJson.value("useGravity", false);
				}
				// ビジュアル設定
				{
					emitterData.visualData.renderingMode = static_cast<RenderingMode>(emitterJson.value("renderingMode", 0));
					emitterData.visualData.texturePath = emitterJson.value("texturePath", "");
					auto textureSplitCountArray = emitterJson.value("textureSplitCount", std::vector<uint32_t>{ 1, 1 });
					if (textureSplitCountArray.size() == 2)	{
						emitterData.visualData.textureSplitCount = { textureSplitCountArray[0], textureSplitCountArray[1] };
					}
					emitterData.visualData.blendState = static_cast<BlendState>(emitterJson.value("blendState", 0));
					if (emitterJson.contains("startSize"))
						emitterData.visualData.startSize = emitterJson["startSize"].get<Range<Vector2>>();
					if (emitterJson.contains("endSize"))
						emitterData.visualData.endSize = emitterJson["endSize"].get<Range<Vector2>>();
					if (emitterJson.contains("sizeEasingTime"))
						emitterData.visualData.sizeEasingTime = emitterJson["sizeEasingTime"].get<Range<float>>();
					if (emitterJson.contains("sizeEasingType"))
						emitterData.visualData.sizeEasingType = emitterJson["sizeEasingType"].get<int>();
					if (emitterJson.contains("useGradient"))
						emitterData.visualData.useGradient = emitterJson["useGradient"].get<bool>();
					if (emitterJson.contains("startColor"))
						emitterData.visualData.startColor = emitterJson["startColor"].get<Range<Color>>();
					if (emitterJson.contains("endColor"))
						emitterData.visualData.endColor = emitterJson["endColor"].get<Range<Color>>();
					if (emitterJson.contains("enableFadeIn"))
						emitterData.visualData.enableFadeIn = emitterJson["enableFadeIn"].get<bool>();
					if (emitterJson.contains("enableFadeOut"))
						emitterData.visualData.enableFadeOut = emitterJson["enableFadeOut"].get<bool>();
					if (emitterJson.contains("fadeInTime"))
						emitterData.visualData.fadeInTime = emitterJson["fadeInTime"].get<Range<float>>();
					if (emitterJson.contains("fadeOutTime"))
						emitterData.visualData.fadeOutTime = emitterJson["fadeOutTime"].get<Range<float>>();

					// グラデーション設定
					{
						ImGradientHDRState gradientState{};
						// カラーのグラデーションマーカーを読み込む
						{
							auto gradientColorsJson = emitterJson.value("gradientColors", json::array());
							for (const auto& colorMarkerJson : gradientColorsJson)
							{
								ImGradientHDRState::ColorMarker colorMarker{};
								colorMarker.Position = colorMarkerJson.value("position", 0.0f);
								auto colorArray = colorMarkerJson.value("color", std::vector<float>{ 1.0f, 1.0f, 1.0f });
								if (colorArray.size() == 3) {
									colorMarker.Color[0] = colorArray[0];
									colorMarker.Color[1] = colorArray[1];
									colorMarker.Color[2] = colorArray[2];
								}
								colorMarker.Intensity = colorMarkerJson.value("intensity", 1.0f);
								gradientState.AddColorMarker(colorMarker.Position, { colorMarker.Color[0], colorMarker.Color[1], colorMarker.Color[2] }, colorMarker.Intensity);
							}
						}

						// アルファのグラデーションマーカーを読み込む
						{
							auto gradientAlphasJson = emitterJson.value("gradientAlphas", json::array());
							for (const auto& alphaMarkerJson : gradientAlphasJson)
							{
								ImGradientHDRState::AlphaMarker alphaMarker{};
								alphaMarker.Position = alphaMarkerJson.value("position", 0.0f);
								alphaMarker.Alpha = alphaMarkerJson.value("alpha", 1.0f);
								gradientState.AddAlphaMarker(alphaMarker.Position, alphaMarker.Alpha);
							}
						}
						// グラデーション状態をエミッターデータに反映
						emitterData.visualData.gradientState = gradientState;
					}
				}
				
				// エミッタデータリストに追加
				emitterDataList.push_back(emitterData);
			}

			// 成功したのでハンドルを返す
			return handle;
		}
	}
	// 読み込み失敗
	Console::LogError("Failed to load effect data from file: " + filePath);
	return -1; // 無効なハンドルを返す
}

EffectHandle EffectManager::LoadEffectDataWithDialog()
{
	// ファイルダイアログ表示
	{
		const char* filter = "JSON Files (*.json)\0*.json\0All Files (*.*)\0*.*\0";
		char filePath[256] = { 0 };
		HWND hwnd = Graphics::GetHwnd();
		DialogResult result = Dialog::OpenFileName(filePath, sizeof(filePath), filter, nullptr, hwnd);
		if (result == DialogResult::OK) {
			std::filesystem::path path(filePath);
			// 拡張子が.jsonでない場合、.jsonを追加
			path.replace_extension(".json");

			// エフェクトデータ読み込み
			return LoadEffectData(path.string());
		}
		else {
			return -1; // 無効なハンドルを返す
		}
	}
}

void EffectManager::SaveEffectData(EffectHandle handle, const std::string& filePath)
{
	// エフェクトデータ保存
	json j;
	// エフェクト名保存
	j["name"] = effectData.at(handle).name;
	// エミッタデータリスト初期化
	j["emitterDataList"] = json::array();
	// エミッタデータ保存
	for (const auto& emitterData : effectData.at(handle).emitters)
	{
		// エミッターデータをJSONに変換
		json emitterJson;
		emitterJson["name"] = emitterData.name;
		emitterJson["isEnabled"] = emitterData.isEnabled;
		// エミット設定
		{
			emitterJson["maxParticles"] = emitterData.emitData.maxParticles;
			emitterJson["emitCount"] = emitterData.emitData.emitCount;
			emitterJson["initialDelay"] = emitterData.emitData.initialDelay;
			emitterJson["emitInterval"] = emitterData.emitData.emitInterval;
			emitterJson["positionOffset"] = emitterData.emitData.positionOffset;
			emitterJson["rotationEuler"] = emitterData.emitData.rotationEuler;
			emitterJson["endRotationEuler"] = emitterData.emitData.endRotationEuler;
			emitterJson["rotationEasingTime"] = emitterData.emitData.rotationEasingTime;
			emitterJson["rotationEasingType"] = emitterData.emitData.rotationEasingType;
			emitterJson["loop"] = emitterData.emitData.loop;
			emitterJson["duration"] = emitterData.emitData.duration;
		}
		// 形状設定
		{
			emitterJson["shapeType"] = static_cast<uint8_t>(emitterData.shapeData.shape);
			emitterJson["directionMode"] = static_cast<uint8_t>(emitterData.shapeData.directionMode);
			emitterJson["directionAxis"] = emitterData.shapeData.directionAxis;
			emitterJson["speed"] = emitterData.shapeData.speed;
			emitterJson["endSpeed"] = emitterData.shapeData.endSpeed;
			emitterJson["speedEasingTime"] = emitterData.shapeData.speedEasingTime;
			emitterJson["speedEasingType"] = emitterData.shapeData.speedEasingType;
			emitterJson["radius"] = emitterData.shapeData.radius;
			emitterJson["height"] = emitterData.shapeData.height;
		}
		// 動作設定
		{
			emitterJson["velocity"] = emitterData.motionData.velocity;
			emitterJson["acceleration"] = emitterData.motionData.acceleration;
			emitterJson["lifeTime"] = emitterData.motionData.lifeTime;
			emitterJson["useGravity"] = emitterData.motionData.useGravity;
		}
		// ビジュアル設定
		{
			emitterJson["renderingMode"] = static_cast<uint8_t>(emitterData.visualData.renderingMode);
			emitterJson["texturePath"] = emitterData.visualData.texturePath;
			emitterJson["textureSplitCount"] = { emitterData.visualData.textureSplitCount.x, emitterData.visualData.textureSplitCount.y };
			emitterJson["blendState"] = static_cast<uint8_t>(emitterData.visualData.blendState);
			emitterJson["startSize"] = emitterData.visualData.startSize;
			emitterJson["endSize"] = emitterData.visualData.endSize;
			emitterJson["sizeEasingTime"] = emitterData.visualData.sizeEasingTime;
			emitterJson["sizeEasingType"] = emitterData.visualData.sizeEasingType;
			emitterJson["useGradient"] = emitterData.visualData.useGradient;
			emitterJson["startColor"] = emitterData.visualData.startColor;
			emitterJson["endColor"] = emitterData.visualData.endColor;
			emitterJson["enableFadeIn"] = emitterData.visualData.enableFadeIn;
			emitterJson["enableFadeOut"] = emitterData.visualData.enableFadeOut;
			emitterJson["fadeInTime"] = emitterData.visualData.fadeInTime;
			emitterJson["fadeOutTime"] = emitterData.visualData.fadeOutTime;

			// グラデーション設定
			{
				// カラーのグラデーションマーカーを保存
				for (int i = 0; i < emitterData.visualData.gradientState.ColorCount; ++i)
				{
					const auto& colorMarker = emitterData.visualData.gradientState.Colors[i];
					emitterJson["gradientColors"].push_back({
							{ "position", colorMarker.Position },
							{ "color", { colorMarker.Color[0], colorMarker.Color[1], colorMarker.Color[2] } },
							{ "intensity", colorMarker.Intensity }
						});
				}
				// アルファのグラデーションマーカーを保存
				for (int i = 0; i < emitterData.visualData.gradientState.AlphaCount; ++i)
				{
					const auto& alphaMarker = emitterData.visualData.gradientState.Alphas[i];
					emitterJson["gradientAlphas"].push_back({
							{ "position", alphaMarker.Position },
							{ "alpha", alphaMarker.Alpha }
						});
				}
			}
		}

		// エミッタデータリストに追加
		j["emitterDataList"].push_back(emitterJson);
	}
	// ファイルに保存
	JsonFileHandler::SaveJsonToFile(j, filePath);
}

void EffectManager::SaveEffectDataWithDialog(EffectHandle handle)
{
	// ファイルダイアログ表示
	{
		const char* filter = "JSON Files (*.json)\0*.json\0All Files (*.*)\0*.*\0";
		char filePath[256] = { 0 };
		HWND hwnd = Graphics::GetHwnd();
		DialogResult result = Dialog::SaveFileName(filePath, sizeof(filePath), filter, nullptr, ".json", hwnd);
		if (result == DialogResult::OK) {
			std::filesystem::path path(filePath);
			// 拡張子が.jsonでない場合、.jsonを追加
			path.replace_extension(".json");
			// エフェクトデータ保存
			SaveEffectData(handle, path.string());
		}
	}
}

// エフェクト再生
int EffectManager::Play(EffectHandle handle, const Vector3& pos, const Vector3& rot)
{
	// ハンドルチェック
	if (handle < 0 || handle >= static_cast<EffectHandle>(effectData.size()))
	{
		Console::LogError("EffectManager::Play: Invalid effect handle " + std::to_string(handle));
		return -1; // 無効な再生インスタンスIDを返す
	}

	// 新しい再生インスタンスIDを生成
	int playInstanceId = nextPlayInstanceId++;

	auto& effect = effectData[handle];
	auto& particleSystemMap = particleSystems[handle][playInstanceId]; // ハンドルと再生インスタンスIDに対応するパーティクルシステムマップ

	// 各エミッターデータを処理
	for (int emitterIndex = 0; emitterIndex < effect.emitters.size(); ++emitterIndex)
	{
		const auto& emitterData = effect.emitters[emitterIndex];
		const std::string& effectFilePath = effect.filePath;
		// テクスチャパスと最大パーティクル数取得
		const std::string& texturePath = emitterData.visualData.texturePath;
		uint32_t maxParticles = emitterData.emitData.maxParticles;

		// 該当するパーティクルシステムが存在しない場合は生成
		if (particleSystemMap.find(emitterIndex) == particleSystemMap.end())
		{
			AssetTexture texture;
			if (!texturePath.empty() && std::filesystem::exists(texturePath))
			{
				texture.LoadFromFile(texturePath);
			}
			else
			{
				texture.MakeDummy(Graphics::GetDevice());
			}
			//テクスチャSRV取得
			Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv = texture.GetSRV();
			//パーティクルシステム生成
			particleSystemMap[emitterIndex] = std::make_unique<ComputeParticleSystem>(Graphics::GetDevice(), maxParticles, srv, emitterData.visualData.textureSplitCount);
		}

		//for (int emitterIndex = 0; emitterIndex < effectData[handle].emitters.size(); ++emitterIndex)
		{
			const auto& emitData = effect.emitters[emitterIndex].emitData;

			float initialDelay = emitData.initialDelay.GetRandom();

			EmitterPlayState state;
			state.handle = handle;
			state.emitterIndex = emitterIndex; // これ必要か怪しい
			state.playInstanceId = playInstanceId;
			state.emitterData = emitterData; // エミッターデータをプレイ状態にコピー（再生中にエミッターデータが変更されても影響しないように）
			state.elapsedTime = 0.0f;
			state.nextEmitTime = initialDelay;
			state.isPlaying = true;
			state.position = pos;
			state.rotationEuler = rot;
			EmitOnce(state); // 初回エミットを即座に行う

			playingEmitters.push_back(state);
		}
	}
	return playInstanceId;
}

void EffectManager::Stop(EffectHandle handle)
{
	// ハンドルチェック
	if (handle < 0 || handle >= static_cast<EffectHandle>(effectData.size()))
	{
		Console::LogError("EffectManager::Stop: Invalid effect handle " + std::to_string(handle));
		return;
	}
	// 再生中のエミッタを停止
	for (auto& emitterState : playingEmitters)
	{
		if (emitterState.handle == handle)
		{
			emitterState.isPlaying = false;
		}
	}
}

void EffectManager::StopImmediate(int playInstanceId)
{
	// 再生インスタンスIDに対応するエミッタを停止
	for (auto& emitterState : playingEmitters)
	{
		if (emitterState.playInstanceId == playInstanceId)
		{
			emitterState.isPlaying = false;
		}
	}
}

bool EffectManager::IsPlaying(EffectHandle handle)
{
	// ハンドルチェック
	if (handle < 0 || handle >= static_cast<EffectHandle>(effectData.size()))
	{
		Console::LogError("EffectManager::IsPlaying: Invalid effect handle " + std::to_string(handle));
		return false;
	}
	// 再生中のエミッタが存在するかチェック
	for (const auto& emitterState : playingEmitters)
	{
		if (emitterState.handle == handle && emitterState.isPlaying)
		{
			return true;
		}
	}
	return false;
}

EffectHandle EffectManager::CopyEffectData(EffectHandle srcHandle)
{
	// エフェクトデータコピー
	if (srcHandle < 0 || srcHandle >= static_cast<EffectHandle>(effectData.size()))
	{
		Console::LogError("EffectManager::CopyEffectData: Invalid effect handle " + std::to_string(srcHandle));
		return -1; // 無効なハンドルを返す
	}
	// 新しいエフェクトデータ追加用のハンドル
	EffectHandle newHandle = CreateEffectData();
	// データコピー
	effectData[newHandle] = effectData[srcHandle];
	effectData[newHandle].handle = newHandle; // ハンドル更新
	effectData[newHandle].name += "_Copy"; // 名前更新
	effectData[newHandle].filePath += "_Copy"; // ファイルパス更新
	return newHandle;
}

EffectManager::EffectData& EffectManager::GetEffectData(EffectHandle handle)
{
	// エフェクトデータ取得
	if (handle < 0 || handle >= static_cast<EffectHandle>(effectData.size()))
	{
		throw std::out_of_range("EffectManager::GetEffectData: Invalid effect handle " + std::to_string(handle));
	}
	return effectData[handle];
}

void EffectManager::ClearEffectData()
{
	effectData.clear();
}

void EffectManager::StopAll()
{
	// 全エフェクト停止
	particleSystems.clear();
	playingEmitters.clear();
}

void EffectManager::Initialize()
{
	//パーティクルシステム初期化
	particleSystems.clear();
	playingEmitters.clear();
}

void EffectManager::Update(float deltaTime)
{
	// 再生中のエミッタを更新
	for (auto& emitterState : playingEmitters)
	{
		if (!emitterState.isPlaying)
			continue;

#ifdef _DEBUG
		const auto& emitData = emitterState.emitterData.emitData;
#else
		const auto& emitterData = effectData[emitterState.handle].emitters[emitterState.emitterIndex];
		const auto& emitData = emitterData.emitData;
#endif // _DEBUG

		
		// 経過時間更新
		emitterState.elapsedTime += deltaTime;

		// ループと再生停止の処理
		if (emitterState.elapsedTime >= emitData.duration)
		{
			if (emitData.loop)
			{
				emitterState.elapsedTime = fmod(emitterState.elapsedTime, emitData.duration); // ループする場合は経過時間をリセット
				EmitOnce(emitterState); // ループする場合は再度エミット
			}
			else
			{
				emitterState.isPlaying = false; // ループしない場合は再生停止
				continue;
			}
		}
	}

	// 再生停止したエミッタをリストから削除
	std::erase_if(playingEmitters, [](const EmitterPlayState& state) { return !state.isPlaying; }); // 再生停止したエミッタをリストから削除

	// 使われなくなったplayInstanceIdのパーティクルシステムをGC
	// handleごとに、まだplayingEmittersに残っているplayInstanceIdのセットを作成し、それ以外のplayInstanceIdのパーティクルシステムを削除する
	{
		// 生存中のplayInstanceIdのセットを作成
		std::unordered_map<EffectHandle, std::unordered_set<int>> alivePlayInstanceIds;
		for (const auto& emitterState : playingEmitters)
		{
			if (emitterState.isPlaying)
			{
				alivePlayInstanceIds[emitterState.handle].insert(emitterState.playInstanceId);
			}
		}

		// パーティクルシステムをGC
		for (auto& [handle, playInstanceMap] : particleSystems)
		{
			std::erase_if(playInstanceMap, [&](const auto& pair) {
				auto it = alivePlayInstanceIds.find(handle);
				return it == alivePlayInstanceIds.end() || it->second.count(pair.first) == 0; // 生存中のplayInstanceIdに存在しない場合は削除
				});
		}
		std::erase_if(particleSystems, [](const auto& pair) {
			return pair.second.empty(); // playInstanceMapが空のhandleは削除
			});
	}

	//パーティクルシステム更新
	ID3D11DeviceContext* immediateContext = Graphics::GetDeviceContext();

	for (auto& [handle, instanceMap] : particleSystems)
	{
		for (auto& [playInstanceId, particleSystemList] : instanceMap)
		{
			for (auto& [emitterIndex, particleSystem] : particleSystemList)
			{
				particleSystem->Update(immediateContext, deltaTime);
			}
		}
	}
}

void EffectManager::Render(RenderContext* rtx)
{
	//パーティクルシステム描画
	for (auto& [handle, instanceMap] : particleSystems)
	{
		for (auto& [playInstanceId, particleSystemList] : instanceMap)
		{
			for (auto& [emitterIndex, particleSystem] : particleSystemList)
			{
				// 描画前設定
				rtx->renderState->BindBlendState(rtx->immediateContext, particleSystem->blendState);
				rtx->renderState->BindDepthStencilState(rtx->immediateContext, particleSystem->depthStencilState);

				// 描画
				particleSystem->Render(rtx->immediateContext);
			}
		}
	}
}

void EffectManager::ReInitializeParticleSystem()
{
	// パーティクルシステム再初期化
	StopAll();
}

void EffectManager::EmitOnce(const EmitterPlayState& state)
{
	// エフェクトを一度だけエミット
#ifdef _DEBUG
	const auto& emitterData = state.emitterData;
#else
	const auto& emitterData = effectData[state.handle].emitters[state.emitterIndex];
#endif // _DEBUG

	if (!emitterData.isEnabled)
	{
		return; // エミットが無効な場合は何もしない
	}

	const std::string& effectFilePath = effectData[state.handle].filePath;
	const std::string& texturePath = emitterData.visualData.texturePath;
	auto& particleSystemList = particleSystems[state.handle][state.playInstanceId];

	if (particleSystemList.find(state.emitterIndex) == particleSystemList.end())
	{
		Console::LogError("EffectManager::EmitOnce: Particle system not found for effect " + effectFilePath + " emitter index " + std::to_string(state.emitterIndex));
		return;
	}


	// ブレンドステート設定
	particleSystemList[state.emitterIndex]->blendState = emitterData.visualData.blendState;
	particleSystemList[state.emitterIndex]->depthStencilState = emitterData.visualData.renderingMode == RenderingMode::ScreenSpace ? DepthStencilState::NoTestNoWrite : DepthStencilState::TestOnly;


	// エミット数分ループ
	int emitCount = emitterData.emitData.emitCount.GetRandom();
	for (int i = 0; i < emitCount; ++i)
	{
		ComputeParticleSystem::EmitParticleData emitData;
		emitData.parameter.x = static_cast<float>(emitterData.visualData.renderingMode); // 描画モード設定
		emitData.parameter.y = emitterData.motionData.lifeTime.GetRandom(); // 生存時間設定

		float delayTime = 0.0f; // 遅延時間初期化
		delayTime += emitterData.emitData.initialDelay.GetRandom(); // 初期遅延時間設定
		delayTime += emitterData.emitData.emitInterval.GetRandom() * i; // エミット間隔設定

		emitData.parameter.z = delayTime; // 遅延時間設定
		emitData.parameter.w = state.emitterIndex; // エミッタインデックスをw成分に設定
		if (!emitterData.visualData.useGradient)
		{
			emitData.parameter.w = -1.0f; // グラデーションを使用しない場合はw成分に-1を設定
		}

		// 位置設定
		{
			emitData.position.x = state.position.x;
			emitData.position.y = state.position.y;
			emitData.position.z = state.position.z;
		}

		// 回転設定
		{
			Vector3 rotationEuler = emitterData.emitData.rotationEuler.GetRandom();
			XMFLOAT3 euler = {
				rotationEuler.x + state.rotationEuler.x + 180.0f, // X軸回転に180度追加（向き反転を補正）
				rotationEuler.y + state.rotationEuler.y,
				rotationEuler.z + state.rotationEuler.z
			};
			emitData.rotation.x = DirectX::XMConvertToRadians(euler.x);
			emitData.rotation.y = DirectX::XMConvertToRadians(euler.y);
			emitData.rotation.z = DirectX::XMConvertToRadians(euler.z);
			emitData.rotation.w = static_cast<float>(emitterData.emitData.rotationEasingType); // 回転イージングタイプをw成分に設定

			Vector3 endRotationEuler = emitterData.emitData.endRotationEuler.GetRandom();
			XMFLOAT3 endEuler = {
				endRotationEuler.x + state.rotationEuler.x + 180.0f, // X軸回転に180度追加（向き反転を補正）
				endRotationEuler.y + state.rotationEuler.y,
				endRotationEuler.z + state.rotationEuler.z
			};
			emitData.endRotation.x = DirectX::XMConvertToRadians(endEuler.x);
			emitData.endRotation.y = DirectX::XMConvertToRadians(endEuler.y);
			emitData.endRotation.z = DirectX::XMConvertToRadians(endEuler.z);
			emitData.endRotation.w = emitterData.emitData.rotationEasingTime.GetRandom(); // 回転イージング時間をw成分に設定
		}

		// 初速度設定
		{
			Vector3 velocity = emitterData.motionData.velocity.GetRandom();
			emitData.velocity.x = velocity.x;
			emitData.velocity.y = velocity.y;
			emitData.velocity.z = velocity.z;
		}

		// 加速度設定
		{
			Vector3 acceleration = emitterData.motionData.acceleration.GetRandom();
			if (emitterData.motionData.useGravity)
			{
				acceleration += Vector3(0.0f, -9.81f, 0.0f); // 重力加速度を追加
			}
			emitData.acceleration = { acceleration.x, acceleration.y, acceleration.z, 0.0f };
		}

		// シェイプエミッタ設定を適用
		ApplyShapeEmitterSettings(emitterData.shapeData, emitData, i, emitCount);

		// その他設定
		{
			// 生成位置オフセット適用
			emitData.position.x += emitterData.emitData.positionOffset.x;
			emitData.position.y += emitterData.emitData.positionOffset.y;
			emitData.position.z += emitterData.emitData.positionOffset.z;
		}

		// ビジュアル設定
		{
			Vector2 startSize = emitterData.visualData.startSize.GetRandom();
			Vector2 endSize = emitterData.visualData.endSize.GetRandom();
			float easingMode = static_cast<float>(emitterData.visualData.sizeEasingType); // サイズイージングタイプをz成分に設定
			emitData.scale = DirectX::XMFLOAT4(startSize.x, startSize.y, easingMode, 0);
			float easingTime = emitterData.visualData.sizeEasingTime.GetRandom();
			emitData.endScale = DirectX::XMFLOAT4(endSize.x, endSize.y, easingTime, 0);

			Color startColor = emitterData.visualData.startColor.GetRandom();
			Color endColor = emitterData.visualData.endColor.GetRandom();
			emitData.startColor = startColor;
			emitData.endColor = endColor;
			if (emitterData.visualData.enableFadeIn)
			{
				float fadeInTime = emitterData.visualData.fadeInTime.GetRandom();
				emitData.fadeInTime = fadeInTime;
			}
			if (emitterData.visualData.enableFadeOut)
			{
				float fadeOutTime = emitterData.visualData.fadeOutTime.GetRandom();
				emitData.fadeOutTime = fadeOutTime;
			}
		}

		// エミット
		particleSystemList[state.emitterIndex]->Emit(emitData);
	}

	// グラデーションの設定 (描画モードがグラデーションの場合のみ)
	// TODO: 描画モードがグラデーションの場合のみ設定するように変更
	if (emitterData.visualData.useGradient)
	{
		particleSystemList[state.emitterIndex]->SetGradient(state.emitterIndex, emitterData.visualData.gradientState);
	}
}


void EffectManager::ApplyShapeEmitterSettings(const EmitterShapeData& s, ComputeParticleSystem::EmitParticleData& emitData, int index, int emitCount)
{
	// シェイプエミッタ設定をemitDataに適用
	Vector3 position{};
	Vector3 velocityDir{};

	// 位置設定
	switch (s.shape)
	{
		case ShapeType::Point:
		{
			// 点の位置は原点
			position = Vector3(0.0f, 0.0f, 0.0f);
			break;
		}
		case ShapeType::Ring:
		{
			// リング上の均等な位置を取得
			float angle = (static_cast<float>(index) / emitCount) * DirectX::XM_2PI;
			position = Vector3(std::cosf(angle), 0.0f, std::sinf(angle)) * s.radius;
			break;
		}
		case ShapeType::Sphere:
		{
			// ランダムな球の中の位置を取得
			float u = Random(0.0f, 1.0f); // 0~1のランダムな値
			float v = Random(0.0f, 1.0f); // 0~1のランダムな値
			float theta = u * DirectX::XM_2PI; // 0~2PIのランダムな角度
			float phi = std::acosf(2.0f * v - 1.0f); // 0~PIのランダムな角度
			float r = s.radius * std::cbrtf(Random(0.0f, 1.0f)); // 半径を考慮したランダムな距離
			float x = r * std::sinf(phi) * std::cosf(theta);
			float y = r * std::sinf(phi) * std::sinf(theta);
			float z = r * std::cosf(phi);
			position = Vector3(x, y, z);
			break;
		}
		case ShapeType::Cylinder:
		{
			// ランダムな円柱の中の位置を取得
			float angle = Random(0.0f, DirectX::XM_2PI);
			float radius = Random(0.0f, s.radius);
			float height = Random(-s.height / 2.0f, s.height / 2.0f);
			float x = radius * std::cosf(angle);
			float z = radius * std::sinf(angle);
			position = Vector3(x, height, z);
			break;
		}
	}

	// 方向設定
	switch (s.directionMode)
	{
		case DirectionMode::Default:
		{
			velocityDir = Vector3(0, 0, 0); // 速度方向は設定しない
			break;
		}
		case DirectionMode::Axis:
		{
			// 指定軸方向を正規化して使用
			velocityDir = s.directionAxis.Normalize();
			break;
		}
		case DirectionMode::Random:
		{
			velocityDir = RandomDirection();
			break;
		}
		case DirectionMode::Outward:
		{
			// 位置ベクトルがゼロベクトルに近い場合の対策
			if (position.LengthSq() < 1e-6f)
			{
				// 位置ベクトルがゼロベクトルに近い場合、ランダム方向を使用
				velocityDir = RandomDirection();
			}
			else // 通常の場合
			{
				velocityDir = position.Normalize();
			}
			break;
		}
		case DirectionMode::Inward:
		{
			// 位置ベクトルがゼロベクトルに近い場合の対策
			if (position.LengthSq() < 1e-6f)
			{
				// 位置ベクトルがゼロベクトルに近い場合、方向はゼロベクトルにする
				velocityDir = Vector3(0.0f, 0.0f, 0.0f);
			}
			else // 通常の場合
			{
				velocityDir = (position * -1.0f).Normalize();
			}
			break;
		}
		case DirectionMode::Normal:
		{
			// 形状に応じた法線方向を設定
			switch (s.shape)
			{
				case ShapeType::Point:
				{
					velocityDir = Vector3(0.0f, 1.0f, 0.0f); // 上方向
					break;
				}
				case ShapeType::Ring:
				case ShapeType::Sphere:
				{
					// 位置ベクトルがゼロベクトルに近い場合の対策
					if (position.LengthSq() < 1e-6f)
					{
						velocityDir = RandomDirection();
					}
					else
					{
						velocityDir = position.Normalize();
					}
					break;
				}
				case ShapeType::Cylinder:
				{
					// 上下面に近い場合
					const float topThreshold = s.height * 0.45f;
					if (std::fabs(position.y) > topThreshold)
					{
						velocityDir = Vector3(0.0f, position.y > 0 ? 1.0f : -1.0f, 0.0f);
					}
					else
					{
						Vector3 radialDir = Vector3(position.x, 0.0f, position.z);
						velocityDir = radialDir.LengthSq() < 1e-6f ?
							Vector3(0.0f, 1.0f, 0.0f) : radialDir.Normalize();
					}
					break;
				}
			}
			break;
		}
	}
	
	// 回転行列計算
	XMMATRIX rotMatrix = XMMatrixRotationRollPitchYawFromVector(XMLoadFloat4(&emitData.rotation));
	XMFLOAT3 localPos = position;
	XMVECTOR LocalPos = XMLoadFloat3(&localPos);

	// 位置回転
	XMVECTOR RotatedLocalPos = XMVector3Transform(LocalPos, rotMatrix);

	// ワールド位置計算
	XMFLOAT3 emitterWorldPos = { emitData.position.x, emitData.position.y, emitData.position.z };
	XMVECTOR EmitterWorldPos = XMLoadFloat3(&emitterWorldPos);
	XMVECTOR WorldPos = RotatedLocalPos + EmitterWorldPos;

	// 位置設定
	XMStoreFloat4(&emitData.position, WorldPos);

	// 速度ベクトル計算
	XMVECTOR dir = XMVector3TransformNormal(XMLoadFloat3(reinterpret_cast<XMFLOAT3*>(&velocityDir)), rotMatrix);
	XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&velocityDir), dir);

	Vector3 velocity = velocityDir.Normalize();

	// 速度設定
	emitData.velocity.x += velocity.x;
	emitData.velocity.y += velocity.y;
	emitData.velocity.z += velocity.z;

	// 速度のランダム設定
	emitData.startSpeed = s.speed.GetRandom();
	emitData.endSpeed = s.endSpeed.GetRandom();
	emitData.speedEasingMode = static_cast<float>(s.speedEasingType); // 速度イージングタイプを設定
	emitData.speedEasingTime = s.speedEasingTime.GetRandom(); // 速度イージング時間を設定
}

float EffectManager::Random(float min, float max)
{
	// minからmaxまでのランダムな浮動小数点数を生成
	return min + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (max - min)));
}

Vector3 EffectManager::RandomBoxPosition(const Vector3& size)
{
	float x = Random(-size.x / 2.0f, size.x / 2.0f);
	float y = Random(-size.y / 2.0f, size.y / 2.0f);
	float z = Random(-size.z / 2.0f, size.z / 2.0f);
	return Vector3(x, y, z);
}

Vector3 EffectManager::RandomDirection()
{
	// ランダムな単位ベクトルを生成
	// -1~1のランダムなz値
	float z = Random(-1.0f, 1.0f);
	// 0~2PIのランダムな角度
	float theta = Random(0.0f, DirectX::XM_2PI);
	// 半径を計算
	float r = std::sqrtf(1.0f - z * z);
	// x,y座標を計算
	float x = r * std::cosf(theta);
	float y = r * std::sinf(theta);

	return Vector3(x, y, z);
}

Vector3 EffectManager::RandomHemisphereDirection(const Vector3& normal)
{
	Vector3 dir = RandomDirection();
	if (dir.Dot(normal) < 0.0f)
	{
		dir = dir * -1.0f;
	}
	return dir.Normalize();
}

Vector3 EffectManager::RandomConeDirection(const Vector3& dir, float coneAngle)
{
	// 基準方向を上ベクトルにして回転行列を作成
	float cosTheta = Random(std::cosf(XMConvertToRadians(coneAngle)), 1.0f);
	float sinTheta = std::sqrtf(1.0f - cosTheta * cosTheta);
	float phi = Random(0.0f, DirectX::XM_2PI);

	Vector3 localDir{
		sinTheta * std::cosf(phi),
		cosTheta,
		sinTheta * std::sinf(phi),
	};

	// dir方向への回転行列を計算
	Vector3 axis = Vector3(Vector3::up).Cross(dir).Normalize();
	float angle = std::acosf(Vector3(Vector3::up).Dot(dir.Normalize()));

	// dirが上ベクトルとほぼ平行な場合、回転は不要
	if (axis.LengthSq() < 1e-6f)
	{
		return dir.Normalize();
	}

	// ロドリゲスの回転公式を使用して回転
	float cosA = std::cosf(angle);
	float sinA = std::sinf(angle);
	Vector3 rotatedDir =
		localDir * cosA +
		axis.Cross(localDir) * sinA +
		axis * (axis.Dot(localDir)) * (1 - cosA);
	return rotatedDir.Normalize();
}