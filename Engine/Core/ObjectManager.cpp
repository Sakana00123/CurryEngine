#include "pch.h"
#include "ObjectManager.h"
#include "PersistentObjectManager.h"
#include "GameObject.h"
#include "Engine/Scenes/SceneManager.h"
#ifdef USE_IMGUI
#include <imgui.h>
#include <ImGuizmo.h>
#include "Engine/Editor/AssetBrowser.h"
#include "Engine/Editor/EditorGUI.h"

#endif // USE_IMGUI

#include <functional>

#include "Engine/Input/InputSystem.h"
#include "Engine/Rendering/Pipeline/Graphics.h"
#include "Engine/UI/RectTransform.h"

#include "Engine/Utils/JsonFileHandler.h"
#include <profiler.h>

#include "ScriptComponent.h"
#include "Engine/Rendering/Camera/EditorCamera.h"
#include "Engine/Utils/UniqueIdGenerator.h"
#include "Engine/Factory/GameObjectFactory.h"
#include "Engine/Core/Layer.h"
#include "Engine/Scenes/SceneMigrator.h"
#include "Engine/EditorSupport/EditorSelection.h"
#include "Engine/EditorSupport/SetValueCommand.h"
#include "Engine/EditorSupport/CompoundCommand.h"

#include "Engine/Editor/InspectorWindow.h"

#include "Engine/EditorSupport/OrderManager.h"
#include <imgui_internal.h>
#include <Engine\Rendering\Renderers\GltfModelRenderer.h>
#include "Engine/Editor/Dialog.h"
#include "Engine/UI/Mask.h"

#include "Engine/EditorConfig/EditorConfigManager.h"
#include "Engine/EditorConfig/IEditorConfig.h"
#include "Engine/EditorConfig/SceneViewConfig.h"


ObjectManager::ObjectManager(Scene* scene) : scene(scene), selectNode(nullptr), inspectorNode(nullptr)
{
	// 選択管理の初期化
	selection = new EditorSelection();
}

ObjectManager::~ObjectManager()
{
	// 選択管理オブジェクトの削除
	if (selection) {
		delete selection;
		selection = nullptr;
	}


	// すべてのオブジェクトを削除
	objects.clear();
}

void ObjectManager::BeginFrame()
{
	std::function<void(GameObject*)> BeginFrame = [&](GameObject* object)
		{
			object->BeginFrame();
			// 子をコピーして安全にループ
			auto& childrenCopy = object->children;
			for (auto child : childrenCopy) {
				BeginFrame(child);
			}
		};

	// 親を持たないオブジェクトに対してBeginFrameを呼び出す
	for (int i = 0; i < objects.size(); i++) {
		auto& object = objects[i];
		// オブジェクトが削除されている可能性があるためチェック
		if (!object) continue;
		std::weak_ptr<GameObject> weakObj = object;
		// 親を持つオブジェクトは親のBeginFrameで更新されるのでスキップ
		if (weakObj.expired() || weakObj.lock()->parent) continue;
		// 親を持たないオブジェクトに対してBeginFrameを呼び出す
		BeginFrame(object.get());
	}
	// PersistentObjectManagerのオブジェクトも更新
	for (auto& object : PersistentObjectManager::GetObjects()) {
		if (!object) continue;
		std::weak_ptr<GameObject> weakObj = object;
		if (weakObj.expired() || weakObj.lock()->parent) continue;
		object->BeginFrame();
	}
}

void ObjectManager::EndFrame()
{
	std::function<void(GameObject*)> EndFrame = [&](GameObject* object)
		{
			object->EndFrame();
			// 子をコピーして安全にループ
			auto& childrenCopy = object->children;
			for (auto child : childrenCopy) {
				EndFrame(child);
			}
		};
	for (int i = 0; i < objects.size(); i++) {
		auto& object = objects[i];
		if (!object) continue;
		std::weak_ptr<GameObject> weakObj = object;
		if (weakObj.expired() || weakObj.lock()->parent) continue;
		EndFrame(object.get());
	}
	for (auto& object : PersistentObjectManager::GetObjects()) {
		if (!object) continue;
		std::weak_ptr<GameObject> weakObj = object;
		if (weakObj.expired() || weakObj.lock()->parent) continue;
		object->EndFrame();
	}
}

void ObjectManager::Start()
{
	// すべてのオブジェクトに対して開始処理を呼び出す
	std::function<void(GameObject*)> Start = [&](GameObject* object)
		{
			object->AwakeComponents(); // コンポーネントのAwakeを呼び出す
			object->RefreshActiveInHierarchy(); // 階層内のアクティブ状態を更新
			// 子をコピーして安全にループ
			auto& childrenCopy = object->children;
			for (auto child : childrenCopy) {
				Start(child);
			}
		};
	for (int i = 0; i < objects.size(); i++) {
		auto& object = objects[i];
		if (!object) continue;
		std::weak_ptr<GameObject> weakObj = object;
		if (weakObj.expired() || weakObj.lock()->parent) continue;
		Start(object.get());
	}
}

void ObjectManager::PreUpdate(float deltaTime)
{
	// 削除予約されたオブジェクトを削除
	{
		ProfileScopedSection_2(0, "EraseObjects", ImGuiControl::Profiler::Blue);

		if (!erases.empty())
		{
			// 遅延削除するオブジェクトを一時的に保持するための配列
			std::vector<std::shared_ptr<GameObject>> delayErases;
			for (auto& obj : erases) {
				if (obj) {
					// 遅延値が0より大きい場合は削除を遅らせる
					if (obj->destroyDelay > 0.0f) {
						obj->destroyDelay -= deltaTime;
						delayErases.push_back(obj);
					}
				}
			}
			for (auto& obj : delayErases) {
				// 遅延削除するオブジェクトを削除予約リストから除外
				erases.erase(std::remove(erases.begin(), erases.end(), obj), erases.end());
			}

			// 削除前にオブジェクトのOnDestroyを呼び出す
			for (auto& obj : erases) {
				if (obj) {
					// 選択状態から解除して shared_ptr の参照を確実に手放す
					if (inspectorNode == obj.get()) {
						Reset();
					}

					// 選択状態から解除して shared_ptr の参照を確実に手放す
					if (selection && selection->IsSelected(obj)) {
						selection->Deselect(obj);
					}
					obj->OnDestroy();
					obj->SetActive(false); // オブジェクトを非アクティブにする
				}
			}

			// オブジェクトマネージャのオブジェクトを削除
			objects.erase(std::remove_if(objects.begin(), objects.end(),
				[&](const auto& obj) {
					return std::find(erases.begin(), erases.end(), obj) != erases.end();
				}),
				objects.end());
			erases.clear();

			erases = delayErases; // 遅延削除するオブジェクトを残す
		}
	}
	// コンポーネントキャッシュを更新
	{
		ProfileScopedSection_2(0, "UpdateComponentCache", ImGuiControl::Profiler::Color::Green);

		componentCacheMap.clear();
		for (auto& object : objects) {
			if (!object) continue;
			for (auto& comp : object->GetAllComponents()) {
				if (comp) {
					componentCacheMap[comp->GetId()] = comp;
				}
			}
		}
	}
}

void ObjectManager::Update(float elapsedTime)
{
	//優先度でソート
	{
		ProfileScopedSection_2(0, "SortObjects", ImGuiControl::Profiler::Yellow);
		CurryEngine::OrderManager::Sort(objects);
	}

	std::function<void(GameObject*)> Update = [&](GameObject* object)
		{
			object->Update(elapsedTime);

			//優先度でソート
			std::sort(object->children.begin(), object->children.end(),
				[](GameObject* a, GameObject* b) {
					if (!a) return false;
					if (!b) return true;
					return a->priority < b->priority;
				});

			// ソート後の子をコピーしてループ
			std::vector<GameObject*> childrenCopy = object->children;
			for (auto child : childrenCopy) {

				Update(child);
			}
		};
	for (int i = 0; i < objects.size(); i++) {
		auto& object = objects[i];
		// オブジェクトが削除されている可能性があるためチェック
		if (!object) continue;
		std::weak_ptr<GameObject> weakObj = object;
		// 親を持つオブジェクトは親のUpdateで更新されるのでスキップ
		if (weakObj.expired() || weakObj.lock()->parent) continue;

		// 計測開始
		ProfileScopedSection_3(0, object->name.c_str(), ImGuiControl::Profiler::Green);

		// 親を持たないオブジェクトに対してUpdateを呼び出す
		Update(object.get());
	}
	for (auto& object : PersistentObjectManager::GetObjects()) {
		// オブジェクトが削除されている可能性があるためチェック
		if (!object) continue;
		std::weak_ptr<GameObject> weakObj = object;
		// 親を持つオブジェクトは親のUpdateで更新されるのでスキップ
		if (weakObj.expired() || weakObj.lock()->parent) continue;

		// 計測開始
		ProfileScopedSection_3(0, object->name.c_str(), ImGuiControl::Profiler::Dark);

		// 親を持たないオブジェクトに対してUpdateを呼び出す
		Update(object.get());
	}
}

void ObjectManager::LateUpdate(float elapsedTime)
{
	std::function<void(GameObject*)> LateUpdate = [&](GameObject* object)
		{
			object->LateUpdate(elapsedTime);
			// 子をコピーして安全にループ
			auto& childrenCopy = object->children;
			for (auto child : childrenCopy) {
				LateUpdate(child);
			}
		};
	for (int i = 0; i < objects.size(); i++) {
		auto& object = objects[i];
		// オブジェクトが削除されている可能性があるためチェック
		if (!object) continue;
		std::weak_ptr<GameObject> weakObj = object;
		// 親を持つオブジェクトは親のUpdateで更新されるのでスキップ
		if (weakObj.expired() || weakObj.lock()->parent) continue;
		LateUpdate(object.get());
	}
	for (auto& object : PersistentObjectManager::GetObjects()) {
		// オブジェクトが削除されている可能性があるためチェック
		if (!object) continue;
		std::weak_ptr<GameObject> weakObj = object;
		// 親を持つオブジェクトは親のUpdateで更新されるのでスキップ
		if (weakObj.expired() || weakObj.lock()->parent) continue;
		LateUpdate(object.get());
	}
}

void ObjectManager::FixedUpdate(float fixedElapsedTime)
{
	std::function<void(GameObject*)> FixedUpdate = [&](GameObject* object)
		{
			object->FixedUpdate(fixedElapsedTime);
			// 子をコピーして安全にループ
			auto& childrenCopy = object->children;
			for (auto child : childrenCopy) {
				FixedUpdate(child);
			}
		};
	for (int i = 0; i < objects.size(); i++) {
		auto& object = objects[i];
		// オブジェクトが削除されている可能性があるためチェック
		if (!object) continue;
		std::weak_ptr<GameObject> weakObj = object;
		// 親を持つオブジェクトは親のUpdateで更新されるのでスキップ
		if (weakObj.expired() || weakObj.lock()->parent) continue;
		FixedUpdate(object.get());
	}
	for (auto& object : PersistentObjectManager::GetObjects()) {
		// オブジェクトが削除されている可能性があるためチェック
		if (!object) continue;
		std::weak_ptr<GameObject> weakObj = object;
		// 親を持つオブジェクトは親のUpdateで更新されるのでスキップ
		if (weakObj.expired() || weakObj.lock()->parent) continue;
		FixedUpdate(object.get());
	}
}

void ObjectManager::Render(RenderContext* rtx)
{
	std::function<void(RenderContext*, GameObject*)> Render = [&](RenderContext* rtx, GameObject* object)
		{
			object->BeginRendering(rtx);
			object->Render(rtx);
			/*auto& childrenCopy = object->children;
			for (auto child : childrenCopy) {
				Render(rtx, child);
			}*/
			object->EndRendering(rtx);
		};

	std::vector<std::shared_ptr<GameObject>> renderQueue; // 描画対象のオブジェクトを格納するキュー

	for (int i = 0; i < objects.size(); i++) {
		auto& object = objects[i];
		// オブジェクトが削除されている可能性があるためチェック
		if (!object) continue;
		std::weak_ptr<GameObject> weakObj = object;
		// //親を持つオブジェクトは親のUpdateで更新されるのでスキップ
		// 無効化されたオブジェクトは描画しない
		if (weakObj.expired()/* || weakObj.lock()->parent*/) continue;

		// TODO: あとでリファクタリングすること。GltfModelRendererのマテリアルのアルファモードをチェックして、透過オブジェクトは後で描画するようにする。
		if (auto* gltfModelRenderer = object->GetComponent<GltfModelRenderer>()) {
			auto& materials = gltfModelRenderer->GetModelAsset()->materials;
			if (!materials.empty() && materials[0].data.alphaMode != 0) {
				// 透過オブジェクトは通常の描画パスで描画されるため、ここではスキップ
				renderQueue.push_back(object); // 後で描画するためにキューに追加
				continue;
			}
		}
		Render(rtx, object.get());
	}
	for (auto& transparentObj : renderQueue) {
		if (transparentObj) {
			Render(rtx, transparentObj.get());
		}
	}
	Graphics::GetRenderState()->BindBlendState(rtx->immediateContext, BlendState::Transparency);


	for (auto& object : PersistentObjectManager::GetObjects()) {
		// オブジェクトが削除されている可能性があるためチェック
		if (!object) continue;
		std::weak_ptr<GameObject> weakObj = object;
		// 親を持つオブジェクトは親のUpdateで更新されるのでスキップ
		if (weakObj.expired() || weakObj.lock()->parent) continue;
		Render(rtx, object.get());
	}
}

void ObjectManager::Draw(RenderContext* rtx)
{
	struct UIDrawCall {
		GameObject* object;
		int globalZ;
		D3D11_RECT scissorRect;
		bool hasScissor;
	};

	std::vector<UIDrawCall> drawCalls;

	// フラット収集（再帰）
	std::function<void(GameObject*, int, D3D11_RECT, bool)> Collect
		= [&](GameObject* object, int parentZ, D3D11_RECT parentScissor, bool hasScissor)
		{
			auto rt = object->GetComponent<RectTransform>();
			if (!rt) return;

			int myZ = parentZ + rt->localSortingOrder;

			// このオブジェクト自身がマスクを持つか確認
			D3D11_RECT myScissor = parentScissor;
			if (auto mask = object->GetComponent<Mask>()) {
				D3D11_RECT r = mask->GetScissorRect();
				// 親との交差をとる（ネストしたマスクの正しい挙動）
				if (hasScissor) {
					myScissor.left = max(parentScissor.left, r.left);
					myScissor.top = max(parentScissor.top, r.top);
					myScissor.right = min(parentScissor.right, r.right);
					myScissor.bottom = min(parentScissor.bottom, r.bottom);
				}
				else {
					myScissor = r;
				}
				hasScissor = true;
			}

			drawCalls.push_back({ object, myZ, myScissor, hasScissor });

			for (auto child : object->children)
				Collect(child, myZ, myScissor, hasScissor);
		};

	auto collectRoots = [&](const auto& objectList) {
		for (auto& object : objectList) {
			if (!object) continue;
			if (object->parent) continue;
			Collect(object.get(), 0, {}, false);
		}
		};

	collectRoots(objects);
	collectRoots(PersistentObjectManager::GetObjects());

	// ソート（stable_sortでヒエラルキー順を維持）
	std::stable_sort(drawCalls.begin(), drawCalls.end(),
		[](const UIDrawCall& a, const UIDrawCall& b) {
			return a.globalZ < b.globalZ;
		});

	// 描画
	for (auto& call : drawCalls) {
		if (call.hasScissor) {
			rtx->immediateContext->RSSetScissorRects(1, &call.scissorRect);
			rtx->renderState->BindRasterizerState(rtx->immediateContext, RasterizerState::UseScissorRects);
		}
		else {
			rtx->renderState->BindRasterizerState(rtx->immediateContext, RasterizerState::SolidCullNone);
		}
		call.object->Begin(rtx);
		call.object->Draw(rtx);
		call.object->End(rtx);

	}
}

// 左上(0,0) 右下(width,height)をそのままNDCにマッピングする正射影
inline void Ortho2D(float left, float right, float bottom, float top, float zNear, float zFar, float* m)
{
	m[0] = 2.0f / (right - left);  m[1] = 0;                     m[2] = 0;                     m[3] = 0;
	m[4] = 0;                     m[5] = 2.0f / (top - bottom);  m[6] = 0;                     m[7] = 0;
	m[8] = 0;                     m[9] = 0;                     m[10] = -2.0f / (zFar - zNear); m[11] = 0;
	m[12] = -(right + left) / (right - left);
	m[13] = -(top + bottom) / (top - bottom);
	m[14] = -(zFar + zNear) / (zFar - zNear);
	m[15] = 1.0f;
}

inline XMFLOAT4X4 ComputePivotMatrix(const std::vector<std::shared_ptr<GameObject>>& objects)
{
	if (objects.empty()) {
		return XMFLOAT4X4{
			1, 0, 0, 0,
			0, 1, 0, 0,
			0, 0, 1, 0,
			0, 0, 0, 1
		};
	}
	Vector3 center{ 0, 0, 0 };
	for (const auto& obj : objects) {
		center += obj->GetTransform()->GetWorldPosition();
	}
	center /= objects.size();

	// ピボット行列は中心位置を反映した平行移動行列
	return XMFLOAT4X4{
		1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		center.x, center.y, center.z, 1
	};
}

void ObjectManager::DrawGuizmo(RenderContext* rtx)
{
#ifdef USE_IMGUI
	const auto& objs = selection->GetAll();
	if (!objs.empty())
	{
		bool allHaveTransform = true;
		for (const auto& obj : objs)
		{
			if (!obj->GetTransform())
			{
				allHaveTransform = false;
				break;
			}
		}
		if (!allHaveTransform)
		{
			return; // 選択されたオブジェクトの中にTransformを持たないものがある場合はギズモを表示しない
		}

		const auto* configPtr = EditorConfigManager::GetSceneViewConfig();
		const SceneViewConfig& config = configPtr ? *configPtr : SceneViewConfig{}; // 設定がない場合はデフォルト値を使用
		
		ImGuizmo::MODE mode = static_cast<ImGuizmo::MODE>(config.guizmoPivotMode);

		static ImGuizmo::OPERATION operation = ImGuizmo::TRANSLATE;
		// ショートカットキーで操作モードを切り替える (Q: BOUNDS, W: TRANSLATE, E: ROTATE, R: SCALE)
		// 特定のウィンドウがフォーカスされてるときにのみショートカットを有効にする
		std::vector<std::string> allowedWindowNames = {
			"Scene", // シーンビューで有効
			"Hierarchy"  // ヒエラルキーで有効
		};
		std::string currentWindowName = ImGui::GetCurrentWindow()->Name;
		bool isShortcutAllowed = false;
		for (const auto& allowedName : allowedWindowNames) {
			if (currentWindowName.find(allowedName) != std::string::npos) {
				// フォーカスされているウィンドウが許可されたウィンドウのいずれかに一致する場合、ショートカットを有効にする
				isShortcutAllowed = true;
				break;
			}
		}
		if (isShortcutAllowed)
		{
			if (ImGui::Shortcut(ImGuiKey_Q, ImGuiInputFlags_RouteAlways))
			{
				operation = ImGuizmo::BOUNDS;
			}
			if (ImGui::Shortcut(ImGuiKey_W, ImGuiInputFlags_RouteAlways))
			{
				operation = ImGuizmo::TRANSLATE;
			}
			if (ImGui::Shortcut(ImGuiKey_E, ImGuiInputFlags_RouteAlways))
			{
				operation = ImGuizmo::ROTATE;
			}
			if (ImGui::Shortcut(ImGuiKey_R, ImGuiInputFlags_RouteAlways))
			{
				operation = ImGuizmo::SCALE;
			}
		}

		// スナップ値
		Vector3 snapValue{};
		switch (operation)
		{
		case ImGuizmo::TRANSLATE: if (config.translationSnap.enabled) snapValue = config.translationSnap.snapValue; break;
		case ImGuizmo::ROTATE: if (config.rotationSnap.enabled) snapValue = config.rotationSnap.snapValue; break;
		case ImGuizmo::SCALE: if (config.scaleSnap.enabled) snapValue = config.scaleSnap.snapValue; break;
		};

		//UI系のオブジェクトを選択しているとき
		bool hasRectTransform = false;
		for (const auto& obj : objs)
		{
			if (RectTransform* rect = dynamic_cast<RectTransform*>(obj->transform))
			{
				hasRectTransform = true;
				break;
			}
		}
		if (hasRectTransform) 
		{
			return; // UIオブジェクトが選択されている場合はギズモを表示しない(ギズモ非対応のため)
		}

		static bool wasDragging = false; // 前フレームがドラッグ中だったかを保存する変数
		static std::vector<XMFLOAT4X4> initialWorlds; // 操作開始前のワールド行列を保存する変数
		static XMFLOAT4X4 initialPivotMatrix; // 操作開始前のピボット行列を保存する変数
		static XMFLOAT4X4 currentPivotMatrix; // 現在のピボット行列を保存する変数
		XMFLOAT4X4 pivotMatrix = (wasDragging)
			? currentPivotMatrix // ドラッグ中は現在のピボット行列を使用する
			: (objs.size() == 1)
				? objs[0]->GetTransform()->GetWorld() // 単一選択の場合はそのオブジェクトのワールド行列をピボットにする
				: ComputePivotMatrix(objs); // 複数選択の場合は共通のTransformのワールド行列をピボットにする

		//ギズモ
		ImGuizmo::SetDrawlist();

		float left, top, right, bottom;
		Graphics::GetScreenRect(left, top, right, bottom);
		ImGuizmo::SetRect(left, top, right - left, bottom - top);

		DirectX::XMFLOAT4X4 view, projection;
		view = rtx->view;
		projection = rtx->projection;
		XMFLOAT4X4 deltaPivotMatrix;
		XMStoreFloat4x4(&deltaPivotMatrix, XMMatrixIdentity());

		// ギズモの操作
		bool manipulated = ImGuizmo::Manipulate(
			&view._11,
			&projection._11,
			operation,
			mode,
			&pivotMatrix._11,
			&deltaPivotMatrix._11,
			(snapValue.Length() > 0) ? &snapValue.x : nullptr
		);

		bool isDragging = ImGuizmo::IsUsing(); // ギズモを操作中かどうか


		if (isDragging && !wasDragging) // ギズモの操作を開始したフレーム
		{
			initialWorlds.clear();
			for (const auto& obj : objs) {
				initialWorlds.push_back(obj->GetTransform()->GetWorld());
			}
			initialPivotMatrix = pivotMatrix;
			currentPivotMatrix = pivotMatrix; // 現在のピボット行列も初期値で初期化
		}
		
		if (isDragging) // ギズモの操作中
		{
			currentPivotMatrix = pivotMatrix; // 現在のピボット行列を更新

			XMMATRIX initialPivot = XMLoadFloat4x4(&initialPivotMatrix); // 操作開始前のピボット行列
			XMMATRIX initialPivotInv = XMMatrixInverse(nullptr, initialPivot);
			XMMATRIX currentPivot = XMLoadFloat4x4(&pivotMatrix); // 現在のピボット行列

			// ピボット行列の変化量を計算するために、現在のピボット行列と操作開始前のピボット行列の逆行列を掛け合わせる
			// これにより、ピボット行列の変化量が得られる。これを各オブジェクトのワールド行列に適用することで、ギズモの操作がオブジェクトに反映される。
			XMMATRIX pivotDelta = XMMatrixMultiply(currentPivot, initialPivotInv); // ピボット行列の変化量を計算
			
			for (size_t i = 0; i < objs.size(); i++) {
				const auto& obj = objs[i];
				Transform* t = obj->GetTransform();
				XMMATRIX initialWorld = XMLoadFloat4x4(&initialWorlds[i]);

				// オブジェクトのワールド行列にピボット行列の変化量を適用するために、ピボット行列の変化量とオブジェクトの初期ワールド行列を掛け合わせる
				XMMATRIX newWorld = XMMatrixMultiply(pivotDelta, initialWorld);

				// 新しいワールド行列をスケール、回転、位置に分解してオブジェクトのTransformに反映させる
				XMVECTOR newScale, newRot, newPos;
				XMMatrixDecompose(&newScale, &newRot, &newPos, newWorld);
				XMFLOAT3 s;
				XMFLOAT4 r;
				XMFLOAT3 p;
				XMStoreFloat3(&s, newScale);
				XMStoreFloat4(&r, newRot);
				XMStoreFloat3(&p, newPos);
				t->SetWorldPosition(Vector3(p));
				t->SetWorldRotation(r);
				t->SetWorldScale(Vector3(s));
			}
		}

		if (!isDragging && wasDragging) // ギズモの操作を終了したフレーム
		{
			if (initialWorlds.size() == objs.size()) { // 保存されている操作開始前のワールド行列の数が現在の選択オブジェクトの数と一致していることを確認
				// Historyに変更を記録する。
				using namespace CurryEngine;
				auto cmd = std::make_shared<CompoundCommand>("Transform Change");

				for (size_t i = 0; i < objs.size(); i++) {
					const auto& obj = objs[i];
					Transform* t = obj->GetTransform();
					XMMATRIX initialWorld = XMLoadFloat4x4(&initialWorlds[i]);
					XMFLOAT4X4 objWorld = t->GetWorld();
					XMMATRIX newWorld = XMLoadFloat4x4(&objWorld);
					XMVECTOR oldScale, oldRot, oldPos;
					XMMatrixDecompose(&oldScale, &oldRot, &oldPos, initialWorld);
					XMVECTOR newScale, newRot, newPos;
					XMMatrixDecompose(&newScale, &newRot, &newPos, newWorld);
					XMFLOAT3 oldS, newS;
					XMFLOAT4 oldR, newR;
					XMFLOAT3 oldP, newP;
					XMStoreFloat3(&oldS, oldScale);
					XMStoreFloat4(&oldR, oldRot);
					XMStoreFloat3(&oldP, oldPos);
					XMStoreFloat3(&newS, newScale);
					XMStoreFloat4(&newR, newRot);
					XMStoreFloat3(&newP, newPos);
					// 変更前の値を保存
					Vector3 prevPos(oldP);
					Vector3 prevScale(oldS);
					Quaternion prevRot(oldR);
					// 変更後の値を保存
					Vector3 newPosVec(newP);
					Vector3 newScaleVec(newS);
					Quaternion newRotQuat(newR);

					// コマンドを作成して履歴に追加
					cmd->AddCommand(std::make_unique<SetValueCommand<Vector3>>(
						"Position",
						[t](const Vector3& value) { t->SetWorldPosition(value); },
						prevPos,
						newPosVec
					));
					cmd->AddCommand(std::make_unique<SetValueCommand<Vector3>>(
						"Scale",
						[t](const Vector3& value) { t->SetWorldScale(value); },
						prevScale,
						newScaleVec
					));
					cmd->AddCommand(std::make_unique<SetValueCommand<Quaternion>>(
						"Rotation",
						[t](const Quaternion& value) { t->SetWorldRotation(value); },
						prevRot,
						newRotQuat
					));
				}
				// コマンドを履歴に追加
				History::ExecuteCommand(cmd);
			}
			initialWorlds.clear(); // 操作開始前のワールド行列の保存をクリア
		}

		wasDragging = isDragging; // 現在のドラッグ状態を保存しておく

		//// ギズモの操作
		//if (manipulated)
		//{
		//	XMVECTOR Scale, Rotation, Position;//ワールド座標を保存
		//	XMMATRIX W = XMLoadFloat4x4(&world);

		//	XMFLOAT3 s;
		//	XMFLOAT4 r;
		//	XMFLOAT3 p;

		//	//ワールド行列を各要素に分解し、更新
		//	if (XMMatrixDecompose(&Scale, &Rotation, &Position, W))
		//	{
		//		XMStoreFloat3(&s, Scale);
		//		XMStoreFloat4(&r, Rotation);
		//		XMStoreFloat3(&p, Position);

		//		transform->SetWorldPosition(Vector3(p));
		//		transform->SetWorldRotation(r);
		//		transform->SetWorldScale(Vector3(s));
		//	}
		//}
	}
#endif // USE_IMGUI
}

// オブジェクトの階層構造を考慮して、親を持たないオブジェクトのみを返す
static std::vector<std::weak_ptr<GameObject>> OrganizeObjects(const std::vector<std::shared_ptr<GameObject>>& objects, GameObject* parent)
{
	std::vector<std::weak_ptr<GameObject>> organized;
	for (const auto& obj : objects) {
		std::weak_ptr<GameObject> weakObj = obj;
		if (weakObj.expired()) continue;
		if (weakObj.lock()->parent == parent) {
			organized.push_back(obj);
		}
	}
	return organized;
}

static void AddChildren(std::vector<std::shared_ptr<GameObject>>& result, const std::shared_ptr<GameObject>& obj) {
	if (!obj) return;
	result.push_back(obj);
	for (const auto& child : obj->children) {
		std::shared_ptr<GameObject> sharedObj = ObjectManager::Find_Ptr(child->GetId());
		if (sharedObj) {
			AddChildren(result, sharedObj);
		}
	}
}

static std::vector<std::shared_ptr<GameObject>> OrganizeTreeNodes(const std::vector<std::shared_ptr<GameObject>>& objects)
{
	std::vector<std::weak_ptr<GameObject>> organized = OrganizeObjects(objects, nullptr);
	std::vector<std::shared_ptr<GameObject>> result;
	
	for (const auto& obj : organized) {
		AddChildren(result, obj.lock());
	}
	return result;
}

void ObjectManager::DrawHierarchy()
{
#ifdef USE_IMGUI
	
	if (ImGui::Begin("Hierarchy", nullptr, ImGuiWindowFlags_None))
	{
		auto* window = ImGui::GetCurrentWindow();
		bool isNotDestroyObject = false;
		bool isDroppedGameObjectThisFrame = false;
		int i = 0;

		auto DrawDropZone = [&](GameObject* target, int& idCounter, bool accept, bool appendToEnd = false)
			{
				ImGui::PushID(idCounter++);

				// 高さ6pxの空間を確保（カーソルは進む）
				ImVec2 pos = ImGui::GetCursorScreenPos();
				float w = ImGui::GetContentRegionAvail().x;
				float h = 6.0f;
				ImGui::Dummy(ImVec2(w, h));

				if (accept && ImGui::BeginDragDropTarget())
				{
					// ホバー中のみライン表示
					if (ImGui::IsMouseHoveringRect(pos, ImVec2(pos.x + w, pos.y + h)))
					{
						ImGui::GetWindowDrawList()->AddLine(
							ImVec2(pos.x, pos.y + h * 0.5f),
							ImVec2(pos.x + w, pos.y + h * 0.5f),
							IM_COL32(100, 180, 255, 255), 2.0f
						);
					}
					if (ImGui::AcceptDragDropPayload("GameObject"))
					{
						m_pendingDrop = PendingDrop{ target, /*reorder=*/true, /*appendToEnd=*/appendToEnd };
					}
					ImGui::EndDragDropTarget();
				}

				ImGui::PopID();
			};


		std::function<void(GameObject*)> DrawNodeTree = [&](GameObject* object)
			{
				// オブジェクトが削除されている可能性があるためチェック
				if (!object) return;

				ImGui::PushID(i++);

				//矢印をクリックで階層を開く。当たり判定は余白も含める
				ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_OpenOnArrow
					| ImGuiTreeNodeFlags_FramePadding
					| ImGuiTreeNodeFlags_SpanAvailWidth;

				// デフォルトで階層を開いておくかどうか
				if (object->isDefaultOpenOnHierarchy) {
					nodeFlags |= ImGuiTreeNodeFlags_DefaultOpen;
				}

				//子がいない場合は矢印をつけない
				size_t childCount = object->children.size();
				if (childCount == 0) {
					nodeFlags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
				}

				//選択フラグ
				if (selection->IsSelected(object)) {
					nodeFlags |= ImGuiTreeNodeFlags_Selected;
				}
				if (isDroppedGameObjectThisFrame) {
					nodeFlags |= ImGuiTreeNodeFlags_Selected;
				}

				//PersistentObjectManagerに登録されているオブジェクトかどうか
				bool acceptDrop = draggingObjectIsNotDestroyObject == isNotDestroyObject;

				//ドロップ先（親子関係を解除したいとき、もしくは優先度の並び替えのとき）にドロップ可能かどうか
				if (selection)
				{
					for (auto& selectObj : selection->GetAll()) {
						// ドロップソースのオブジェクトがUIオブジェクトの場合、ドロップ先がUIオブジェクトでないと親子関係を構築できないようにする
						if (selectObj && selectObj->GetComponent<RectTransform>()) {
							if (!object->GetComponent<RectTransform>()) {
								if (object->GetParent() != selectObj->GetParent())
									acceptDrop = false;
							}
							break;
						}
					}
				}

				
				DrawDropZone(object, i, acceptDrop);

				//ツリーノードを描画
				float alpha = 1;//uniqueId-isVisible
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 1, 1, alpha));
				bool isActive = object->IsActiveSelf();
				if (ImGui::Checkbox(/*std::to_string(i + 1).c_str()*/ "", &isActive))
					object->SetActive(isActive);
				ImGui::PopStyleColor();
				ImGui::SameLine();
				//ImGui::Text(std::to_string(object->id.Value()).c_str());
				//ImGui::SameLine();
#if 0
				// 優先度表示
				ImGui::Text("%d", object->priority);
				ImGui::SameLine();
#endif // 0
				float textColor = object->IsActive() ? 1.0f : 0.5f;
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(textColor, textColor, textColor, 1.0f));
				ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.25f, 0.25f, 0.25f, 1.0f));
				ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
				ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.3f, 0.3f, 0.3f, 1.0f));
				// ノードを描画。開いているかどうかを返す
				bool opened = ImGui::TreeNodeEx(object, nodeFlags, object->GetName().c_str());
				object->isDefaultOpenOnHierarchy = opened; //開いているかどうかを保存しておく
				
				//ノードに対してドラッグ（親子関係構築）
				if (acceptDrop && ImGui::BeginDragDropTarget()) {
					if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("GameObject")) {
						IM_ASSERT(payload->DataSize == sizeof(ObjectId*));
						//IM_ASSERT(payload->DataSize == sizeof(EditorSelection*));
						ObjectId* pRef = static_cast<ObjectId*>(payload->Data);
						if (pRef)
						{
							m_pendingDrop = PendingDrop{
								.target = object,
								.reorder = false
							};
							isDroppedGameObjectThisFrame = true;
						}
					}
					ImGui::EndDragDropTarget();
				}
				//GameObject*データとしてドラッグ
				if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)) {
					ImGui::SetDragDropPayload("GameObject", &object->id, sizeof(ObjectId*));
					ImGui::Text(std::format("Dragging {} object(s)", selection->GetAll().size()).c_str());
					for (auto& notDestroyObject : PersistentObjectManager::GetObjects()) {
						if (notDestroyObject->GetId() == selectNode->GetId()) {
							draggingObjectIsNotDestroyObject = true;
							break;
						}
					}

					ImGui::EndDragDropSource();
				}
				//フォーカスされたノードを選択する
				static bool delayClick = false;
				if ((ImGui::IsItemClicked(ImGuiMouseButton_Left) || ImGui::IsItemClicked(ImGuiMouseButton_Right)) && !ImGui::IsItemToggledOpen())
				{
					if (selection)
					{
						if (selection->IsSelected(object))
						{
							delayClick = true;
						}
						else
						{
							bool ctrl = ImGui::IsKeyDown(ImGuiKey_LeftCtrl) || ImGui::IsKeyDown(ImGuiKey_RightCtrl);
							bool shift = ImGui::IsKeyDown(ImGuiKey_LeftShift) || ImGui::IsKeyDown(ImGuiKey_RightShift);
							const std::shared_ptr<GameObject>& spObject = Find_Ptr(object->GetId());
							if (shift) // Shiftキーが押されている場合は、クリックしたオブジェクトから現在の選択範囲までを選択する
								selection->SelectRange(spObject, OrganizeTreeNodes(objects), /*additive=*/ctrl);
							else // Shiftキーが押されていない場合は、クリックしたオブジェクトを選択する。Ctrlキーが押されている場合は、選択に追加する。押されていない場合は、選択を置き換える
								selection->Select(spObject, /*additive=*/ctrl);
						}
					}
				}
				// クリックしてからマウスを動かしても選択されないように、クリック後のフレームで選択する
				bool isReleased = ImGui::IsMouseReleased(ImGuiMouseButton_Left) || ImGui::IsMouseReleased(ImGuiMouseButton_Right);
				if (delayClick && ImGui::IsItemHovered() && isReleased)
				{
					if (selection)
					{
						bool ctrl = ImGui::IsKeyDown(ImGuiKey_LeftCtrl) || ImGui::IsKeyDown(ImGuiKey_RightCtrl);
						bool shift = ImGui::IsKeyDown(ImGuiKey_LeftShift) || ImGui::IsKeyDown(ImGuiKey_RightShift);
						const std::shared_ptr<GameObject>& spObject = Find_Ptr(object->GetId());
						if (shift) selection->SelectRange(spObject, OrganizeTreeNodes(objects));
						else if (ctrl) selection->Select(spObject, true);
						else selection->Select(spObject, false);
					}
					delayClick = false;
				}

				ImGui::PopStyleColor(4);

				// 選択されているノードをInspectorに表示する
				bool flag = ImGui::IsItemActive() || ImGui::IsItemHovered(); //ノードがアクティブまたはホバーされているか
				bool isActiveAndHovered = ImGui::IsItemActive() && ImGui::IsItemHovered(); //ノードがアクティブかつホバーされているか
				auto selectNode = GetSelectNode();
				if (selectNode && flag && isReleased) {
					if (!ImGui::GetDragDropPayload()) {
						SelectInspectorNode(selectNode);
					}
				}

				// ダブルクリックでそのオブジェクトのフォーカスに移動
				if (selectNode && (isActiveAndHovered) && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
				{
					if (selectNode->transform && scene)
					{
						Vector3 pos = (selectNode->transform->GetWorldPosition());
						scene->GetSceneViewEditorCamera()->SetPosition(pos);
					}
				}

				if (ImGui::IsItemHovered())
				{
					ImGui::SetTooltip("Priority: %d\nID: %d", object->GetPriority(), object->GetId().Value());
				}

				// ノードIDを戻す
				ImGui::PopID();
				//開かれている場合、子階層にも同じ処理をする
				if (opened && childCount > 0) {
					if (object) {
						for (GameObject* child : object->children) {
							DrawNodeTree(child);
						}
					}
					DrawDropZone(object, i, acceptDrop);
					ImGui::TreePop();
				}
			};

		// 空白クリックで選択解除
		if ((ImGui::IsMouseClicked(ImGuiMouseButton_Left) || ImGui::IsMouseClicked(ImGuiMouseButton_Right)) && ImGui::IsWindowHovered())
		{
			// ノード上ではなく、ウィンドウの余白がクリックされた場合
			if (!ImGui::IsAnyItemHovered())
			{
				Reset();
			}
		}

		// --- 右クリックメニュー ---
		if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right))
		{
			ImGui::OpenPopup("HierarchyContextMenu");
		}
		
		//ドロップ先（親子関係を解除したいとき）
		bool acceptDrop = true;
		if (selection)
		{
			for (auto& selectObj : selection->GetAll()) {
				// ドロップソースのオブジェクトがUIオブジェクトの場合、ドロップ先がUIオブジェクトでないと受け入れない
				if (selectObj && selectObj->GetComponent<RectTransform>()) {
					acceptDrop = false;
					break;
				}
			}
		}
		if (acceptDrop)
		{
			if (ImGui::BeginDragDropTarget()) {
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("GameObject")) {
					IM_ASSERT(payload->DataSize == sizeof(ObjectId*));
					ObjectId* pRef = static_cast<ObjectId*>(payload->Data);
					if (pRef)
					{
						for (auto& pObj : selection->GetAll())
						{
							GameObject* obj = pObj.get();
							obj->SetParent(nullptr);
						}
					}
				}
				ImGui::EndDragDropTarget();
			}
		}

		for (auto& object : objects) {
			//開かれている場合、子階層にも同じ処理をする
			if (!object || object->parent) continue;
			DrawNodeTree(object.get());
		}

		// リスト末尾の番兵DropZone
		// ドロップ先として「最後のルートオブジェクト」を使う
		{
			GameObject* lastRoot = nullptr;
			for (auto it = objects.rbegin(); it != objects.rend(); ++it) {
				if (*it && !(*it)->parent) { lastRoot = it->get(); break; }
			}
			bool sentinelAccept = true;
			if (selection) {
				for (auto& selectObj : selection->GetAll()) {
					if (selectObj && selectObj->GetComponent<RectTransform>()) {
						sentinelAccept = false; break;
					}
				}
			}
			if (lastRoot) {
				DrawDropZone(lastRoot, i, sentinelAccept, true);
			}
		}


		if (m_pendingDrop.has_value())
		{
			auto& drop = m_pendingDrop.value();
			for (auto& pObj : selection->GetAll())
			{
				GameObject* obj = pObj.get();
				if (!obj) continue;

				// ドロップ先が同じ親を持つオブジェクトの並び替えかどうかは、ドロップ先とドロップ元の親が同じかどうかで判断する
				if (drop.reorder)
				{
					if (obj->GetParent() != drop.target->GetParent())
						obj->SetParent(drop.target->GetParent());

					auto itObj = std::find_if(objects.begin(), objects.end(),
						[&](const auto& o) { return o.get() == obj; });
					if (itObj == objects.end()) continue;
					size_t fromIdx = std::distance(objects.begin(), itObj);

					size_t toIdx = 0;
					if (drop.appendToEnd)
					{
						// ドロップ先の親と同じ親を持つオブジェクトの数を数える
						toIdx = objects.size();
					}
					else
					{
						auto itTarget = std::find_if(objects.begin(), objects.end(),
							[&](const auto& o) { return o.get() == drop.target; });

						if (itTarget == objects.end()) continue;
						toIdx = std::distance(objects.begin(), itTarget);
					}
					CurryEngine::OrderManager::MoveObject(objects, fromIdx, toIdx);
				}
				else
				{
					// ドロップ先がドロップ元の子であるかどうかを確認するために、ドロップ先の親をたどっていく
					bool isChild = false;
					GameObject* parent = drop.target;
					while (parent)
					{
						if (parent == obj)
						{
							isChild = true;
							break;
						}
						parent = parent->GetParent();
					}
					// ドロップ先がドロップ元の子でない場合のみ親子関係を構築する
					if (!isChild) {
						obj->SetParent(drop.target); // 親子関係構築
					}
				}
			}
			m_pendingDrop.reset();
		}

		isNotDestroyObject = true;

		//DontDestroyOnLoadで保持されているオブジェクト
		if (!PersistentObjectManager::GetObjects().empty()) {
			if (ImGui::TreeNodeEx("Don't Destroy Objects", ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick)) {
				for (auto& object : PersistentObjectManager::GetObjects()) {
					//開かれている場合、子階層にも同じ処理をする
					if (!object || object->parent) continue;
					DrawNodeTree(object.get());
				}
				ImGui::TreePop();
			}
		}

		//アセットブラウザからドラッグアンドドロップでモデルインスタンス生成
		if (window)
		{
			if (ImGui::BeginDragDropTargetCustom(window->Rect(), window->ID))
			{
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET_PATH")) {
					const char* p = static_cast<const char*>(payload->Data);
					std::filesystem::path path = p ? p : "";
					AssetType assetType = AssetBrowser::DetectAssetTypeFromFile(path);
					Scene* currentScene = SceneManager::GetCurrentScene();
					switch (assetType)
					{
					case AssetType::Unknown:
						break;
					case AssetType::Texture:
						GameObjectFactory::CreateImage(currentScene, path.stem().string(), nullptr, path.wstring().c_str());
						break;
					case AssetType::Model:
						GameObjectFactory::CreateModel(currentScene, path.stem().string(), path.string());
						break;
					case AssetType::Sound:
						GameObjectFactory::CreateAudioSource(currentScene, path.stem().string(), path.wstring().c_str());
						break;
					case AssetType::Scene:
					{
						SceneManager::ChangeScene(path.stem().string());
						break;
					}
					case AssetType::Prefab:
					{
						json prefabJson;
						JsonFileHandler::LoadJsonFromFile(prefabJson, path.string(), JsonIOFormat::Binary);
						GameObject* newObject = Instantiate(prefabJson);
						newObject->SetParent(GetSelectNode());
						break;
					}
					default:
						break;
					}
				}
				ImGui::EndDragDropTarget();
			}
		}

		// --- 右クリックメニューの中身 ---
		if (selection)
		{
			if (ImGui::BeginPopup("HierarchyContextMenu"))
			{
				// 選択中のノードに対するメニュー
				if (!selection->IsEmpty())
				{
					// 削除ボタン
					if (ImGui::MenuItem("Delete", "Del", false))
					{
						auto selectAll = selection->GetAll();
						for (int i = selectAll.size() - 1; i >= 0; --i)
						{
							if (selectAll[i])
							{
								Destroy(selectAll[i]->GetName());
							}
						}
						selection->Clear();
					}
					// 複製ボタン
					if (ImGui::MenuItem("Duplicate", "Ctrl+D", false))
					{
						auto selectAll = selection->GetAll();
						selection->Clear();
						for (auto& pObj : selectAll)
						{
							if (GameObject* newObject = Duplicate(pObj.get()))
							{
								selection->Select(Find_Ptr(newObject->GetId()), true);
								// 複製したオブジェクトをInspectorに表示する
								SelectInspectorNode(newObject);
							}
						}
					}
					//// 優先度変更ボタン
					//if (ImGui::MenuItem("Increase Priority", "Alt+Up", false))
					//{
					//	for (auto& pObj : selection->GetAll())
					//	{
					//		int oldPriority = pObj->priority;

					//		pObj->priority++;
					//	}
					//}
					//if (ImGui::MenuItem("Decrease Priority", "Alt+Down", false))
					//{
					//	for (auto& pObj : selection->GetAll())
					//	{
					//		pObj->priority--;
					//	}
					//}
					// プレハブ化ボタン
					// TODO: プレハブ化を右クリックメニューからではなく、ドラッグアンドドロップでできるようにする。（複数選択のときの挙動が難しいため。）
					if (ImGui::MenuItem("Create Prefab", "", false))
					{
						if (auto selectNode = GetSelectNode())
						{
							char buffer[256] = "";
							if (Dialog::SaveFileName(buffer, 256, "Prefab Files\0*.prefab\0All Files\0*.*\0", "prefab") == DialogResult::OK)
							{
								// 拡張子を.prefabに変更
								std::filesystem::path savePath = buffer;
								savePath.replace_extension(".prefab");
								// プレハブとして保存
								SaveGameObject(selectNode, savePath.string());
							}
						}
					}
				}

				EditorGUI::DrawGameObjectMenu();
				ImGui::EndPopup();
			}

			if (!selection->IsEmpty())
			{
				// 削除のショートカットキー
				if (ImGui::Shortcut(ImGuiKey_Delete, ImGuiInputFlags_RouteFocused))
				{
					// コピーを作成してループ中のリスト変更による問題を回避する
					auto selectAll = selection->GetAll();
					for (auto& pObj : selectAll)
					{
						if (pObj)
						{
							Destroy(pObj->GetName());
						}
					}
					selection->Clear(); // 処理後に選択をクリア
				}
				// 優先度変更のショートカットキー
				/*if (ImGui::Shortcut(ImGuiMod_Alt | ImGuiKey_UpArrow, ImGuiInputFlags_RouteFocused))
				{
					for (auto& pObj : selection->GetAll())
					{
						if (pObj)
						{
							pObj->priority++;
						}
					}
				}
				if (ImGui::Shortcut(ImGuiMod_Alt | ImGuiKey_DownArrow, ImGuiInputFlags_RouteFocused))
				{
					for (auto& pObj : selection->GetAll())
					{
						if (pObj)
						{
							pObj->priority--;
						}
					}
				}*/
				// 複製のショートカットキー
				if (ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiKey_D, ImGuiInputFlags_RouteFocused))
				{
					auto selectAll = selection->GetAll();
					selection->Clear();
					for (auto& pObj : selectAll)
					{
						if (pObj)
						{
							if (GameObject* newObject = Duplicate(pObj.get()))
							{
								selection->Select(Find_Ptr(newObject->GetId()), true);
								// 複製したオブジェクトをInspectorに表示する
								SelectInspectorNode(newObject);
							}
						}
					}
				}
			}
		}

	}
	ImGui::End();
#endif // USE_IMGUI
}

void ObjectManager::DrawProperty()
{
#ifdef USE_IMGUI
	CurryEngine::InspectorWindow::Get().Draw(this);
#endif // USE_IMGUI
}

GameObject* ObjectManager::Find(const std::string& name)
{
	for (Scene* scene : SceneManager::GetActiveScenes())
	{
		if (scene)
		{
			for (auto& object : scene->objectManager->objects) {
				// オブジェクトがnullptrの場合はスキップ
				if (!object) continue;
				// 弱参照が切れている場合はスキップ
				std::weak_ptr<GameObject> weakObj = object;
				if (weakObj.expired()) continue;

				// 名前が一致したら返す
				if (object->GetName() == name) {
					return object.get();
				}
			}
		}
	}
	for (auto& object : PersistentObjectManager::GetObjects()) {
		// オブジェクトがnullptrの場合はスキップ
		if (!object) continue;
		// 弱参照が切れている場合はスキップ
		std::weak_ptr<GameObject> weakObj = object;
		if (weakObj.expired()) continue;

		// 名前が一致したら返す
		if (object->GetName() == name) {
			return object.get();
		}
	}
	return nullptr;
}
GameObject* ObjectManager::Find(const ObjectId& id)
{
	for (Scene* scene : SceneManager::GetActiveScenes())
	{
		if (scene)
		{
			for (auto& object : scene->objectManager->objects) {
				// オブジェクトがnullptrの場合はスキップ
				if (!object) continue;
				// 弱参照が切れている場合はスキップ
				std::weak_ptr<GameObject> weakObj = object;
				if (weakObj.expired()) continue;
				// IDが一致したら返す
				if (object->GetId() == id) {
					return object.get();
				}
			}
		}
	}
	for (auto& object : PersistentObjectManager::GetObjects()) {
		// オブジェクトがnullptrの場合はスキップ
		if (!object) continue;
		// 弱参照が切れている場合はスキップ
		std::weak_ptr<GameObject> weakObj = object;
		if (weakObj.expired()) continue;
		// IDが一致したら返す
		if (object->GetId() == id) {
			return object.get();
		}
	}
	return nullptr;
}

std::shared_ptr<GameObject> ObjectManager::Find_Ptr(const std::string& name)
{
	for (Scene* scene : SceneManager::GetActiveScenes())
	{
		if (scene)
		{
			for (auto& object : scene->objectManager->objects) {
				// オブジェクトがnullptrの場合はスキップ
				if (!object) continue;
				// 弱参照が切れている場合はスキップ
				std::weak_ptr<GameObject> weakObj = object;
				if (weakObj.expired()) continue;
				// 名前が一致したら返す
				if (object->GetName() == name) {
					return object;
				}
			}
		}
	}
	for (auto& object : PersistentObjectManager::GetObjects()) {
		// オブジェクトがnullptrの場合はスキップ
		if (!object) continue;
		// 弱参照が切れている場合はスキップ
		std::weak_ptr<GameObject> weakObj = object;
		if (weakObj.expired()) continue;
		// 名前が一致したら返す
		if (object->GetName() == name) {
			return object;
		}
	}
	return nullptr;
}
std::shared_ptr<GameObject> ObjectManager::Find_Ptr(const ObjectId& id)
{
	for (Scene* scene : SceneManager::GetActiveScenes())
	{
		if (scene)
		{
			for (auto& object : scene->objectManager->objects) {
				// オブジェクトがnullptrの場合はスキップ
				if (!object) continue;
				// 弱参照が切れている場合はスキップ
				std::weak_ptr<GameObject> weakObj = object;
				if (weakObj.expired()) continue;
				// IDが一致したら返す
				if (object->GetId() == id) {
					return object;
				}
			}
		}
	}
	for (auto& object : PersistentObjectManager::GetObjects()) {
		// オブジェクトがnullptrの場合はスキップ
		if (!object) continue;
		// 弱参照が切れている場合はスキップ
		std::weak_ptr<GameObject> weakObj = object;
		if (weakObj.expired()) continue;
		// IDが一致したら返す
		if (object->GetId() == id) {
			return object;
		}
	}
	return nullptr;
}

std::shared_ptr<Component> ObjectManager::FindComponent(const ObjectId& id)
{
	for (Scene* scene : SceneManager::GetActiveScenes())
	{
		if (scene)
		{
			const auto& components = scene->GetObjectManager()->GetComponentCacheMap();
			auto it = components.find(id);
			if (it != components.end()) {
				std::weak_ptr<Component> weakComp = it->second;
				if (!weakComp.expired()) {
					return it->second.lock();
				}
			}
		}
	}
	return nullptr;
}


GameObject* ObjectManager::FindInObjects(const std::string& name)
{
	for (auto& object : objects) {
		// オブジェクトがnullptrの場合はスキップ
		if (!object) continue;
		// 弱参照が切れている場合はスキップ
		std::weak_ptr<GameObject> weakObj = object;
		if (weakObj.expired()) continue;
		// 名前が一致したら返す
		if (object->GetName() == name) {
			return object.get();
		}
	}
	// 見つからなかった場合はnullptrを返す
	return nullptr;
}

GameObject* ObjectManager::FindInObjects(const ObjectId& id)
{
	for (auto& object : objects) {
		// オブジェクトがnullptrの場合はスキップ
		if (!object) continue;
		// 弱参照が切れている場合はスキップ
		std::weak_ptr<GameObject> weakObj = object;
		if (weakObj.expired()) continue;
		// IDが一致したら返す
		if (object->GetId() == id) {
			return object.get();
		}
	}
	// 見つからなかった場合はnullptrを返す
	return nullptr;
}

void ObjectManager::Destroy(const std::string& name) {
	std::shared_ptr<GameObject> object = Find_Ptr(name);
	if (object) {
		// すでに削除予定リストにあるかどうかを確認
		auto it = std::find(erases.begin(), erases.end(), object);
		if (it != erases.end()) {
			return; // すでに削除予定リストにある場合は何もしない
		}
		// オブジェクトを破棄予定リストに追加
		erases.emplace_back(object);

		// 子オブジェクトも再帰的に削除
		DestroyChildren(object.get());
	}
}

GameObject* ObjectManager::GetSelectNode() const
{
	return selection->GetPrimary().get();
}

void ObjectManager::DestroyChildren(GameObject* object) {
	if (object) {
		for (GameObject* child : object->GetChildren()) {
			Destroy(child->GetName());
			//DestroyChildren(child); // 子オブジェクトはDestroy関数内で再帰的に削除されるため、ここでは呼び出さない
		}
	}
}

void ObjectManager::Register(std::shared_ptr<GameObject> object)
{
	if (object) {
		if (object->version >= (int)CurryEngine::GameObjectSerializeVersion::Priority)
		{
			int tailPriority = objects.empty()
				? 0
				: objects.back()->GetPriority() + CurryEngine::OrderManager::STEP;
			object->SetPriority(tailPriority);
		}
		objects.push_back(object);
		object->scene = scene; // シーンを設定
	}
}

void ObjectManager::Reset()
{
	selectNode = nullptr;
	inspectorNode = nullptr;
	lockInspector = false;
	if (selection)
	{
		selection->Clear();
	}
}

void ObjectManager::SelectInspectorNode(GameObject* node)
{
	if (!lockInspector) {
		inspectorNode = node;
	}
}

json ObjectManager::Serialize() const
{
	// 全オブジェクトをJSONにシリアライズ
	json j;
	for (auto& object : objects) {
		json obj = object->Serialize();
		if (!obj.is_null()) {
			j.push_back(obj);
		}
	}
	return j;
}

void ObjectManager::Deserialize(const json& j)
{
	if (!j.is_array()) return;

	// JSONデータからオブジェクトを復元
	for (const json& item : j) {
		if (item.contains("name") && item["name"].is_string()) {
			std::shared_ptr<GameObject> object = std::make_shared<GameObject>();
			object->Deserialize(item);
			Register(object);
		}
	}

	// レガシーIDを新しいIDに変換する
	SceneMigrator::Migrate(this);

	// 親子関係の復元
	for (const json& item : j) {
		if (item.contains("name") && item["name"].is_string()) {
			std::string name = item["name"];
			if (GameObject* object = FindInObjects(name))
			{
				// 親IDが設定されている場合、親オブジェクトを探して設定
				if (object->pendingParentID.IsValid())
				{
					GameObject* parent = FindInObjects(object->pendingParentID);
					if (parent) {
						object->SetParent(parent);
					}
					else {
						object->SetParent(nullptr);
					}
					object->pendingParentID = ObjectId::Invalid();
				}
				// コンポーネントのデシリアライズ
				object->DeserializeComponents(item, {});

				// アクティブ状態の更新
				object->RefreshActiveInHierarchy();

			}
		}
		
	}
}

GameObject* ObjectManager::Duplicate(GameObject* original)
{
	if (!original) return nullptr;

	// オリジナルのオブジェクトをシリアライズ(子も含む)
	std::vector<GameObject*> targets;
	std::function<void(GameObject*)> collectObjects = [&](GameObject* obj)
		{
			targets.push_back(obj);
			for (GameObject* child : obj->GetChildren()) {
				collectObjects(child);
			}
		};
	collectObjects(original);

#if 0
	std::unordered_map<ObjectId, ObjectId> idMap; // 古いIDから新しいIDへのマッピング

	// 新しいIDを生成してマッピングを作成
	for (GameObject* obj : targets) {
		ObjectId oldID = obj->id;
		ObjectId newID = ObjectId::Generate(); // 新しいIDを生成
		idMap[oldID] = newID;
	}

	// 親IDを変換する関数
	std::function<ObjectId(ObjectId)> convertParent = [&](ObjectId oldParentID) {
		auto it = idMap.find(oldParentID);
		if (it != idMap.end()) {
			return it->second;
		}
		return ObjectId::Invalid(); // 見つからなかった場合
		};
#endif // 0


	// オブジェクトをJSONにシリアライズ
	json j;
	for (GameObject* obj : targets) {
		json objJson = obj->Serialize();
		
		if (!objJson.is_null())
		{
			j.push_back(objJson);
		}
	}

#if 0
	// オブジェクトリストをクリア
	targets.clear();

	// 新しいオブジェクトを作成してデシリアライズ
	for (const json& item : j)
	{
		if (item.contains("name") && item["name"].is_string())
		{
			std::shared_ptr<GameObject> newObject = std::make_shared<GameObject>();
			newObject->Deserialize(item);
			// ユニークな名前に変更
			newObject->name = newObject->MakeUniqueName(newObject->name);

			// 新しいIDを設定
			ObjectId oldID = newObject->id;
			newObject->id = idMap[oldID];
			// 親IDを変換して設定
			newObject->pendingParentID = convertParent(newObject->pendingParentID);

			// 新しいオブジェクトを登録
			Register(newObject);

			// 新しいオブジェクトリストに追加
			targets.push_back(newObject.get());
		}
	}
#endif // 0


	//GameObject* rootObject = nullptr;
	GameObject* rootObject = Instantiate(j);

#if 0
	// 親子関係の復元
	for (size_t i = 0; i < j.size(); ++i)
	{
		const json& item = j[i];
		if (item.contains("name") && item["name"].is_string())
		{
			std::string name = item["name"];
			GameObject* newObject = targets[i];
			// 親IDが設定されている場合、親オブジェクトを探して設定
			if (newObject->pendingParentID.IsValid())
			{
				GameObject* parent = FindInObjects(newObject->pendingParentID);
				if (parent)
				{
					newObject->SetParent(parent);
				}
				else
				{
					newObject->SetParent(nullptr);
				}
				newObject->pendingParentID = ObjectId::Invalid();
			}
			// コンポーネントのデシリアライズ
			newObject->DeserializeComponents(item);
		}
	}


	// ルートオブジェクトを取得
	for (GameObject* obj : targets)
	{
		if (obj->parent == nullptr)
		{
			rootObject = obj;
			break;
		}
	}
#endif // 0

	// ルートオブジェクトが見つかった場合、ルートオブジェクトの親に、複製元の親を設定
	if (rootObject && original)
	{
		rootObject->SetParent(original->GetParent());
		// アクティブ状態の更新
		rootObject->RefreshActiveInHierarchy();
	}

	// ルートオブジェクトを返す
	return rootObject;
}

GameObject* ObjectManager::Instantiate(const json& j)
{
	if (j.is_null()) return nullptr;
	std::unordered_map<ObjectId, ObjectId> idMap; // 古いIDから新しいIDへのマッピング
	// 新しいIDを生成してマッピングを作成
	for (const json& item : j)
	{
		if (item.contains("id"))
		{
			// 古いIDを取得（整数型か文字列型のどちらかで保存されている可能性があるため両方に対応）
			ObjectId oldID = item["id"].is_number_integer()
				? ObjectId::FromLegacy(item["id"].get<int>()) // 整数型の場合は旧形式から変換
				: ObjectId::FromString(item["id"].get<std::string>()); // 文字列型の場合はそのまま変換
			ObjectId newID = ObjectId::Generate(); // 新しいIDを生成
			idMap[oldID] = newID;
		}
		if (item.contains("components"))
		{
			for (const auto& compJson : item["components"]) {
				if (compJson.contains("id")) {
					ObjectId oldCompID = compJson["id"].is_number_integer()
						? ObjectId::FromLegacy(compJson["id"].get<int>())
						: ObjectId::FromString(compJson["id"].get<std::string>());
					idMap[oldCompID] = ObjectId::Generate();
				}
			}
		}
	}
	// 親IDを変換する関数
	std::function<ObjectId(ObjectId)> convertParent = [&](ObjectId oldParentID) {
		auto it = idMap.find(oldParentID);
		if (it != idMap.end()) {
			return it->second;
		}
		return ObjectId::Invalid(); // 見つからなかった場合
		};
	GameObject* rootObject = nullptr;
	std::vector<GameObject*> targets;
	// オブジェクトを作成してデシリアライズ
	for (const json& item : j)
	{
		if (item.contains("name") && item["name"].is_string())
		{
			std::shared_ptr<GameObject> newObject = std::make_shared<GameObject>();
			newObject->Deserialize(item);
			// ユニークな名前に変更
			newObject->name = newObject->MakeUniqueName(newObject->GetName());
			// 新しいIDを設定
			ObjectId oldID = newObject->GetId();
			newObject->SetId(idMap[oldID]);
			// 親IDを変換して設定
			newObject->pendingParentID = convertParent(newObject->pendingParentID);
			// 新しいオブジェクトを登録
			Register(newObject);
			// ルートオブジェクトの設定
			if (!newObject->pendingParentID.IsValid())
			{
				rootObject = newObject.get();
			}
			// 新しいオブジェクトリストに追加
			targets.push_back(newObject.get());
		}
	}
	// 親子関係の復元
	for (size_t i = 0; i < j.size(); ++i)
	{
		const json& item = j[i];
		if (item.contains("name") && item["name"].is_string())
		{
			GameObject* newObject = targets[i];
			// 親IDが設定されている場合、親オブジェクトを探して設定
			if (newObject->pendingParentID.IsValid())
			{
				GameObject* parent = FindInObjects(newObject->pendingParentID);
				if (parent)
				{
					newObject->SetParent(parent);
				}
				else
				{
					newObject->SetParent(nullptr);
				}
				newObject->pendingParentID = ObjectId::Invalid(); // 親IDをリセット
			}
			// コンポーネントのデシリアライズ（idMapを渡す）
			newObject->DeserializeComponents(item, idMap);
		}
	}

	// リフレクションプロパティ内の ObjectId の差し替え（Fixup）
	for (GameObject* obj : targets)
	{
		auto replaceObjectIds = [&](Object* targetObj) {
			if (!targetObj) return;
			std::vector<std::string> toVisit = { targetObj->GetTypeName() };
			std::unordered_set<std::string> visited;
			while (!toVisit.empty()) {
				std::string currentClass = toVisit.front();
				toVisit.erase(toVisit.begin());
				if (visited.count(currentClass)) continue;
				visited.insert(currentClass);

				if (auto* meta = ReflectionRegistry::FindClass(currentClass)) {
					for (const auto& prop : meta->properties) {
						if (prop.type == "ObjectId") {
							char* base = reinterpret_cast<char*>(targetObj);
							ObjectId* idPtr = reinterpret_cast<ObjectId*>(base + prop.offset);
							auto it = idMap.find(*idPtr);
							if (it != idMap.end()) {
								*idPtr = it->second; // 参照先が複製対象の場合は新しいIDに書き換え
							}
						}
					}
					for (const auto& base : meta->bases) {
						if (!base.empty()) toVisit.push_back(base);
					}
				}
			}
		};

		replaceObjectIds(obj);
		for (const auto& comp : obj->GetAllComponents()) {
			replaceObjectIds(comp.get());
		}
	}

	if (rootObject)
	{
		// アクティブ状態の更新
		rootObject->RefreshActiveInHierarchy();
	}

	// 生成したコンポーネントを即時にキャッシュに登録
	for (GameObject* obj : targets)
	{
		for (const auto& comp : obj->GetAllComponents())
		{
			componentCacheMap[comp->GetId()] = comp;
		}
	}

	return rootObject;
}

void ObjectManager::SaveGameObject(GameObject* object, const std::string& filePath)
{
	if (!object) return;
	
	std::vector<GameObject*> targets;
	std::function<void(GameObject*)> collectObjects = [&](GameObject* obj)
		{
			targets.push_back(obj);
			for (GameObject* child : obj->GetChildren()) {
				collectObjects(child);
			}
		};
	collectObjects(object);

	
	// オブジェクトをシリアライズ
	json j;
	for (GameObject* obj : targets) {
		json objJson = obj->Serialize();
		if (!objJson.is_null()) {
			j.push_back(objJson);
		}
	}
	// JSONデータをファイルに保存
	JsonFileHandler::SaveJsonToFile(j, filePath, JsonIOFormat::Binary);
}
