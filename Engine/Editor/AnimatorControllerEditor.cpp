#include "pch.h"
#include "AnimatorControllerEditor.h"
#include "Engine/Resources/AssetDatabase.h"
#include <imnodes.h>

AnimatorControllerEditor::AnimatorControllerEditor()
{
#ifdef USE_IMGUI
	NodeEditorInitialize();
#endif // USE_IMGUI
}

AnimatorControllerEditor::~AnimatorControllerEditor()
{
#ifdef USE_IMGUI
	NodeEditorShutdown();
#endif // USE_IMGUI
}

void AnimatorControllerEditor::Open()
{
	s_isOpen = true;
#ifdef USE_IMGUI
	if (ImNodes::GetCurrentContext() == nullptr)
	{
		NodeEditorInitialize();
	}
#endif // USE_IMGUI

}

void AnimatorControllerEditor::Close()
{
	s_isOpen = false;
#ifdef USE_IMGUI
	if (ImNodes::GetCurrentContext() != nullptr)
	{
		NodeEditorShutdown();
	}
#endif // USE_IMGUI

}

bool AnimatorControllerEditor::IsOpen()
{
	return s_isOpen;
}

void AnimatorControllerEditor::OpenAsset(const std::filesystem::path& path)
{
	auto meta = CurryEngine::Resources::AssetDatabase::GetOrImport(path);
	if (!meta)
	{
		LOG_ERROR("Failed to get or import asset meta: " + path.string());
		return;
	}

	// アセットの種類がAnimatorControllerであることを確認
	if (meta->type != AssetType::AnimatorController)
	{
		LOG_ERROR("Asset is not an AnimatorController: " + path.string());
		return;
	}

	// アセットIDを使用してAnimatorControllerをロード
	auto animatorController = CurryEngine::Resources::AssetDatabase::LoadAsset<AnimatorController>(meta->id);
	if (!animatorController)
	{
		LOG_ERROR("Failed to load AnimatorController asset: " + path.string());
		return;
	}

	// エディタを開き、ロードしたAnimatorControllerを設定
	s_animatorController = animatorController;
	Open();
}

namespace 
{
	constexpr int kAnyStateNodeId = -1; // "Any State" ノードのIDを定義
	constexpr int kAnyStateOutputPinId = -2; // "Any State" ノードの出力ピンIDを定義

	// stateインデックスからノードID、入力ピンID、出力ピンIDを計算する関数
	int NodeIdFromStateIndex(int stateIndex) {
		return stateIndex * 3; // ノードIDはstateインデックスx3(3の倍数)で割り当てる
	}

	int InputPinId(int stateIndex) {
		return stateIndex * 3 + 1; // 入力ピンのIDはstateインデックスx3+1(3の倍数+1)で割り当てる
	}

	int OutputPinId(int stateIndex) {
		return stateIndex * 3 + 2; // 出力ピンのIDはstateインデックスx3+2(3の倍数+2)で割り当てる
	}

	int StateIndexFromNodeId(int nodeId) {
		return nodeId / 3; // ノードIDからstateインデックスを計算する
	}

	int StateIndexFromInputPinId(int pinId) {
		return (pinId - 1) / 3; // 入力ピンIDからstateインデックスを計算する
	}

	int StateIndexFromOutputPinId(int pinId) {
		return (pinId - 2) / 3; // 出力ピンIDからstateインデックスを計算する
	}


	// Link ID = transitions のインデックスそのまま使用する（Linkは別プールなのでNode/Pinと衝突しない）
}

void AnimatorControllerEditor::DrawGUI()
{
#ifdef USE_IMGUI
	if (!s_isOpen) return;

	DrawNodeEditor(s_nodeEditorState);

#endif // USE_IMGUI

}

#ifdef USE_IMGUI

void AnimatorControllerEditor::NodeEditorInitialize()
{
	// ImNodesのコンテキストを作成
	auto context = ImNodes::CreateContext();

	s_nodeEditorState.context = ImNodes::EditorContextCreate();
	ImNodes::PushAttributeFlag(ImNodesAttributeFlags_EnableLinkDetachWithDragClick);

	ImNodesIO& io = ImNodes::GetIO();
	io.LinkDetachWithModifierClick.Modifier = &ImGui::GetIO().KeyCtrl;
	io.MultipleSelectModifier.Modifier = &ImGui::GetIO().KeyCtrl;

	ImNodesStyle& style = ImNodes::GetStyle();
	style.Flags |= ImNodesStyleFlags_GridLinesPrimary | ImNodesStyleFlags_GridSnapping;

}

void AnimatorControllerEditor::NodeEditorShutdown()
{
	if (s_nodeEditorState.context)
	{
		ImNodes::PopAttributeFlag();
		ImNodes::EditorContextFree(s_nodeEditorState.context);
		s_nodeEditorState.context = nullptr;
	}

	// ImNodesのコンテキストを破棄
	ImNodes::DestroyContext();
}


void AnimatorControllerEditor::DrawNodeEditor(NodeEditorState& state)
{
	ImGui::Begin("Animator Controller Editor", &s_isOpen);

	ImNodes::EditorContextSet(state.context);

	auto& controller = s_animatorController;

	if (controller)
	{
		ImNodes::BeginNodeEditor();


		if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) &&
			ImNodes::IsEditorHovered())
		{

			// 右クリックでノード追加メニューを表示
			if (ImGui::IsMouseClicked(ImGuiMouseButton_Right))
			{
				// 右クリックメニューを表示
				ImGui::OpenPopup("AddNodePopup");
			}
		}
		if (ImGui::BeginPopup("AddNodePopup"))
		{
			if (ImGui::MenuItem("Add State"))
			{
				// 新しい状態を追加
				const int newStateIndex = static_cast<int>(controller->states.size());
				ImNodes::SetNodeScreenSpacePos(NodeIdFromStateIndex(newStateIndex), ImGui::GetMousePos());
				controller->states.push_back({ "New State" });
			}
			ImGui::EndPopup();
		}

		// AnyState疑似ノード
		ImNodes::BeginNode(kAnyStateNodeId);
		ImNodes::BeginNodeTitleBar();
		ImGui::TextUnformatted("Any State");
		ImNodes::EndNodeTitleBar();

		ImNodes::BeginOutputAttribute(kAnyStateOutputPinId);

		const float nodeWidth = 125.0f;
		ImGui::SetNextItemWidth(nodeWidth);
		ImGui::Text("Output");
		
		ImNodes::EndOutputAttribute();
		ImNodes::EndNode();

		for (int i = 0; i < controller->states.size(); ++i)
		{
			auto& state = controller->states[i];
			ImNodes::BeginNode(NodeIdFromStateIndex(i));
			ImNodes::BeginNodeTitleBar(); ImGui::TextUnformatted(state.name.c_str()); ImNodes::EndNodeTitleBar();
			ImNodes::BeginInputAttribute(InputPinId(i));
			ImGui::Text("Input");
			ImNodes::EndInputAttribute();

			ImNodes::BeginOutputAttribute(OutputPinId(i));
			ImGui::Text("Output");
			ImNodes::EndOutputAttribute();
			ImNodes::EndNode();
		}

		for (int j = 0; j < controller->transitions.size(); ++j)
		{
			const auto& t = controller->transitions[j];
			int fromPin = (t.fromStateIndex == -1) ? kAnyStateOutputPinId : OutputPinId(t.fromStateIndex);
			ImNodes::Link(j, fromPin, InputPinId(t.toStateIndex));
		}

		ImNodes::EndNodeEditor();

		int startPin, endPin;
		if (ImNodes::IsLinkCreated(&startPin, &endPin))
		{
			controller->transitions.push_back({ (startPin == kAnyStateOutputPinId) ? -1 : StateIndexFromOutputPinId(startPin), 
				StateIndexFromInputPinId(endPin) });
		}

		int linkId;
		if (ImNodes::IsLinkDestroyed(&linkId))
		{
			// linkId は transitions のインデックスそのままなので、削除する
			if (linkId >= 0 && linkId < controller->transitions.size())
			{
				controller->transitions.erase(controller->transitions.begin() + linkId);
			}
		}
	}
	else
	{
		ImGui::Text("No AnimatorController loaded.");
	}

	ImGui::End();
}


#endif // USE_IMGUI
