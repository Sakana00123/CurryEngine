#pragma once
#include "Graphic.h"
#include <map>
#include <string>
#include <fstream>

#include "Engine/Resources/Texture.h"
#include "Image.h"
#include "Engine/Utils/stdUtiles.h"

class Text : public Graphic
{
    C_REFLECT(Text)

    // -----------------------------------------------------------------------
    // 内部型
    // -----------------------------------------------------------------------
private:
    /// @brief .fntファイルから読み込んだ1文字分の情報
    struct Character
    {
        int x, y, width, height;
        int xoffset, yoffset;
        int xadvance;
        int page, chnl;
    };

    /// @brief レイアウト計算済みグリフ情報（スクリーン座標系: +x右 / +y下）
    struct GlyphLayout
    {
        float x;                ///< UnrotatedTopLeftからの相対X
        float y;                ///< UnrotatedTopLeftからの相対Y
        wchar_t ch;
        const Character* glyph;
        size_t charIndex;       ///< text[] 上のインデックス（CharModifier参照用）
    };

    /// @brief 頂点バッファの1頂点
    struct Vertex
    {
        DirectX::XMFLOAT3 position;
        DirectX::XMFLOAT4 color;
        DirectX::XMFLOAT2 texcoord;
    };

    // -----------------------------------------------------------------------
    // 公開型
    // -----------------------------------------------------------------------
public:
    /// @brief テキストの配置基準
    enum class Alignment
    {
        TopLeft,    TopCenter,    TopRight,
        MiddleLeft, MiddleCenter, MiddleRight,
        BottomLeft, BottomCenter, BottomRight,
        COUNT
    };

    /// @brief 横方向のはみ出し動作
    enum class HorizontalOverflow
    {
        Wrap,
        Overflow,
        COUNT
    };

    // -----------------------------------------------------------------------
    //  CharModifier ― 文字ごとの演出パラメータ
    // -----------------------------------------------------------------------

    /**
     * @brief 1文字に適用する演出オフセット情報。
     *
     * すべてのフィールドはベース値への**加算 / 乗算**として扱われます。
     * デフォルト値は「何も変化しない」状態を表します。
     *
     * @note 座標系は +x 右 / +y 下 のスクリーン座標系です。
     * @note rotation は文字クワッドの中心を軸に、Text 全体の worldAngle に加算されます。
     * @note colorMul は Text::color に成分ごとに乗算されます。
     */
    struct CharModifier
    {
        Vector2 posOffset = { 0.0f, 0.0f };             ///< 位置オフセット（ピクセル）
        Vector2 scale     = { 1.0f, 1.0f };             ///< スケール乗数（1,1 = 変化なし）
        float   rotation  =   0.0f;                     ///< 追加回転（度、文字クワッド中心基準）
        Color   colorMul  = { 1.0f, 1.0f, 1.0f, 1.0f };///< 色乗算（1,1,1,1 = 変化なし）

        /// @brief デフォルト状態（変化なし）かどうかを判定する
        bool IsIdentity() const
        {
            return posOffset.x == 0.0f && posOffset.y == 0.0f
                && scale.x    == 1.0f  && scale.y    == 1.0f
                && rotation   == 0.0f
                && colorMul.r == 1.0f  && colorMul.g == 1.0f
                && colorMul.b == 1.0f  && colorMul.a == 1.0f;
        }
    };

    // -----------------------------------------------------------------------
    // 公開メンバ
    // -----------------------------------------------------------------------
public:
    std::string  fontFilePath  = "./Assets/Fonts/madoufmg.fnt";
    std::wstring text          = L"Text";
    float        fontSize      = 64.0f;
    float        lineSpacing   = 1.0f;
    Color        color         { 1, 1, 1, 1 };
    int          characterLimit = 256;  ///< 0 = 無制限
    bool         autoSize      = false; ///< サイズをテキストに自動フィット
    bool         bestFit       = false; ///< フォントサイズを矩形にフィット

    Alignment          alignment         = Alignment::TopLeft;
    HorizontalOverflow horizontalOverflow = HorizontalOverflow::Overflow;

    // -----------------------------------------------------------------------
    //  CharModifier API
    // -----------------------------------------------------------------------
public:
    /**
     * @brief 指定インデックスの文字に CharModifier を設定する。
     *
     * 毎フレーム上書きする演出用途を想定しています（例: Update() 内で呼ぶ）。
     * インデックスが範囲外の場合は何もしません。
     *
     * @param index text[] 上のインデックス
     * @param mod   設定する CharModifier
     */
    void SetCharModifier(size_t index, const CharModifier& mod);

    /**
     * @brief 指定インデックスの CharModifier を取得する。
     * @param index text[] 上のインデックス
     * @return 対応する CharModifier の const 参照（範囲外は identity を返す）
     */
    const CharModifier& GetCharModifier(size_t index) const;

    /**
     * @brief すべての CharModifier を identity（変化なし）にリセットする。
     */
    void ResetCharModifiers();

    /**
     * @brief modifiers を text の長さに合わせてリサイズする。
     *
     * SetText() 後は自動的に呼ばれます。
     * text を直接書き換えた場合は手動で呼んでください。
     */
    void ResizeModifiers();

    // -----------------------------------------------------------------------
    // 公開API
    // -----------------------------------------------------------------------
public:
    Text()          = default;
    ~Text() override = default;

    // --- テキスト ---
    void                SetText(const std::wstring& newText);
    const std::wstring& GetText() const { return text; }

	void InsertText(size_t index, const std::wstring& newText);
	void EraseText(size_t index, size_t count);

    // --- フォントサイズ ---
    void  SetFontSize(float newSize)   { fontSize = newSize; }
    float GetFontSize() const          { return fontSize; }

    // --- 色 ---
    void         SetColor(const Color& newColor) { color = newColor; }
    const Color& GetColor() const                { return color; }

    // --- 配置 ---
    void      SetAlignment(Alignment newAlignment)  { alignment = newAlignment; }
    Alignment GetAlignment() const                   { return alignment; }

    // --- オーバーフロー ---
    void               SetHorizontalOverflow(HorizontalOverflow v) { horizontalOverflow = v; }
    HorizontalOverflow GetHorizontalOverflow() const               { return horizontalOverflow; }

    // --- 自動サイズ ---
    void SetAutoSize(bool enable) { autoSize = enable; }
    bool GetAutoSize() const      { return autoSize; }

    // --- ベストフィット ---
    void SetBestFit(bool enable) { bestFit = enable; }
    bool GetBestFit() const      { return bestFit; }

    // --- 文字数上限 ---
    void SetCharacterLimit(int limit);
    int  GetCharacterLimit() const { return characterLimit; }

    // --- Graphic インタフェース ---
    void Initialize() override;
    void Begin(RenderContext* rtx) override;
    void Draw(RenderContext* rtx) override;
    void End(RenderContext* rtx) override;
    void DrawProperty() override;

    // --- シリアライズ ---
    json Serialize()  const override;
    void Deserialize(const json& j) override;

    // --- カーソル ---
    /// @brief カーソル位置のスクリーン座標を取得する（ワールド座標）
    void GetCursorPos(size_t cursorPos, _Out_ float& x, _Out_ float& y);

	/// @brief スクリーン座標からカーソル位置を取得する（ワールド座標）
    size_t GetCursorIndexFromPoint(float screenX, float screenY);

    // --- レイアウト ---
    void  PerformLayout(float maxWidth);
    float ComputeLineWidth(const std::wstring& line);
    float ComputeLineHeight();
    float CalcAlignedX(float lineWidth);
    float CalcAlignedY();

    /// @brief テキストを行ごとに分割（L"\n"のみ改行）
    std::vector<std::wstring> SplitTextToLines() const;
    /// @brief テキストを行ごとに分割（L"\n" + Wrap対応）
    std::vector<std::wstring> SplitTextToLines(float maxWidth);

    // -----------------------------------------------------------------------
    // 内部実装
    // -----------------------------------------------------------------------
private:
    /// @brief .fntファイルとシェーダーを読み込む（Deserialize/初期化から呼ぶ）
    void Setup(const std::string& fontFilePath,
               const char* customPsName = nullptr,
               const char* customVsName = nullptr);

    /// @brief 頂点バッファを確保する
    void SetupBuffer(size_t vertexCapacity);

    /**
     * @brief 1文字分の頂点を vertices に追加する。
     * @param c     フォントの文字情報
     * @param x     描画基点X（スクリーン座標、+x右）
     * @param y     描画基点Y（スクリーン座標、+y下）
     * @param mod   この文字に適用する CharModifier
	 * @param ref   Canvasの参照解像度（スケール算出用）
     * @param ctx   D3D11デバイスコンテキスト
     */
    void DrawCharacter(const Character& c, float x, float y,
                       const CharModifier& mod,
		               Vector2 ref,
                       ID3D11DeviceContext* ctx);

    /// @brief スクリーン座標系での2D回転（+x右 / +y下）
    void Rotate(float& x, float& y, float cx, float cy, float angleDeg);

	/// @brief Canvasの参照解像度を取得する
	Vector2 GetReferenceResolution() const;


    // --- 文字ごとの演出データ ---
    // text[i] に対応する CharModifier を保持する。
    // text と常に同じ長さを保つ（SetText / ResizeModifiers で管理）。
    std::vector<CharModifier> modifiers;

    /// @brief 範囲外アクセス用の identity（GetCharModifier の返却値）
    static const CharModifier kIdentityModifier;

    // --- フォントデータ ---
    struct FontData
    {
        std::map<wchar_t, Character> characters;
        std::wstring faceName;          ///< フォント名（face=）※dangling回避のためwstring保持
		std::wstring texturePath;       ///< テクスチャファイルパス（page=）
        float        fntSize = 1.0f;   ///< .fntのsize値（スケール算出の基準）
        int          bold = 0;
        int          italic = 0;
        int          lineHeight = 0;
    };
	static inline std::map<std::string, FontData> fontDataCache; ///< フォントファイルパスごとのキャッシュ

    std::map<wchar_t, Character> characters;
    std::wstring faceName;          ///< フォント名（face=）※dangling回避のためwstring保持
    float        fntSize  = 1.0f;   ///< .fntのsize値（スケール算出の基準）
    int          bold     = 0;
    int          italic   = 0;
    int          lineHeight = 0;

    // --- レイアウトキャッシュ ---
    std::vector<GlyphLayout> layoutedGlyphs;
    float layoutWidth  = 0.0f;
    float layoutHeight = 0.0f;

    // --- D3D11リソース ---
    std::wstring filePath;
    D3D11_TEXTURE2D_DESC texture2dDesc{};
    size_t maxVertices = 0;

    std::vector<Vertex>                               vertices;
    Microsoft::WRL::ComPtr<ID3D11VertexShader>        vertexShader;
    Microsoft::WRL::ComPtr<ID3D11PixelShader>         pixelShader;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>  shaderResourceView;
    Microsoft::WRL::ComPtr<ID3D11InputLayout>         inputLayout;
    Microsoft::WRL::ComPtr<ID3D11Buffer>              vertexBuffer;

    // --- ImGui用 ---
    static constexpr const char* kAlignmentStr[] = {
        "TopLeft",    "TopCenter",    "TopRight",
        "MiddleLeft", "MiddleCenter", "MiddleRight",
        "BottomLeft", "BottomCenter", "BottomRight"
    };
    static constexpr const char* kHorizontalOverflowStr[] = {
        "Wrap",
        "Overflow"
    };

    friend class InputField;
    //friend class TextMesh;
    friend class GameObjectFactory;
};
