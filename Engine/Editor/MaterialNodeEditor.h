#pragma once
#include <vector>
#include <filesystem>
struct Node
{
	int id;
	float value;
	Node(const int id, const float value) : id(id), value(value) {}
};

struct Link
{
	int id;
	int startAttrId;
	int endAttrId;
	Link(const int id, const int startAttrId, const int endAttrId) : id(id), startAttrId(startAttrId), endAttrId(endAttrId) {}
};

struct Pin
{
	int id;
	int nodeId;
	Pin(const int id, const int nodeId) : id(id), nodeId(nodeId) {}
};

struct NodeEditorState
{
	struct ImNodesEditorContext* context = nullptr;
	std::vector<Node> nodes;
	std::vector<Link> links;
	std::vector<Pin> pins;
	int currentId = 0;
};


class MaterialNodeEditor
{
public:
	MaterialNodeEditor();
	~MaterialNodeEditor();


	/** @brief エディタを開く。*/
	static void Open();

	/** @brief エディタを閉じる。*/
	static void Close();

	/** @brief エディタが開いているかどうかを取得。*/
	static bool IsOpen();

	/** @brief アセットを開く。*/
	static void OpenAsset(const std::filesystem::path& path);

	/** @brief エディタのGUIを描画。*/
	static void DrawGUI();

private:

	static void NodeEditorInitialize();

	static void NodeEditorShutdown();

#ifdef USE_IMGUI

	static void DrawNodeEditor(NodeEditorState& state);
#endif // USE_IMGUI


	static inline NodeEditorState s_nodeEditorState; ///< ノードエディタの状態を保持する構造体

	static inline bool s_isOpen; ///< エディタが開いているかどうかのフラグ
};
