#pragma once
#include <vector>
#include <filesystem>
/** @brief Node を表す構造体です。 */
struct Node
{
	int id;
	float value;
	Node(const int id, const float value) : id(id), value(value) {}
};

/** @brief Link を表す構造体です。 */
struct Link
{
	int id;
	int startAttrId;
	int endAttrId;
	Link(const int id, const int startAttrId, const int endAttrId) : id(id), startAttrId(startAttrId), endAttrId(endAttrId) {}
};

/** @brief Pin を表す構造体です。 */
struct Pin
{
	int id;
	int nodeId;
	Pin(const int id, const int nodeId) : id(id), nodeId(nodeId) {}
};

/** @brief NodeEditorState を表す構造体です。 */
struct NodeEditorState
{
	struct ImNodesEditorContext* context = nullptr;
	std::vector<Node> nodes;
	std::vector<Link> links;
	std::vector<Pin> pins;
	int currentId = 0;
};


/** @brief MaterialNodeEditor を表すクラスです。 */
class MaterialNodeEditor
{
public:
	/**
	 * @brief MaterialNodeEditor を構築します。
	 */
	MaterialNodeEditor();
	/**
	 * @brief MaterialNodeEditor を破棄します。
	 */
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

	/**
	 * @brief NodeEditorInitialize の処理を行います。
	 */
	static void NodeEditorInitialize();

	/**
	 * @brief NodeEditorShutdown の処理を行います。
	 */
	static void NodeEditorShutdown();

#ifdef USE_IMGUI

	/**
	 * @brief 描画処理を行います。
	 */
	static void DrawNodeEditor(NodeEditorState& state);
#endif // USE_IMGUI


	static inline NodeEditorState s_nodeEditorState; ///< ノードエディタの状態を保持する構造体

	static inline bool s_isOpen; ///< エディタが開いているかどうかのフラグ
};
