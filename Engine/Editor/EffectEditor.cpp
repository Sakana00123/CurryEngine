#include "pch.h"
#define NOMINMAX
#include "EffectEditor.h"
#include <filesystem>
#ifdef USE_IMGUI
#include <imgui.h>
#include <ImGradientHDR.h>
#endif // USE_IMGUI
#include "Engine/Rendering/Pipeline/Graphics.h"
#include "Engine/Utils/JsonFileHandler.h"
#include "Dialog.h"
#include "Engine/EditorSupport/ImGuiHelpers.h"
#include <Engine\Rendering\Camera\EditorCamera.h>
#include <Engine\Scenes\SceneManager.h>

static const char* EasingTypes[] = {
	"Linear",
	"InQuad",
	"OutQuad",
	"InOutQuad",
	"InCubic",
	"OutCubic",
	"InOutCubic",
	"InQuart",
	"OutQuart",
	"InOutQuart",
	"InQuint",
	"OutQuint",
	"InOutQuint",
	"InSine",
	"OutSine",
	"InOutSine",
	"InExpo",
	"OutExpo",
	"InOutExpo",
	"InCirc",
	"OutCirc",
	"InOutCirc",
	"InBack",
	"OutBack",
	"InOutBack",
	"InElastic",
	"OutElastic",
	"InOutElastic",
};


void EffectEditor::Show()
{
	isOpen = true;

	if (EditorCamera* editorCamera = SceneManager::GetCurrentScene()->GetEditorCamera(EDITOR_CAMERA_EFFECT_PREVIEW))
	{
		editorCamera->Initialize();
		editorCamera->SetPosition(Vector3::Zero);
	}
}

bool EffectEditor::IsOpen()
{
	return isOpen;
}

bool EffectEditor::IsPreviewFocused()
{
	return isPreviewFocused;
}

void EffectEditor::Initialize()
{
	// 初期化処理が必要ならここに追加
}

void EffectEditor::DrawGUI(RenderContext* context)
{
#ifdef USE_IMGUI
	if (!isOpen)
	{
		isPreviewFocused = false;
		return;
	}
	else
	{
		ImGui::Begin("Effect Editor", &isOpen);

		// ロード・セーブ・適用・クリアボタン
		{
			// ロード・セーブボタン
			if (ImGui::Button("Load"))
			{ 
				if (EffectHandle handle = EffectManager::LoadEffectDataWithDialog(); handle > -1)
				{
					currentEffectHandle = handle;
				}
			}
			ImGui::SameLine();
			if (ImGui::Button("Save")) { EffectManager::SaveEffectDataWithDialog(currentEffectHandle); }
			ImGui::SameLine(0, 30.0f);
			// クリアボタン
			if (ImGui::Button("Clear")) { EffectManager::ClearAll(); }

		}

		//ImGui::BeginTable("EffectEditorTable", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable);
		//ImGui::TableSetupColumn("EffectPreviewWindow", ImGuiTableColumnFlags_WidthStretch);
		//ImGui::TableSetupColumn("Effect Data List", ImGuiTableColumnFlags_WidthFixed, 200.0f);

		//// 左側のカラム
		//ImGui::TableNextRow();
		//ImGui::TableSetColumnIndex(0);
		
		// エフェクトプレビューウィンドウ
		if (auto previewImage = static_cast<RenderTexture*>(context->GetSharedResource("PreRenderTexture")))
		{
			static constexpr float aspectRatio = 9.0f / 16.0f; // 16:9のアスペクト比
			static constexpr float windowWidth = 400.0f; // ウィンドウの幅を固定
			static constexpr float windowHeight = windowWidth * aspectRatio; // 高さをアスペクト比に基づいて計算
			ImVec2 windowSize(windowWidth, windowHeight);
			ImGui::Image(previewImage->GetSRV(), windowSize);

			// プレビューウィンドウがフォーカスされているかどうかをチェック
			isPreviewFocused = ImGui::IsItemHovered();
		}

		// 右側のカラム
		//ImGui::TableSetColumnIndex(1);

		// エフェクトデータリスト
		if (ImGui::CollapsingHeader("Effect Data List", ImGuiTreeNodeFlags_Leaf))
		{
			ImGui::Dummy(ImVec2(0.0f, 3.0f)); // 少しスペースを空ける

			//for (size_t emitterIndex = 0; emitterIndex < EffectManager::effectData.size(); ++emitterIndex)
			for (auto& [emitterIndex, effect] : EffectManager::effectData)
			{
				// 一意のIDをプッシュ
				ImGui::PushID(static_cast<int>(emitterIndex));

				ImGui::Dummy(ImVec2(0.0f, 2.0f)); // 少しスペースを空ける

				// クリック状態フラグ
				bool isClicked = false;

				// セレクタブルでエフェクト名を表示
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 1, 1, 1));
				ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.25f, 0.25f, 0.25f, 1.0f));
				ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
				ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.3f, 0.3f, 0.3f, 1.0f));

				ImGui::Dummy(ImVec2(2.0f, 0.0f)); // 少しスペースを空ける
				ImGui::SameLine();
				isClicked |= ImGui::Selectable(effect.name.c_str(), currentEffectHandle == emitterIndex);
				ImGui::PopStyleColor(4);

				// クリックしたら現在のエフェクトハンドルを更新
				if (isClicked)
				{
					currentEffectHandle = static_cast<EffectHandle>(emitterIndex);
				}
#if 1
				// 右クリックで削除メニュー表示
				if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
				{
					ImGui::OpenPopup("EffectDataContextMenu");
				}
				if (ImGui::BeginPopup("EffectDataContextMenu"))
				{
#if 0
					if (ImGui::MenuItem("Delete"))
					{
						EffectManager::effectData.erase(EffectManager::effectData.begin() + emitterIndex);
						// 現在のエフェクトハンドルが削除された場合、無効にする
						if (currentEffectHandle == static_cast<EffectHandle>(emitterIndex))
						{
							currentEffectHandle = -1;
						}
						ImGui::EndPopup();
						ImGui::PopID();
						break; // ループを抜けて再描画
					}
#endif // 0
					if (ImGui::MenuItem("Copy"))
					{
						// エフェクトデータをコピー（複製）
						EffectManager::CopyEffectData(static_cast<EffectHandle>(emitterIndex));
					}
					ImGui::EndPopup();
				}
#endif // 0
				ImGui::PopID();
			}

			ImGui::Dummy(ImVec2(0.0f, 5.0f)); // 少しスペースを空ける

			// 新しいエフェクトデータ追加ボタン
			if (ImGui::Button("Add New Effect Data"))
			{
				currentEffectHandle = EffectManager::CreateEffectData();
				// 追加したエフェクトデータの名前を設定
				EffectManager::effectData[currentEffectHandle].name = "Effect " + std::to_string(currentEffectHandle);
			}
			ImGui::SameLine();
			// 全エフェクトデータクリアボタン
			if (ImGui::Button("Clear All Effect Data"))
			{
				EffectManager::ClearEffectData();
				currentEffectHandle = -1;
			}
		}

		ImGui::Dummy(ImVec2(0.0f, 5.0f)); // 少しスペースを空ける

		ImGui::Separator();
		ImGui::Dummy(ImVec2(0.0f, 5.0f)); // 少しスペースを空ける

		// エミッタエディタ
		{
			// 現在のエフェクトハンドルが有効かチェック
			if (currentEffectHandle < 0 || currentEffectHandle >= EffectManager::effectData.size())
			{
				ImGui::Text("No Emitter Data Selected.");
				//ImGui::EndTable();
				ImGui::End();
				return;
			}
			// エミッタデータリストの参照
			auto& emitterDataList = EffectManager::effectData.at(currentEffectHandle).emitters;

			// エフェクト名編集
			char effectNameBuffer[256]{};
			if (!ImGui::IsItemEdited())
			{
				strncpy_s(effectNameBuffer, EffectManager::effectData.at(currentEffectHandle).name.c_str(), sizeof(effectNameBuffer));
			}
			ImGui::InputText("Effect Name", effectNameBuffer, sizeof(effectNameBuffer));
			if (ImGui::IsItemDeactivatedAfterEdit())
			{
				EffectManager::effectData.at(currentEffectHandle).name = effectNameBuffer;
			}
			// エフェクト再生ボタン
			if (ImGui::Button("Play"))
			{
				EffectManager::Play(currentEffectHandle);
			}
			ImGui::SameLine();
			// エフェクト停止ボタン
			if (ImGui::Button("Stop"))
			{
				EffectManager::Stop(currentEffectHandle);
			}

			ImGui::BeginChild("EmitterDataList", ImVec2(0, -35.0f));

			// 各エミッタデータ表示
			for (size_t i = 0; i < emitterDataList.size(); ++i)
			{
				// 一意のIDをプッシュ
				ImGui::PushID(static_cast<int>(i));

				auto& emitterData = emitterDataList[i];
				//if (ImGui::TreeNode((void*)(intptr_t)i, "Emitter %zu", i))
				ImGui::Checkbox("##isEnabled", &emitterData.isEnabled);
				ImGui::SameLine();
				ImGui::SetNextItemOpen(true, ImGuiCond_Once);
				if (ImGui::TreeNodeEx(emitterData.name.c_str(), ImGuiTreeNodeFlags_DefaultOpen))
				{
					char nameBuffer[256]{};
					if (!ImGui::IsItemEdited())
					{
						strncpy_s(nameBuffer, emitterData.name.c_str(), sizeof(nameBuffer));
					}
					ImGui::InputText("Name", nameBuffer, sizeof(nameBuffer));
					if (ImGui::IsItemDeactivatedAfterEdit())
					{
						emitterData.name = nameBuffer;
					}

					// duration
					DrawFloat("Duration", emitterData.emitData.duration, 0.1f, 0.0f, 100.0f);

					// loop
					DrawCheckbox("Loop", emitterData.emitData.loop);

					// テクスチャパスのバッファ
					char texturePathBuffer[256]{};
					// テクスチャパスの参照ボタン
					if (ImGui::Button("Browse"))
					{
						// ファイルダイアログを開く
						const char* filter = "Image Files\0*.png;*.jpg;*.jpeg;*.bmp;*.tga\0All Files\0*.*\0";
						DialogResult result = Dialog::OpenFileName(texturePathBuffer, sizeof(texturePathBuffer), filter, "Select Texture", NULL, false);
						if (result == DialogResult::OK)
						{
							emitterData.visualData.texturePath = texturePathBuffer;
						}
					}
					ImGui::SameLine();
					// テクスチャパスの入力欄
					if (!ImGui::IsItemEdited())
					{
						strncpy_s(texturePathBuffer, emitterData.visualData.texturePath.c_str(), sizeof(texturePathBuffer));
					}
					ImGui::InputText("Texture Path", texturePathBuffer, sizeof(texturePathBuffer));
					if (ImGui::IsItemDeactivatedAfterEdit())
					{
						emitterData.visualData.texturePath = texturePathBuffer;
					}
					ImGui::DragInt2("Texture Split", reinterpret_cast<int*>(&emitterData.visualData.textureSplitCount.x), 1, 1, 100);

					// ビジュアル設定
					{
						const char* renderingModeItems[] = { "Billboard", "StretchedBillboard", "FixedRotation", "ScreenSpace" };
						int renderingModeIndex = static_cast<int>(emitterData.visualData.renderingMode);
						/*if (ImGui::Combo("Rendering Mode", &renderingModeIndex, renderingModeItems, IM_ARRAYSIZE(renderingModeItems)))
						{
							emitterData.visualData.renderingMode = static_cast<EffectManager::RenderingMode>(renderingModeIndex);
						}*/
						auto renderingModeSetter = [&emitterData](int index)
						{
							emitterData.visualData.renderingMode = static_cast<EffectManager::RenderingMode>(index);
						};
						DrawCombo("Rendering Mode", renderingModeIndex, renderingModeItems, IM_ARRAYSIZE(renderingModeItems), renderingModeSetter);

						const char* blendStateItems[] = { "Opaque", "Transparency", "Additive", "Subtraction", "Multiply" };
						int blendStateIndex = static_cast<int>(emitterData.visualData.blendState);
						/*if (ImGui::Combo("Blend State", &blendStateIndex, blendStateItems, IM_ARRAYSIZE(blendStateItems)))
						{
							emitterData.visualData.blendState = static_cast<BlendState>(blendStateIndex);
						}*/
						auto blendStateSetter = [&emitterData](int index)
						{
							emitterData.visualData.blendState = static_cast<BlendState>(index);
						};
						DrawCombo("Blend State", blendStateIndex, blendStateItems, IM_ARRAYSIZE(blendStateItems), blendStateSetter);
					}

					DrawInt("Max Particles", emitterData.emitData.maxParticles, 1, 1, 100000);
					emitterData.emitData.maxParticles = (std::max)(1, emitterData.emitData.maxParticles);

					// エミット設定
					if (ImGui::TreeNode("Emit Settings"))
					{
						DrawRangeInt("Emit Count", emitterData.emitData.emitCount, 1, 1, emitterData.emitData.maxParticles);
						DrawRangeFloat("Initial Delay", emitterData.emitData.initialDelay, 0.1f, 0.0f, 100.0f);
						DrawRangeFloat("Emit Interval", emitterData.emitData.emitInterval, 0.1f, 0.0f, 100.0f);
						DrawVector3("Position", emitterData.emitData.positionOffset, 0.1f);

						// TODO: イージング系のパラメータのUI修正するときはここいじる
						if (ImGui::TreeNodeEx("Rotation", ImGuiTreeNodeFlags_DefaultOpen))
						{

							DrawRangeVector3("Start Rotation", emitterData.emitData.rotationEuler, 1.0f);
							DrawRangeVector3("End Rotation", emitterData.emitData.endRotationEuler, 1.0f);
							
							auto easeTypeSetter = [&emitterData](int index)
							{
								emitterData.emitData.rotationEasingType = index;
							};
							DrawCombo("Rotation Easing Type", emitterData.emitData.rotationEasingType, EasingTypes, IM_ARRAYSIZE(EasingTypes), easeTypeSetter);
							DrawRangeFloat("Rotation Easing Time", emitterData.emitData.rotationEasingTime, 0.1f, 0.0f, emitterData.motionData.lifeTime.max);
							ImGui::TreePop();
						}
						
						//ImGui::Checkbox("Loop", &emitterData.emitData.loop);
						ImGui::TreePop();
					}
					// 形状エミッタ設定
					if (ImGui::TreeNode("Shape Emitter Settings"))
					{
						const char* shapeTypeItems[] = { "Point", "Ring", "Sphere", "Cylinder" };
						int shapeTypeIndex = static_cast<int>(emitterData.shapeData.shape);
						/*if (ImGui::Combo("Shape Type", &shapeTypeIndex, shapeTypeItems, IM_ARRAYSIZE(shapeTypeItems)))
						{
							emitterData.shapeData.shape = static_cast<EffectManager::ShapeType>(shapeTypeIndex);
						}*/
						auto shapeTypeSetter = [&emitterData](int index)
						{
							emitterData.shapeData.shape = static_cast<EffectManager::ShapeType>(index);
						};
						DrawCombo("Shape Type", shapeTypeIndex, shapeTypeItems, IM_ARRAYSIZE(shapeTypeItems), shapeTypeSetter);

						const char* directionModeItems[] = { "Default", "Random", "Axis", "Outward", "Inward", "Normal" };
						int directionModeIndex = static_cast<int>(emitterData.shapeData.directionMode);
						/*if (ImGui::Combo("Direction Mode", &directionModeIndex, directionModeItems, IM_ARRAYSIZE(directionModeItems)))
						{
							emitterData.shapeData.directionMode = static_cast<EffectManager::DirectionMode>(directionModeIndex);
						}*/
						auto directionModeSetter = [&emitterData](int index)
						{
							emitterData.shapeData.directionMode = static_cast<EffectManager::DirectionMode>(index);
						};
						DrawCombo("Direction Mode", directionModeIndex, directionModeItems, IM_ARRAYSIZE(directionModeItems), directionModeSetter);

						// パラメータ表示マスク定義
						static constexpr uint32_t None = 0;
						static constexpr uint32_t DirectionAxis = 1 << 0;
						static constexpr uint32_t Speed = 1 << 1;
						static constexpr uint32_t Radius = 1 << 2;
						static constexpr uint32_t Height = 1 << 3;
						// 形状タイプごとのパラメータ表示マスク
						static constexpr uint32_t shapeParamMask[] = {
							/*ShapeType::Point*/ None,
							/*ShapeType::Ring*/ Radius,
							/*ShapeType::Sphere*/ Radius,
							/*ShapeType::Cylinder*/ (Radius | Height),
						};
						// 方向モードごとのパラメータ表示マスク
						static constexpr uint32_t directionParamMask[] = {
							/*DirectionMode::Default*/ None,
							/*DirectionMode::Axis*/ DirectionAxis | Speed,
							/*DirectionMode::Random*/ Speed,
							/*DirectionMode::Outward*/ Speed,
							/*DirectionMode::Inward*/ Speed,
							/*DirectionMode::Normal*/ Speed,
						};
						// 現在のエミッタ設定に基づく表示フラグ
						uint32_t parameterFlags = shapeParamMask[static_cast<uint32_t>(emitterData.shapeData.shape)] |
							directionParamMask[static_cast<uint32_t>(emitterData.shapeData.directionMode)];

						if (parameterFlags & Speed)
						{
							// TODO: イージング系のパラメータのUI修正するときはここいじる
							if (ImGui::TreeNodeEx("Speed", ImGuiTreeNodeFlags_DefaultOpen))
							{
								DrawRangeFloat("Start Speed", emitterData.shapeData.speed, 0.1f, 0.0f, 100.0f);
								DrawRangeFloat("End Speed", emitterData.shapeData.endSpeed, 0.1f, 0.0f, 100.0f);

								auto easeTypeSetter = [&emitterData](int index)
								{
									emitterData.shapeData.speedEasingType = index;
								};
								DrawCombo("Speed Easing Type", emitterData.shapeData.speedEasingType, EasingTypes, IM_ARRAYSIZE(EasingTypes), easeTypeSetter);
								DrawRangeFloat("Speed Easing Time", emitterData.shapeData.speedEasingTime, 0.1f, 0.0f, emitterData.motionData.lifeTime.max);

								ImGui::TreePop();
							}
						}
						if (parameterFlags & DirectionAxis)
						{
							DrawVector3("Direction Axis", emitterData.shapeData.directionAxis, 0.1f);
						}
						if (parameterFlags & Radius)
						{
							DrawFloat("Radius", emitterData.shapeData.radius, 0.1f, 0.0f, 100.0f);
						}
						if (parameterFlags & Height)
						{
							DrawFloat("Height", emitterData.shapeData.height, 0.1f, 0.0f, 100.0f);
						}

						ImGui::TreePop();
					}

					// 動作設定
					if (ImGui::TreeNode("Motion Settings"))
					{
						DrawRangeVector3("Velocity", emitterData.motionData.velocity, 0.1f);
						DrawRangeVector3("Acceleration", emitterData.motionData.acceleration, 0.1f);
						DrawRangeFloat("Lifetime", emitterData.motionData.lifeTime, 0.1f, 0.0f, 100.0f);
						DrawCheckbox("Use Gravity", emitterData.motionData.useGravity);
						ImGui::TreePop();
					}

					// ビジュアル設定
					if (ImGui::TreeNode("Visual Settings"))
					{
						// TODO: イージング系のパラメータのUI修正するときはここいじる
						if (ImGui::TreeNodeEx("Size", ImGuiTreeNodeFlags_DefaultOpen))
						{
							DrawRangeVector2("Start Size", emitterData.visualData.startSize, 0.1f);
							DrawRangeVector2("End Size", emitterData.visualData.endSize, 0.1f);

							auto easeTypeSetter = [&emitterData](int index)
							{
								emitterData.visualData.sizeEasingType = index;
							};
							DrawCombo("Size Easing Type", emitterData.visualData.sizeEasingType, EasingTypes, IM_ARRAYSIZE(EasingTypes), easeTypeSetter);
							DrawRangeFloat("Size Easing Time", emitterData.visualData.sizeEasingTime, 0.1f, 0.0f, emitterData.motionData.lifeTime.max);

							ImGui::TreePop();
						}

						
						// 色の設定方法を切り替えるためのフラグ（0: 開始/終了色、1: グラデーション）
						// TODO: UIで切り替えられるようにする
						DrawCheckbox("Use Gradient", emitterData.visualData.useGradient);

						if (!emitterData.visualData.useGradient)
						{
							DrawRangeColor("Start Color", emitterData.visualData.startColor);
							DrawRangeColor("End Color", emitterData.visualData.endColor);
						}
						else
						{
							DrawGradient(i, &emitterData.visualData.gradientState, &emitterData.visualData.gradientTempState);
						}
						
						DrawCheckbox("Enable Fade In", emitterData.visualData.enableFadeIn);
						if (emitterData.visualData.enableFadeIn)
						{
							DrawRangeFloat("Fade In Time", emitterData.visualData.fadeInTime, 0.1f, 0.0f, emitterData.motionData.lifeTime.max);
						}
						DrawCheckbox("Enable Fade Out", emitterData.visualData.enableFadeOut);
						if (emitterData.visualData.enableFadeOut)
						{
							DrawRangeFloat("Fade Out Time", emitterData.visualData.fadeOutTime, 0.1f, 0.0f, emitterData.motionData.lifeTime.max);
						}
						ImGui::TreePop();
					}

					// エミッタ削除ボタン
					if (ImGui::Button("Remove"))
					{
						emitterDataList.erase(emitterDataList.begin() + i);
						ImGui::TreePop();
						ImGui::PopID();
						break;
					}
					ImGui::TreePop();
				}
				ImGui::PopID();
				ImGui::Separator();
			}
			ImGui::EndChild();
			if (ImGui::Button("+", ImVec2(25, 25)))
			{
				auto& data = emitterDataList.emplace_back();
				data.name = "Emitter" + std::to_string(emitterDataList.size() - 1);
			}
			ImGui::SameLine();
			if (ImGui::Button("Clear Emitters"))
			{
				emitterDataList.clear();
			}

		}

		//ImGui::EndTable();

		ImGui::End();
	}
#endif // USE_IMGUI
}

#ifdef USE_IMGUI

bool EffectEditor::DrawRangeInt(const char* label, ::Range<int>& range, int speed, int min, int max)
{
	IMGUI_PROPERTY_BEGIN();
	ImGui::PushID(label); // 一意のIDをプッシュ
	
	IMGUI_PROPERTY(label);

	bool changed = false;
	bool edited = false;
	static int prevMin = range.min;
	static int prevMax = range.max;
	ImGui::PushItemWidth(70);

	changed |= ImGui::DragInt("##Min", &range.min, static_cast<float>(speed), min, range.max, "Min:%d");
	if (ImGui::IsItemActivated()) /* 編集開始時に前回の値を保存 */
	{
		prevMin = range.min;
		prevMax = range.max;
	}
	edited |= ImGui::IsItemDeactivatedAfterEdit();
	ImGui::SameLine();
	changed |= ImGui::DragInt("##Max", &range.max, static_cast<float>(speed), range.min, max, "Max:%d");
	if (ImGui::IsItemActivated()) /* 編集開始時に前回の値を保存 */
	{
		prevMin = range.min;
		prevMax = range.max;
	}
	edited |= ImGui::IsItemDeactivatedAfterEdit();
	ImGui::PopItemWidth();
	ImGui::PopID();

	if (edited)
	{
		// 整合性チェック
		if (range.min > range.max)
		{
			if (range.min != prevMin)
				range.max = range.min;
			else
				range.min = range.max;
			changed = true;
		}

		int newMin = range.min;
		int newMax = range.max;
		std::string oldValueStr = "Min:" + std::to_string(prevMin) + " Max:" + std::to_string(prevMax);
		std::string newValueStr = "Min:" + std::to_string(newMin) + " Max:" + std::to_string(newMax);
		auto setter = [&, label](const Range<int>& value) { range = value; };
		Range<int> oldValue = { prevMin, prevMax };
		Range<int> newValue = { newMin, newMax };
		
		// 変更前後の値をクランプしてコマンド登録
		IMGUI_PROPERTY_COMMAND_CUSTOM(label, newValue, oldValue, newValueStr, oldValueStr, setter);
	}

	IMGUI_PROPERTY_END();
	return changed;
}

bool EffectEditor::DrawRangeUInt(const char* label, ::Range<unsigned int>& range, unsigned int speed, unsigned int min, unsigned int max)
{
	IMGUI_PROPERTY_BEGIN();
	ImGui::PushID(label);
	bool changed = false;
	bool edited = false;
	static uint32_t prevMin = range.min;
	static uint32_t prevMax = range.max;

	IMGUI_PROPERTY(label);
	ImGui::PushItemWidth(70);

	changed |= ImGui::DragScalar("##Min", ImGuiDataType_U32, &range.min, static_cast<float>(speed), &min, &range.max, "Min:%u");
	if (ImGui::IsItemActivated()) /* 編集開始時に前回の値を保存 */
	{
		prevMin = range.min;
		prevMax = range.max;
	}
	edited |= ImGui::IsItemDeactivatedAfterEdit();
	ImGui::SameLine();
	changed |= ImGui::DragScalar("##Max", ImGuiDataType_U32, &range.max, static_cast<float>(speed), &range.min, &max, "Max:%u");
	if (ImGui::IsItemActivated()) /* 編集開始時に前回の値を保存 */
	{
		prevMin = range.min;
		prevMax = range.max;
	}
	edited |= ImGui::IsItemDeactivatedAfterEdit();

	ImGui::PopItemWidth();
	ImGui::PopID();

	// 変更前後の値をクランプしてコマンド登録
	if (edited)
	{
		// 整合性チェック
		if (range.min > range.max)
		{
			if (range.min != prevMin)
				range.max = range.min;
			else
				range.min = range.max;
			changed = true;
		}

		uint32_t newMin = range.min;
		uint32_t newMax = range.max;
		std::string oldValueStr = "Min:" + std::to_string(prevMin) + " Max:" + std::to_string(prevMax);
		std::string newValueStr = "Min:" + std::to_string(newMin) + " Max:" + std::to_string(newMax);
		Range<unsigned int> oldValue = { prevMin, prevMax };
		Range<unsigned int> newValue = { newMin, newMax };
		auto setter = [&, label](const Range<unsigned int>& value) { range = value; };
		// 変更前後の値をクランプしてコマンド登録
		IMGUI_PROPERTY_COMMAND_CUSTOM(label, newValue, oldValue, newValueStr, oldValueStr, setter);
	}

	IMGUI_PROPERTY_END();
	return changed;
}

bool EffectEditor::DrawRangeFloat(const char* label, ::Range<float>& range, float speed, float min, float max)
{
	IMGUI_PROPERTY_BEGIN();
	ImGui::PushID(label);
	bool changed = false;
	bool edited = false;

	//ImGui::Text("%s", label);
	//ImGui::SameLine();
	IMGUI_PROPERTY(label);
	ImGui::PushItemWidth(70);

#if 1
	static float prevMin = range.min;
	static float prevMax = range.max;
#endif // 0

	changed |= ImGui::DragFloat("##Min", &range.min, speed, min, range.max, "Min:%.3f");
	if (ImGui::IsItemActivated()) /* 編集開始時に前回の値を保存 */
	{
		prevMin = range.min;
		prevMax = range.max;
	}
	edited |= ImGui::IsItemDeactivatedAfterEdit();
	ImGui::SameLine();
	changed |= ImGui::DragFloat("##Max", &range.max, speed, range.min, max, "Max:%.3f");
	if (ImGui::IsItemActivated()) /* 編集開始時に前回の値を保存 */
	{
		prevMin = range.min;
		prevMax = range.max;
	}
	edited |= ImGui::IsItemDeactivatedAfterEdit();

	ImGui::PopItemWidth();
	ImGui::PopID();

	if (edited)
	{
#if 1
		// 整合性チェック
		if (range.min > range.max)
		{
			if (range.min != prevMin)
				range.max = range.min;
			else
				range.min = range.max;
			changed = true;
		}

		// Clamp不要判定
		float epsilon = 1e-6f;
		if (fabsf(min) < epsilon && fabsf(max) < epsilon)
		{
			//return changed; // Clamp不要
		}
		else
		{
			// 範囲制限
			range.min = std::clamp(range.min, min, max);
			range.max = std::clamp(range.max, min, max);
		}
#endif // 0

		float newMin = range.min;
		float newMax = range.max;
		std::string oldValueStr = "Min:" + std::to_string(prevMin) + " Max:" + std::to_string(prevMax);
		std::string newValueStr = "Min:" + std::to_string(newMin) + " Max:" + std::to_string(newMax);
		Range<float> oldValue = { prevMin, prevMax };
		Range<float> newValue = { newMin, newMax };
		auto setter = [&, label](const Range<float>& value) { range = value; };

		// 変更前後の値をクランプしてコマンド登録
		IMGUI_PROPERTY_COMMAND_CUSTOM(label, newValue, oldValue, newValueStr, oldValueStr, setter);
	}

	IMGUI_PROPERTY_END();
	return changed;
}

bool EffectEditor::DrawRangeVector2(const char* label, ::Range<Vector2>& range, float speed, float min, float max)
{
	bool changed = false;
	bool edited = false;
	ImGui::PushID(label); // 一意のIDをプッシュ

	static Vector2 prevMin = range.min;
	static Vector2 prevMax = range.max;

	if (ImGui::TreeNodeEx(label, ImGuiTreeNodeFlags_DefaultOpen))
	{
		IMGUI_PROPERTY_BEGIN();
		ImGui::PushItemWidth(140);
		// Minベクトル編集
		//ImGui::Text("Min");
		//ImGui::SameLine();
		IMGUI_PROPERTY("Min");
		changed |= ImGui::DragFloat2("##Min", reinterpret_cast<float*>(&range.min), speed, min, max, "%.3f");
		if (ImGui::IsItemActivated()) /* 編集開始時に前回の値を保存 */
		{
			prevMin = range.min;
			prevMax = range.max;
		}
		edited |= ImGui::IsItemDeactivatedAfterEdit();
		// Maxベクトル編集
		//ImGui::Text("Max");
		//ImGui::SameLine();
		IMGUI_PROPERTY("Max");
		changed |= ImGui::DragFloat2("##Max", reinterpret_cast<float*>(&range.max), speed, min, max, "%.3f");
		if (ImGui::IsItemActivated()) /* 編集開始時に前回の値を保存 */
		{
			prevMin = range.min;
			prevMax = range.max;
		}
		edited |= ImGui::IsItemDeactivatedAfterEdit();

		ImGui::PopItemWidth();

		IMGUI_PROPERTY_END();
		ImGui::TreePop();
	}
	ImGui::PopID();

#if 0
	// 整合性チェック：軸ごと
	for (int i = 0; i < 2; ++i)
	{
		float* minPtr = (&range.min.x) + i;
		float* maxPtr = (&range.max.x) + i;
		float* prevMinPtr = (&prevMin.x) + i;

		if (*minPtr > *maxPtr)
		{
			if (*minPtr != *prevMinPtr)
				*maxPtr = *minPtr;
			else
				*minPtr = *maxPtr;
			changed = true;
		}

		float epsilon = 1e-6f;
		if (fabsf(min) < epsilon && fabsf(max) < epsilon)
			continue; // Clamp不要

		// Clamp
		*minPtr = std::clamp(*minPtr, min, max);
		*maxPtr = std::clamp(*maxPtr, min, max);
	}
#endif // 0

	if (edited)
	{
		Vector2 newMin = range.min;
		Vector2 newMax = range.max;
		std::string oldValueStr = "Min:(" + std::to_string(prevMin.x) + "," + std::to_string(prevMin.y) + ") Max:(" + std::to_string(prevMax.x) + "," + std::to_string(prevMax.y) + ")";
		std::string newValueStr = "Min:(" + std::to_string(newMin.x) + "," + std::to_string(newMin.y) + ") Max:(" + std::to_string(newMax.x) + "," + std::to_string(newMax.y) + ")";
		Range<Vector2> oldValue = { prevMin, prevMax };
		Range<Vector2> newValue = { newMin, newMax };
		auto setter = [&, label](const Range<Vector2>& value) { range = value; };
		// 変更前後の値をクランプしてコマンド登録
		IMGUI_PROPERTY_COMMAND_CUSTOM(label, newValue, oldValue, newValueStr, oldValueStr, setter);
	}
	
	return changed;
}

bool EffectEditor::DrawRangeVector3(const char* label, ::Range<Vector3>& range, float speed, float min, float max)
{
	
	bool changed = false;
	bool edited = false;
	ImGui::PushID(label); // 一意のIDをプッシュ
	
	static Vector3 prevMin = range.min;
	static Vector3 prevMax = range.max;

	if (ImGui::TreeNodeEx(label, ImGuiTreeNodeFlags_DefaultOpen))
	{
		IMGUI_PROPERTY_BEGIN();
		ImGui::PushItemWidth(210);
		// Minベクトル編集
		//ImGui::Text("Min");
		//ImGui::SameLine();
		IMGUI_PROPERTY("Min");
		changed |= ImGui::DragFloat3("##Min", reinterpret_cast<float*>(&range.min), speed, min, max, "%.3f");
		if (ImGui::IsItemActivated()) /* 編集開始時に前回の値を保存 */
		{
			prevMin = range.min;
			prevMax = range.max;
		}
		edited |= ImGui::IsItemDeactivatedAfterEdit();
		// Maxベクトル編集
		//ImGui::Text("Max");
		//ImGui::SameLine();
		IMGUI_PROPERTY("Max");
		changed |= ImGui::DragFloat3("##Max", reinterpret_cast<float*>(&range.max), speed, min, max, "%.3f");
		if (ImGui::IsItemActivated()) /* 編集開始時に前回の値を保存 */
		{
			prevMin = range.min;
			prevMax = range.max;
		}
		edited |= ImGui::IsItemDeactivatedAfterEdit();

		ImGui::PopItemWidth();
		IMGUI_PROPERTY_END();
		ImGui::TreePop();
	}
	ImGui::PopID();

#if 0
	// 整合性チェック：軸ごと
	for (int i = 0; i < 3; ++i)
	{
		float* minPtr = (&range.min.x) + i;
		float* maxPtr = (&range.max.x) + i;
		float* prevMinPtr = (&prevMin.x) + i;

		if (*minPtr > *maxPtr)
		{
			if (*minPtr != *prevMinPtr)
				*maxPtr = *minPtr;
			else
				*minPtr = *maxPtr;
			changed = true;
		}

		float epsilon = 1e-6f;
		if (fabsf(min) < epsilon && fabsf(max) < epsilon)
			continue; // Clamp不要

		// Clamp
		*minPtr = std::clamp(*minPtr, min, max);
		*maxPtr = std::clamp(*maxPtr, min, max);
	}
#endif // 0

	if (edited)
	{
		Vector3 newMin = range.min;
		Vector3 newMax = range.max;
		std::string oldValueStr = "Min:(" + std::to_string(prevMin.x) + "," + std::to_string(prevMin.y) + "," + std::to_string(prevMin.z) + ") Max:(" + std::to_string(prevMax.x) + "," + std::to_string(prevMax.y) + "," + std::to_string(prevMax.z) + ")";
		std::string newValueStr = "Min:(" + std::to_string(newMin.x) + "," + std::to_string(newMin.y) + "," + std::to_string(newMin.z) + ") Max:(" + std::to_string(newMax.x) + "," + std::to_string(newMax.y) + "," + std::to_string(newMax.z) + ")";
		Range<Vector3> oldValue = { prevMin, prevMax };
		Range<Vector3> newValue = { newMin, newMax };
		auto setter = [&, label](const Range<Vector3>& value) { range = value; };
		// 変更前後の値をクランプしてコマンド登録
		IMGUI_PROPERTY_COMMAND_CUSTOM(label, newValue, oldValue, newValueStr, oldValueStr, setter);
	}

	return changed;
}

bool EffectEditor::DrawRangeColor(const char* label, ::Range<Color>& range)
{
	bool changed = false;
	bool edited = false;
	ImGui::PushID(label); // 一意のIDをプッシュ

	static Color prevMin = range.min;
	static Color prevMax = range.max;

	if (ImGui::TreeNodeEx(label, ImGuiTreeNodeFlags_DefaultOpen))
	{
		IMGUI_PROPERTY_BEGIN();
		ImGui::PushItemWidth(200);
		// Minカラー編集
		//ImGui::Text("Min");
		//ImGui::SameLine();
		IMGUI_PROPERTY("Min");
		changed |= ImGui::ColorEdit4("##Min", &range.min.r);
		if (ImGui::IsItemActivated()) /* 編集開始時に前回の値を保存 */
		{
			prevMin = range.min;
			prevMax = range.max;
		}
		edited |= ImGui::IsItemDeactivatedAfterEdit();
		// Maxカラー編集
		//ImGui::Text("Max");
		//ImGui::SameLine();
		IMGUI_PROPERTY("Max");
		changed |= ImGui::ColorEdit4("##Max", &range.max.r);
		if (ImGui::IsItemActivated()) /* 編集開始時に前回の値を保存 */
		{
			prevMin = range.min;
			prevMax = range.max;
		}
		edited |= ImGui::IsItemDeactivatedAfterEdit();

		ImGui::PopItemWidth();
		IMGUI_PROPERTY_END();
		ImGui::TreePop();
	}
	ImGui::PopID();

	if (edited)
	{
		Range<Color> oldValue = { prevMin, prevMax };
		Range<Color> newValue = { range.min, range.max };
		std::string oldValueStr = "Min:(" + std::to_string(prevMin.r) + "," + std::to_string(prevMin.g) + "," + std::to_string(prevMin.b) + "," + std::to_string(prevMin.a)
			+ ") Max:(" + std::to_string(prevMax.r) + "," + std::to_string(prevMax.g) + "," + std::to_string(prevMax.b) + "," + std::to_string(prevMax.a) + ")";
		std::string newValueStr = "Min:(" + std::to_string(newValue.min.r) + "," + std::to_string(newValue.min.g) + "," + std::to_string(newValue.min.b) + "," + std::to_string(newValue.min.a)
			+ ") Max:(" + std::to_string(newValue.max.r) + "," + std::to_string(newValue.max.g) + "," + std::to_string(newValue.max.b) + "," + std::to_string(newValue.max.a) + ")";
		
		auto setter = [&, label](const Range<Color>& value) { range = value; };

		// 変更前後の値をクランプしてコマンド登録
		IMGUI_PROPERTY_COMMAND_CUSTOM(label, newValue, oldValue, newValueStr, oldValueStr, setter);
	}

	
	return changed;
}

bool EffectEditor::DrawInt(const char* label, int& value, int speed, int min, int max)
{
	IMGUI_PROPERTY_BEGIN();
	ImGui::PushID(label); // 一意のIDをプッシュ
	bool changed = false;
	ImGui::PushItemWidth(200);
	static int prevValue = value;
	//ImGui::Text("%s", label);
	//ImGui::SameLine();
	IMGUI_PROPERTY(label);
	changed |= ImGui::DragInt("##Value", &value, static_cast<float>(speed), min, max);
	if (ImGui::IsItemActivated()) /* 編集開始時に前回の値を保存 */
	{
		prevValue = value;
	}
	ImGui::PopItemWidth();
	ImGui::PopID();
	if (ImGui::IsItemDeactivatedAfterEdit())
	{
		int newValue = value;
		std::string oldValueStr = std::to_string(prevValue);
		std::string newValueStr = std::to_string(newValue);
		auto setter = [&, label](int v) { value = v; };
		// 変更前後の値をクランプしてコマンド登録
		IMGUI_PROPERTY_COMMAND_CUSTOM(label, newValue, prevValue, newValueStr, oldValueStr, setter);
	}
	IMGUI_PROPERTY_END();
	return changed;
}

bool EffectEditor::DrawUInt(const char* label, unsigned int& value, unsigned int speed, unsigned int min, unsigned int max)
{
	IMGUI_PROPERTY_BEGIN();
	ImGui::PushID(label); // 一意のIDをプッシュ
	bool changed = false;
	ImGui::PushItemWidth(200);
	static uint32_t prevValue = value;
	//ImGui::Text("%s", label);
	//ImGui::SameLine();
	IMGUI_PROPERTY(label);
	changed |= ImGui::DragScalar("##Value", ImGuiDataType_U32, &value, static_cast<float>(speed), &min, &max);
	if (ImGui::IsItemActivated()) /* 編集開始時に前回の値を保存 */
	{
		prevValue = value;
	}
	ImGui::PopItemWidth();
	ImGui::PopID();
	if (ImGui::IsItemDeactivatedAfterEdit())
	{
		uint32_t newValue = value;
		std::string oldValueStr = std::to_string(prevValue);
		std::string newValueStr = std::to_string(newValue);
		auto setter = [&, label](uint32_t v) { value = v; };
		// 変更前後の値をクランプしてコマンド登録
		IMGUI_PROPERTY_COMMAND_CUSTOM(label, newValue, prevValue, newValueStr, oldValueStr, setter);
	}
	IMGUI_PROPERTY_END();
	return changed;
}

bool EffectEditor::DrawFloat(const char* label, float& value, float speed, float min, float max)
{
	IMGUI_PROPERTY_BEGIN();
	ImGui::PushID(label); // 一意のIDをプッシュ
	bool changed = false;
	ImGui::PushItemWidth(200);

	static float prevValue = value;

	//ImGui::Text("%s", label);
	//ImGui::SameLine();
	IMGUI_PROPERTY(label);
	changed |= ImGui::DragFloat("##Value", &value, speed, min, max, "%.3f");
	if (ImGui::IsItemActivated()) /* 編集開始時に前回の値を保存 */
	{
		prevValue = value;
	}
	ImGui::PopItemWidth();
	ImGui::PopID();

	if (ImGui::IsItemDeactivatedAfterEdit())
	{
		float newValue = value;
		std::string oldValueStr = std::to_string(prevValue);
		std::string newValueStr = std::to_string(newValue);
		auto setter = [&, label](float v) { value = v; };
		// 変更前後の値をクランプしてコマンド登録
		IMGUI_PROPERTY_COMMAND_CUSTOM(label, newValue, prevValue, newValueStr, oldValueStr, setter);
	}

	IMGUI_PROPERTY_END();
	return changed;
}

bool EffectEditor::DrawVector2(const char* label, Vector2& value, float speed, float min, float max)
{
	IMGUI_PROPERTY_BEGIN();
	ImGui::PushID(label); // 一意のIDをプッシュ
	bool changed = false;
	ImGui::PushItemWidth(200);
	static Vector2 prevValue = value;
	//ImGui::Text("%s", label);
	//ImGui::SameLine();
	IMGUI_PROPERTY(label);
	changed |= ImGui::DragFloat2("##Value", reinterpret_cast<float*>(&value), speed, min, max, "%.3f");
	if (ImGui::IsItemActivated()) /* 編集開始時に前回の値を保存 */
	{
		prevValue = value;
	}
	ImGui::PopItemWidth();
	ImGui::PopID();
	if (ImGui::IsItemDeactivatedAfterEdit())
	{
		Vector2 newValue = value;
		std::string oldValueStr = "(" + std::to_string(prevValue.x) + "," + std::to_string(prevValue.y) + ")";
		std::string newValueStr = "(" + std::to_string(newValue.x) + "," + std::to_string(newValue.y) + ")";
		auto setter = [&, label](const Vector2& v) { value = v; };
		// 変更前後の値をクランプしてコマンド登録
		IMGUI_PROPERTY_COMMAND_CUSTOM(label, newValue, prevValue, newValueStr, oldValueStr, setter);
	}
	IMGUI_PROPERTY_END();
	return changed;
}


bool EffectEditor::DrawVector3(const char* label, Vector3& value, float speed, float min, float max)
{
	IMGUI_PROPERTY_BEGIN();
	ImGui::PushID(label); // 一意のIDをプッシュ
	bool changed = false;
	ImGui::PushItemWidth(210);

	static Vector3 prevValue = value;

	//ImGui::Text("%s", label);
	//ImGui::SameLine();
	IMGUI_PROPERTY(label);
	changed |= ImGui::DragFloat3("##Value", reinterpret_cast<float*>(&value), speed, min, max, "%.3f");
	if (ImGui::IsItemActivated()) /* 編集開始時に前回の値を保存 */
	{
		prevValue = value;
	}
	ImGui::PopItemWidth();
	ImGui::PopID();

	if (ImGui::IsItemDeactivatedAfterEdit())
	{
		Vector3 newValue = value;
		std::string oldValueStr = "(" + std::to_string(prevValue.x) + "," + std::to_string(prevValue.y) + "," + std::to_string(prevValue.z) + ")";
		std::string newValueStr = "(" + std::to_string(newValue.x) + "," + std::to_string(newValue.y) + "," + std::to_string(newValue.z) + ")";
		auto setter = [&, label](const Vector3& v) { value = v; };
		// 変更前後の値をクランプしてコマンド登録
		IMGUI_PROPERTY_COMMAND_CUSTOM(label, newValue, prevValue, newValueStr, oldValueStr, setter);
	}

	IMGUI_PROPERTY_END();
	return changed;
}

bool EffectEditor::DrawColor(const char* label, Color& value)
{
	IMGUI_PROPERTY_BEGIN();
	ImGui::PushID(label); // 一意のIDをプッシュ
	bool changed = false;
	static Color prevValue = value;
	//ImGui::Text("%s", label);
	//ImGui::SameLine();
	IMGUI_PROPERTY(label);
	changed |= ImGui::ColorEdit4("##Value", &value.r);
	if (ImGui::IsItemActivated()) /* 編集開始時に前回の値を保存 */
	{
		prevValue = value;
	}
	ImGui::PopID();
	if (ImGui::IsItemDeactivatedAfterEdit())
	{
		Color newValue = value;
		std::string oldValueStr = "(" + std::to_string(prevValue.r) + "," + std::to_string(prevValue.g) + "," + std::to_string(prevValue.b) + "," + std::to_string(prevValue.a) + ")";
		std::string newValueStr = "(" + std::to_string(newValue.r) + "," + std::to_string(newValue.g) + "," + std::to_string(newValue.b) + "," + std::to_string(newValue.a) + ")";
		Color oldValueClamped = Color(std::clamp(prevValue.r, 0.0f, 1.0f), std::clamp(prevValue.g, 0.0f, 1.0f), std::clamp(prevValue.b, 0.0f, 1.0f), std::clamp(prevValue.a, 0.0f, 1.0f));
		Color newValueClamped = Color(std::clamp(newValue.r, 0.0f, 1.0f), std::clamp(newValue.g, 0.0f, 1.0f), std::clamp(newValue.b, 0.0f, 1.0f), std::clamp(newValue.a, 0.0f, 1.0f));
		auto setter = [&, label](const Color& v) { value = v; };
		// 変更前後の値をクランプしてコマンド登録
		IMGUI_PROPERTY_COMMAND_CUSTOM(label, newValueClamped, oldValueClamped, newValueStr, oldValueStr, setter);
	}
	IMGUI_PROPERTY_END();
	return changed;
}

bool EffectEditor::DrawCheckbox(const char* label, bool& value)
{
	IMGUI_PROPERTY_BEGIN();
	ImGui::PushID(label); // 一意のIDをプッシュ
	bool changed = false;
	static bool prevValue = value;
	IMGUI_PROPERTY(label);
	changed |= ImGui::Checkbox("##Checkbox", &value);
	if (ImGui::IsItemActivated()) /* 編集開始時に前回の値を保存 */
	{
		prevValue = value;
	}
	ImGui::PopID();
	if (ImGui::IsItemDeactivatedAfterEdit())
	{
		bool newValue = value;
		std::string oldValueStr = prevValue ? "True" : "False";
		std::string newValueStr = value ? "True" : "False";
		auto setter = [&, label](bool v) { value = v; };
		// コマンド登録
		IMGUI_PROPERTY_COMMAND_CUSTOM(label, newValue, prevValue, newValueStr, oldValueStr, setter);
	}
	IMGUI_PROPERTY_END();
	return changed;
}

bool EffectEditor::DrawCombo(const char* label, int& currentIndex, const char* const items[], int itemCount, std::function<void(int)> setter)
{
	IMGUI_PROPERTY_BEGIN();
	ImGui::PushID(label); // 一意のIDをプッシュ
	bool changed = false;
	static int prevIndex = currentIndex;
	IMGUI_PROPERTY(label);
	changed |= ImGui::Combo("##Combo", &currentIndex, items, itemCount);
	if (ImGui::IsItemActivated()) /* 編集開始時に前回の値を保存 */
	{
		prevIndex = currentIndex;
	}
	ImGui::PopID();
	if (changed)
	{
		int newIndex = currentIndex;
		std::string oldValueStr = items[prevIndex];
		std::string newValueStr = items[newIndex];
		// コマンド登録
		IMGUI_PROPERTY_COMMAND_CUSTOM(label, newIndex, prevIndex, newValueStr, oldValueStr, setter);
	}
	IMGUI_PROPERTY_END();
	return changed;
}

bool EffectEditor::DrawGradient(uint32_t gradientId, ImGradientHDRState* state, ImGradientHDRTemporaryState* tempState)
{
	// グラデーションエディタの描画
	bool isMarkerShown = true;
	ImGradientHDR(gradientId, *state, *tempState, isMarkerShown);

	if (ImGui::IsItemHovered())
	{
		ImGui::SetTooltip("Gradient Editor");
	}

	if (tempState->selectedMarkerType == ImGradientHDRMarkerType::Color)
	{
		auto selectedColorMarker = state->GetColorMarker(tempState->selectedIndex);
		if (selectedColorMarker)
		{
			// Color編集
			{
				IMGUI_PROPERTY_BEGIN();
				bool changed = false;
				static std::array<float, 3> prevColor = { 1,1,1 };
				IMGUI_PROPERTY("Color");
				changed |= ImGui::ColorEdit3("##Color", selectedColorMarker->Color.data(), ImGuiColorEditFlags_Float);
				if (ImGui::IsItemActivated()) /* 編集開始時に前回の値を保存 */
				{
					prevColor = selectedColorMarker->Color;
				}
				if (ImGui::IsItemDeactivatedAfterEdit())
				{
					std::array<float, 3> newColor = selectedColorMarker->Color;
					std::string oldValueStr = "(" + std::to_string(prevColor[0]) + "," + std::to_string(prevColor[1]) + "," + std::to_string(prevColor[2]) + ")";
					std::string newValueStr = "(" + std::to_string(newColor[0]) + "," + std::to_string(newColor[1]) + "," + std::to_string(newColor[2]) + ")";
					auto setter = [state, tempState](const std::array<float, 3>& color)
						{ 
							auto* selectedColorMarker = state->GetColorMarker(tempState->selectedIndex);
							selectedColorMarker->Color = color;

						};
					// コマンド登録
					IMGUI_PROPERTY_COMMAND_CUSTOM("Color", newColor, prevColor, newValueStr, oldValueStr, setter);
				}
				IMGUI_PROPERTY_END();
			}

			// Intensity編集
			DrawFloat("Intensity", selectedColorMarker->Intensity, 0.1f, 0.0f, 100.0f);
		}
	}

	if (tempState->selectedMarkerType == ImGradientHDRMarkerType::Alpha)
	{
		auto selectedAlphaMarker = state->GetAlphaMarker(tempState->selectedIndex);
		if (selectedAlphaMarker)
		{
			// Alpha編集
			DrawFloat("Alpha", selectedAlphaMarker->Alpha, 0.1f, 0.0f, 1.0f);
		}
	}

	if (tempState->selectedMarkerType != ImGradientHDRMarkerType::Unknown)
	{
		if (ImGui::Button("Delete"))
		{
			if (tempState->selectedMarkerType == ImGradientHDRMarkerType::Color)
			{
				state->RemoveColorMarker(tempState->selectedIndex);
				tempState->selectedMarkerType = ImGradientHDRMarkerType::Unknown; // 選択状態をリセット
				tempState->selectedIndex = -1; // コマンド登録は不要（削除後は存在しないインデックスになるため）
			}
			else if (tempState->selectedMarkerType == ImGradientHDRMarkerType::Alpha)
			{
				state->RemoveAlphaMarker(tempState->selectedIndex);
				tempState->selectedMarkerType = ImGradientHDRMarkerType::Unknown; // 選択状態をリセット
				tempState->selectedIndex = -1; // コマンド登録は不要（削除後は存在しないインデックスになるため）
			}
		}
	}

	return false;
}


#endif // USE_IMGUI
