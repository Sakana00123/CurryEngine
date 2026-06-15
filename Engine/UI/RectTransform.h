#pragma once
#include "Engine/Core/Component.h"
#include "Engine/Core/Transform.h"

/**
 * @file
 * @brief UI 用の 2D 変換を扱う `Transform` 派生クラス。
 * @details アンカー、ピボット、オフセットを用いたレイアウト計算と、
 *          画面座標・NDC 座標の相互変換、当たり判定（矩形/回転矩形）を提供します。
 */

class RectTransform : public Transform
{
	C_REFLECT(RectTransform)
public:
	/** @brief アンカー基準の座標（スクリーン座標系）。*/
	C_PROPERTY(CurryEngine::PropertyAttributes::Getter("GetAnchoredPosition"), CurryEngine::PropertyAttributes::Setter("SetAnchoredPosition"), CurryEngine::PropertyAttributes::Speed(1.0f))
	Vector2 anchoredPosition{ 0,0 };
	//DirectX::Vector2 sizeDelta{};

	/** @brief ワールド座標のピボット位置。*/
	Vector2 worldPos{};//pivot位置
	/** @brief ワールド座標系でのサイズ。*/
	Vector2 worldSize{};
	
	/** @brief 回転後の四隅（スクリーン座標）。*/
	Vector2 topLeft{};
	Vector2 topRight{};
	Vector2 bottomLeft{};
	Vector2 bottomRight{};
	/** @brief 回転前（軸回り未回転）の四隅（スクリーン座標）。*/
	Vector2 unrotatedTopLeft{};
	Vector2 unrotatedTopRight{};
	Vector2 unrotatedBottomLeft{};
	Vector2 unrotatedBottomRight{};
	/** @brief インスペクタ表示の開閉状態（ImGui 用）。*/
	bool isOpen = false;//ImGui用

	/** @brief アンカーからの左下オフセット（ピクセル）。*/
	Vector2 offsetMin = { 0.f, 0.f };// anchorMinからのオフセット（ピクセル）
	/** @brief アンカーからの右上オフセット（ピクセル）。*/
	Vector2 offsetMax = { 0.f, 0.f };// anchorMaxからのオフセット（ピクセル）

	/** @brief ワールド空間での回転角（ラジアン）。*/
	float worldAngle = 0.0f;
public:
	/** @brief ローカル回転角（ラジアン）。*/
	C_PROPERTY(CurryEngine::PropertyAttributes::Getter("GetAngle"), CurryEngine::PropertyAttributes::Setter("SetAngle"), CurryEngine::PropertyAttributes::Speed(1.0f))
	float angle = 0.0f;
	/** @brief ローカルサイズ（ピクセル）。*/
	C_PROPERTY(CurryEngine::PropertyAttributes::Getter("GetWorldSize"), CurryEngine::PropertyAttributes::Setter("SetSize"), CurryEngine::PropertyAttributes::Speed(1.0f))
	Vector2 size{};
	/** @brief ピボット（0-1、左上(0,0)、右下(1,1)）。*/
	C_PROPERTY(CurryEngine::PropertyAttributes::Getter("GetPivot"), CurryEngine::PropertyAttributes::Setter("SetPivot"), CurryEngine::PropertyAttributes::Speed(0.1f))
	Vector2 pivot = { 0.5f, 0.5f };
	/** @brief 最小アンカー（0-1、親左上(0,0)、右下(1,1)）。*/
	C_PROPERTY(CurryEngine::PropertyAttributes::HideInInspector)
	Vector2 anchorMin = { 0.5f, 0.5f };// 0～1（親の左上が (0,0)、右下が (1,1)）
	/** @brief 最大アンカー（`anchorMin==anchorMax` なら固定）。*/
	C_PROPERTY(CurryEngine::PropertyAttributes::HideInInspector)
	Vector2 anchorMax = { 0.5f, 0.5f };// anchorMin == anchorMax なら固定位置
	/** @brief ソーティングオーダー。数値が大きいほど前面に描画されることを想定。*/
	C_PROPERTY()
	int localSortingOrder = 0; // 親子関係を考慮しないローカルなソーティングオーダー。描画順は親のソーティングオーダー + localSortingOrder で決定される。
public:
	/** @brief コンストラクタ。*/
	RectTransform() : size({ 100,100 }) { position = { 0,0,0 }; }
	/** @brief デストラクタ。*/
	virtual ~RectTransform() override = default;

	/** @brief 初期化処理。*/
	void Start() override;

	/**
	 * @brief 親の `RectTransform` を取得します。
	 * @return 親の `RectTransform`。存在しない場合は `nullptr`。
	 */
	RectTransform* GetParent() const;

	/**
	 * @brief 変換の更新処理。
	 * @param elapsedTime 経過時間（秒）。
	 */
	void Update(float elapsedTime) override;

#ifdef USE_IMGUI
	/**
	 * @brief インスペクタ用プロパティ描画。
	 */
	void DrawProperty(const PropertyDrawContext& context) override;
#endif // USE_IMGUI

	/**
	 * @brief アンカープリセット UI を描画します。
	 * @return 値が変更された場合は `true`。
	 */
	bool DrawAnchorPreset();

	/**
	 * @brief 指定点が矩形内（回転考慮）にあるかを判定します。
	 * @param point スクリーン座標の判定点。
	 * @return 矩形内であれば `true`。
	 */
	bool Contains(const Vector2& point);

	/**
	 * @brief 任意四辺形内判定。
	 * @param p 判定点。
	 * @param a,b,c,d 四辺形の頂点（スクリーン座標、順序は一貫していること）。
	 * @return 内部にあれば `true`。
	 */
	bool PointInQuad(Vector2 p, Vector2 a, Vector2 b, Vector2 c, Vector2 d);

	/** @brief 左上のスクリーン座標。*/
	Vector2 TopLeft() const { return topLeft; }
	/** @brief 右上のスクリーン座標。*/
	Vector2 TopRight() const { return topRight; }
	/** @brief 左下のスクリーン座標。*/
	Vector2 BottomLeft() const { return bottomLeft; }
	/** @brief 右下のスクリーン座標。*/
	Vector2 BottomRight() const { return bottomRight; }

	/** @brief 回転前の左上。*/
	Vector2 UnrotatedTopLeft() const { return unrotatedTopLeft; }
	/** @brief 回転前の右上。*/
	Vector2 UnrotatedTopRight() const { return unrotatedTopRight; }
	/** @brief 回転前の左下。*/
	Vector2 UnrotatedBottomLeft() const { return unrotatedBottomLeft; }
	/** @brief 回転前の右下。*/
	Vector2 UnrotatedBottomRight() const { return unrotatedBottomRight; }

	/**
	 * @brief 自身のアンカーポジションを NDC に変換して取得します。
	 * @return NDC 座標の 2D ベクトル。
	 */
	Vector2 ToNDC() const;
	/**
	 * @brief スクリーン座標を NDC に変換します。
	 */
	static Vector2 ScreenToNDC(const Vector2& anchoredPosition);
	/**
	 * @brief NDC をスクリーン座標に変換します。
	 */
	static Vector2 NDCToScreen(const Vector2& ndc);

	/**
	 * @brief ピボットのワールド座標を取得します。
	 */
	Vector2 GetWorldPosition();
	/**
	 * @brief アンカーポジション（スクリーン座標）を取得します。
	 */
	C_FUNCTION()
	Vector2 GetAnchoredPosition() const;

	/**
	 * @brief ワールド座標系でのサイズを取得します。
	 */
	C_FUNCTION()
	Vector2 GetWorldSize();

	/**
	 * @brief ワールド回転角（ラジアン）を取得します。
	 */
	C_FUNCTION()
	float GetWorldAngle() const;

	/** @brief 最大アンカーを設定。*/
	C_FUNCTION()
	void SetAnchorMax(const Vector2& max) { anchorMax = max; }
	/** @brief 最小アンカーを設定。*/
	C_FUNCTION()
	void SetAnchorMin(const Vector2& min) { anchorMin = min; }
	/** @brief アンカーポジションを設定（スクリーン座標）。*/
	C_FUNCTION()
	void SetAnchoredPosition(const Vector2& pos) { anchoredPosition = pos; }

	/**
	 * @brief 指定したアンカー基準位置にピボットを合わせて配置します。
	 * @param targetAnchor 親のアンカー位置（0-1、左上(0,0)、右下(1,1)）。
	 * @param targetAnchoredPos 親のアンカーポジション（スクリーン座標）。
	 */
	void SetAnchoredPositionByAnchor(const Vector2& targetAnchor, const Vector2& targetAnchoredPos);

	/** @brief 指定した `Transform` の位置にピボットを合わせて配置します。
	 *  @param transform 対象の `Transform`。
	 *  @details Z 座標は無視されます。
	 */
	void SetAnchoredPositionByTransform(Transform* transform);

	/** @brief ローカル回転角を設定（度）。*/
	C_FUNCTION()
	void SetAngle(float angle) { this->angle = angle; }

	/** @brief ローカル回転角を取得（度）。*/
	C_FUNCTION()
	float GetAngle() const { return angle; }

	/** @brief サイズを設定（ピクセル）。*/
	C_FUNCTION()
	void SetSize(const Vector2& size) { this->size = size; }

	/** @brief ピボットを設定（0-1）。*/
	C_FUNCTION()
	void SetPivot(const Vector2& pivot) { this->pivot = pivot; }
	/** @brief ピボットを取得。*/
	C_FUNCTION()
	Vector2 GetPivot() const { return pivot; }

	//void SetSizeDelta(const Vector2& delta) { sizeDelta = delta; }
	//Vector2 GetSizeDelta() const { return sizeDelta; }

	/** @brief 左オフセットを設定（ピクセル）。*/
	C_FUNCTION()
	void SetLeft(float left) { offsetMin.x = left; }
	/** @brief 右オフセットを設定（ピクセル）。*/
	C_FUNCTION()
	void SetRight(float right) { offsetMax.x = right; }
	/** @brief 上オフセットを設定（ピクセル）。*/
	C_FUNCTION()
	void SetTop(float top) { offsetMin.y = top; }
	/** @brief 下オフセットを設定（ピクセル）。*/
	C_FUNCTION()
	void SetBottom(float bottom) { offsetMax.y = bottom; }

	/** @brief 左オフセット（ピクセル）。*/
	C_FUNCTION()
	float GetLeft() const { return offsetMin.x; }
	/** @brief 右オフセット（ピクセル）。*/
	C_FUNCTION()
	float GetRight() const { return offsetMax.x; }
	/** @brief 上オフセット（ピクセル）。*/
	C_FUNCTION()
	float GetTop() const { return offsetMin.y; }
	/** @brief 下オフセット（ピクセル）。*/
	C_FUNCTION()
	float GetBottom() const { return offsetMax.y; }

private:
	/**
	 * @brief 点を中心点周りに回転します。
	 * @param point 対象点（入出力）。
	 * @param center 回転の中心。
	 * @param angle 回転角（度）。
	 */
	void Rotate(Vector2& point, Vector2 center, float angle);
};