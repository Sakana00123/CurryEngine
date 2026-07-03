#include "pch.h"
#include "ParticleComponent.h"
#include "Engine/Core/GameObject.h"
#include "Engine/Core/Transform.h"
#include "Engine/Editor/Dialog.h"

REGISTER_COMPONENT(ParticleComponent, "Effects");

void ParticleComponent::Awake()
{
	if (playOnAwake && effectHandle != -1)
	{
		Play();
	}
}

void ParticleComponent::OnDestroy()
{
	Stop();
}

void ParticleComponent::Load(const std::string& filePath)
{
	this->filePath = filePath;
	effectHandle = EffectManager::LoadEffectData(filePath);
}

void ParticleComponent::Play()
{
	//if (effectHandle != -1) return;
	
	// エフェクト発生前コールバック実行
	if (settings.onPreEmit)
	{
		settings.onPreEmit();
	}

	// エフェクト再生
	{
		// ゲームオブジェクトの位置と回転を取得
		Vector3 position = GetOwner()->GetTransform()->GetWorldPosition();
		Vector3 rotation = GetOwner()->GetTransform()->GetWorldRotation().ToEuler();

		// 線上にエフェクトを再生する場合
		if (settings.lineData.useLine)
		{
			// 線上に分割してエフェクトを再生
			for (int i = 0; i <= settings.lineData.segments.size(); ++i)
			{
				LineData::Segment& segment = settings.lineData.segments[i % settings.lineData.segments.size()];

				Vector3 startPos = segment.start ? segment.start->GetWorldPosition() : Vector3();
				Vector3 endPos = segment.end ? segment.end->GetWorldPosition() : Vector3();
				int segmentCount = segment.segmentCount > 0 ? segment.segmentCount : 1;

				if (segment.start && segment.end)
				{
					// 始点から終点へ向く回転を設定
					XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&rotation), Transform::QuaternionLookAt(Transform::QuaternionToXMVector(segment.start->GetWorldRotation()), Transform::QuaternionToXMVector(segment.end->GetWorldRotation())));
				}

				for (int j = 0; j < segmentCount; ++j)
				{
					float t = static_cast<float>(j) / static_cast<float>(segmentCount);
					Vector3 pos{};
					pos.x = startPos.x + (endPos.x - startPos.x) * t;
					pos.y = startPos.y + (endPos.y - startPos.y) * t;
					pos.z = startPos.z + (endPos.z - startPos.z) * t;
					int instanceId = EffectManager::Play(effectHandle, pos, rotation);
					instanceIDs.push_back(instanceId);
				}
			}
		}
		else
		{
			// 通常の位置でエフェクト再生
			int instanceId = EffectManager::Play(effectHandle, position, rotation);
			instanceIDs.push_back(instanceId);
		}
	}
}

void ParticleComponent::Stop()
{
	// 全インスタンス停止
	for (int instanceId : instanceIDs)
	{
		EffectManager::StopImmediate(instanceId);
	}
	instanceIDs.clear();
}

bool ParticleComponent::IsPlaying() const
{
	return isPlaying;
}

void ParticleComponent::SetEffectData(const EffectManager::EffectData& data)
{
	EffectManager::GetEffectData(effectHandle) = data;
}

void ParticleComponent::Update(float elapsedTime)
{
	// ループ再生でない場合は何もしない
	if (effectHandle == -1 || !IsPlaying())
	{
		return;
	}

	// エフェクトが再生中か確認
	isPlaying = EffectManager::IsPlaying(effectHandle);
}

#ifdef USE_IMGUI
void ParticleComponent::DrawProperty(const PropertyDrawContext& context)
{
	Component::DrawProperty(context);
	//// ファイルパス表示
	//ImGui::Text("File Path: %s", filePath.c_str());
	//ImGui::SameLine();

	//// ファイル選択ボタン
	//if (ImGui::Button("..."))
	//{
	//	// ダイアログを開いてエフェクトデータを選択
	//	char filename[256]{};
	//	char filter[] = "Particle Effect Files\0*.json\0All Files\0*.*\0";
	//	
	//	if (Dialog::OpenFileName(filename, sizeof(filename), filter) == DialogResult::OK)
	//	{
	//		filePath = filename;
	//		Load(filePath);
	//	}
	//}

	//// Play On Awake チェックボックス
	//if (ImGui::Checkbox("Play On Awake", &playOnAwake))
	//{

	//}

	if (context.isMultiSelect)
	{
		ImGui::Text("Multiple Particle Components Selected");
		return;
	}

	if (effectHandle != -1)
	{
		if (ImGui::Button("Play Effect"))
		{
			Play();
		}
		if (ImGui::Button("Stop Effect"))
		{
			Stop();
		}

		ImGui::Text("Effect Handle: %d", effectHandle);
		ImGui::Text("Is Playing: %s", IsPlaying() ? "True" : "False");
	}
	else
	{
		ImGui::Text("No Effect Loaded");
	}


}
#endif // USE_IMGUI

json ParticleComponent::Serialize() const
{
	json j = Component::Serialize();
	j["filePath"] = filePath;
	j["playOnAwake"] = playOnAwake;
	return j;
}

void ParticleComponent::Deserialize(const json& j)
{
	Component::Deserialize(j);
	if (j.contains("filePath"))
	{
		filePath = j["filePath"].get<std::string>();
		Load(filePath);
	}
	if (j.contains("playOnAwake"))
	{
		playOnAwake = j["playOnAwake"].get<bool>();
	}
}