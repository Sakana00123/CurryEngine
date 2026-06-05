#include "pch.h"
#include "Text.h"
//#include "Canvas.h"
//#include "CanvasScaler.h"

#ifdef USE_IMGUI
#include "Engine/Scenes/Scene.h"
#endif // USE_IMGUI


REGISTER_COMPONENT_WITH_ATTRIBUTES(Text, "UI",
    ComponentAttributes::DisallowMultiple | ComponentAttributes::ExecuteInEditMode, {});

// identity の定義（宣言は .h 側の static const）
const Text::CharModifier Text::kIdentityModifier{};

// =============================================================================
//  Setup / 初期化
// =============================================================================

void Text::Setup(const std::string& fontFilePath,
                 const char* customPsName,
                 const char* customVsName)
{
    auto device = Graphics::GetDevice();

	std::filesystem::path fontDir = std::filesystem::path(fontFilePath).parent_path();

	if (fontDataCache.contains(fontFilePath))
    {
        // キャッシュから読み込み
        const FontData& cachedData = fontDataCache[fontFilePath];
        characters = cachedData.characters;
        faceName = cachedData.faceName;
        fntSize = cachedData.fntSize;
        bold = cachedData.bold;
        italic = cachedData.italic;
        lineHeight = cachedData.lineHeight;
        filePath = cachedData.texturePath;
    }
	else
    {
        // --- .fnt ファイルのパース ---
        std::ifstream file(fontFilePath);
        if (!file)
        {
            std::u8string u8FontFilePath(fontFilePath.begin(), fontFilePath.end());
            LOG_WARNING(u8"フォントファイルが開けませんでした: " + u8FontFilePath);
            return;
        }

        std::string line;
        while (std::getline(file, line))
        {
            if (line.find("info") != std::string::npos)
            {
                char name[256] = {};
                sscanf_s(line.c_str(),
                    "info face=\"%255[^\"]\" size=%f bold=%d italic=%d",
                    name, (unsigned)_countof(name), &fntSize, &bold, &italic);

                faceName = std::wstring(name, name + strlen(name));
            }
            else if (line.find("common") != std::string::npos)
            {
                sscanf_s(line.c_str(), "common lineHeight=%d", &lineHeight);
            }
            else if (line.find("page id=") != std::string::npos)
            {
                char fileName[256] = {};
                sscanf_s(line.c_str(), "page id=%*d file=\"%255[^\"]\"",
                    fileName, (unsigned)_countof(fileName));
                std::string texPath = (fontDir / std::string(fileName)).string();
                filePath = std::wstring(texPath.begin(), texPath.end());
            }
            else if (line.find("char id=") != std::string::npos)
            {
                Character c{};
                unsigned int id = 0;
                sscanf_s(line.c_str(),
                    "char id=%u x=%d y=%d width=%d height=%d "
                    "xoffset=%d yoffset=%d xadvance=%d page=%d chnl=%d",
                    &id,
                    &c.x, &c.y, &c.width, &c.height,
                    &c.xoffset, &c.yoffset, &c.xadvance, &c.page, &c.chnl);
                characters[static_cast<wchar_t>(id)] = c;
            }
        }

		// キャッシュに保存
        fontDataCache[fontFilePath] = FontData{
            characters,
            faceName,
			filePath,
            fntSize,
            bold,
            italic,
            lineHeight
		};
    }

    SetCharacterLimit(characterLimit);

    // --- シェーダー / 入力レイアウト ---
    HRESULT hr = S_OK;

    D3D11_INPUT_ELEMENT_DESC inputElementDesc[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,       0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };
	std::string dir = EnginePaths::ShadersDataDir;
    hr = CreateVertexShaderFromCSO(device, customVsName ? customVsName : (dir + "sprite_vs.cso").c_str(),
        vertexShader.ReleaseAndGetAddressOf(),
        inputLayout.ReleaseAndGetAddressOf(),
        inputElementDesc, _countof(inputElementDesc));
    _ASSERT_EXPR(SUCCEEDED(hr), HrTrace(hr));

    hr = CreatePixelShaderFromCSO(device, customPsName ? customPsName : (dir + "sprite_ps.cso").c_str(),
        pixelShader.ReleaseAndGetAddressOf());
    _ASSERT_EXPR(SUCCEEDED(hr), HrTrace(hr));

    hr = LoadTextureFromFile(device, filePath.c_str(),
        shaderResourceView.ReleaseAndGetAddressOf(), &texture2dDesc);
    _ASSERT_EXPR(SUCCEEDED(hr), HrTrace(hr));
}

void Text::SetupBuffer(size_t vertexCapacity)
{
    if (vertexCapacity == 0) return;

    maxVertices = vertexCapacity;

    auto device = Graphics::GetDevice();
    std::unique_ptr<Vertex[]> initData{ std::make_unique<Vertex[]>(maxVertices) };

    D3D11_BUFFER_DESC bd{};
    bd.ByteWidth      = static_cast<UINT>(sizeof(Vertex) * maxVertices);
    bd.Usage          = D3D11_USAGE_DYNAMIC;
    bd.BindFlags      = D3D11_BIND_VERTEX_BUFFER;
    bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    D3D11_SUBRESOURCE_DATA sd{};
    sd.pSysMem = initData.get();

    HRESULT hr = device->CreateBuffer(&bd, &sd, vertexBuffer.ReleaseAndGetAddressOf());
    _ASSERT_EXPR(SUCCEEDED(hr), HrTrace(hr));
}

void Text::Initialize()
{
    // Deserialize から呼ばれる Setup で初期化済み
}

// =============================================================================
//  CharModifier API
// =============================================================================

void Text::SetCharModifier(size_t index, const CharModifier& mod)
{
    if (index >= modifiers.size()) return;
    modifiers[index] = mod;
}

const Text::CharModifier& Text::GetCharModifier(size_t index) const
{
    if (index >= modifiers.size()) return kIdentityModifier;
    return modifiers[index];
}

void Text::ResetCharModifiers()
{
    std::fill(modifiers.begin(), modifiers.end(), CharModifier{});
}

void Text::ResizeModifiers()
{
    // 縮小時は末尾を切り捨て、拡張時は identity で埋める
    modifiers.resize(text.size(), CharModifier{});
}

// =============================================================================
//  プロパティ setter
// =============================================================================

void Text::SetText(const std::wstring& newText)
{
    text = newText;
    if (characterLimit > 0 && text.length() > static_cast<size_t>(characterLimit))
    {
        text = text.substr(0, characterLimit);
        LOG_WARNING(u8"テキストの文字数が上限を超えています。切り詰めて表示します。");
    }
    ResizeModifiers();
}

void Text::InsertText(size_t index, const std::wstring& insertText)
{
    if (index > text.size()) index = text.size();
    text.insert(index, insertText);
    if (characterLimit > 0 && text.length() > static_cast<size_t>(characterLimit))
    {
        text = text.substr(0, characterLimit);
        LOG_WARNING(u8"テキストの文字数が上限を超えています。切り詰めて表示します。");
    }
    ResizeModifiers();
}

void Text::EraseText(size_t index, size_t count)
{
    if (index >= text.size()) return;
    text.erase(index, count);
    ResizeModifiers();
}

void Text::SetCharacterLimit(int limit)
{
    characterLimit = limit;
    if (characterLimit > 0 && text.length() > static_cast<size_t>(characterLimit))
    {
        text = text.substr(0, characterLimit);
        LOG_WARNING(u8"テキストの文字数が上限を超えています。切り詰めて表示します。");
    }

    const size_t capacity = (limit > 0)
        ? static_cast<size_t>(limit) * 6
        : static_cast<size_t>(2048) * 6;
    SetupBuffer(capacity);
}

// =============================================================================
//  描画
// =============================================================================

void Text::Begin(RenderContext* /*rtx*/)
{
    vertices.clear();
}

void Text::Draw(RenderContext* rtx)
{
	if (text.empty() || vertexBuffer == nullptr) return;

    ID3D11DeviceContext* ctx = rtx->immediateContext;
    RectTransform* rect = GetRectTransform();

    // レイアウト計算（+x右 / +y下 スクリーン座標系）
    PerformLayout(rect->size.x);

    // グリフを頂点バッファへ
    const Vector2 origin = rect->UnrotatedTopLeft();
	const Vector2 ref = GetReferenceResolution();
    for (const auto& gl : layoutedGlyphs)
    {
        const CharModifier& mod = GetCharModifier(gl.charIndex);
        DrawCharacter(*gl.glyph,
			origin.x + gl.x,
			origin.y + gl.y,
            mod,
            ref,
            ctx);
    }

    // 頂点バッファを転送
    HRESULT hr = S_OK;
    D3D11_MAPPED_SUBRESOURCE mapped{};
    hr = ctx->Map(vertexBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
    _ASSERT_EXPR(SUCCEEDED(hr), HrTrace(hr));

    const size_t vertexCount = vertices.size();
    _ASSERT_EXPR(vertexCount <= maxVertices, L"頂点数がバッファの最大数を超えています。");
    if (mapped.pData)
    {
        memcpy_s(mapped.pData, maxVertices * sizeof(Vertex),
            vertices.data(), vertexCount * sizeof(Vertex));
    }
    ctx->Unmap(vertexBuffer.Get(), 0);

    UINT stride = sizeof(Vertex);
    UINT offset = 0;
    ctx->IASetVertexBuffers(0, 1, vertexBuffer.GetAddressOf(), &stride, &offset);
    ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    ctx->IASetInputLayout(inputLayout.Get());
    ctx->VSSetShader(vertexShader.Get(), nullptr, 0);
    ctx->PSSetShader(pixelShader.Get(), nullptr, 0);
    ctx->PSSetShaderResources(0, 1, shaderResourceView.GetAddressOf());

    ctx->Draw(static_cast<UINT>(vertexCount), 0);
}

void Text::End(RenderContext* /*rtx*/)
{
}

// =============================================================================
//  DrawCharacter ― 1文字分の頂点を生成
// =============================================================================

void Text::DrawCharacter(const Character& c, float x, float y,
                         const CharModifier& mod,
	                     Vector2 ref,
                         ID3D11DeviceContext* ctx)
{
    RectTransform* rect = GetRectTransform();

    // スケール（Text 全体のフォントスケール × 文字ごとのスケール乗数）
    const float baseScale = fontSize / fntSize;
	const float sx = baseScale * mod.scale.x;
	const float sy = baseScale * mod.scale.y;

    // オフセット・xoffset/yoffset 適用後のクワッドサイズ（+x右 / +y下）
	const float dx = x + static_cast<float>(c.xoffset) * baseScale + mod.posOffset.x;
	const float dy = y + static_cast<float>(c.yoffset) * baseScale + mod.posOffset.y;
    const float dw = static_cast<float>(c.width)  * sx;
    const float dh = static_cast<float>(c.height) * sy;

    // 文字クワッドの中心（文字ごと回転の軸）
    const float charCx = dx + dw * 0.5f;
    const float charCy = dy + dh * 0.5f;

    // 頂点 (スクリーン座標、回転前)
    float x0 = dx,      y0 = dy;        // left-top
    float x1 = dx + dw, y1 = dy;        // right-top
    float x2 = dx,      y2 = dy + dh;   // left-bottom
    float x3 = dx + dw, y3 = dy + dh;   // right-bottom

    // ① 文字ごとの追加回転（文字クワッド中心基準）
    if (mod.rotation != 0.0f)
    {
        Rotate(x0, y0, charCx, charCy, mod.rotation);
        Rotate(x1, y1, charCx, charCy, mod.rotation);
        Rotate(x2, y2, charCx, charCy, mod.rotation);
        Rotate(x3, y3, charCx, charCy, mod.rotation);
    }

    // ② RectTransform 全体の worldAngle（ピボット中心基準）
    const float worldAngle = rect->GetWorldAngle();
    if (worldAngle != 0.0f)
    {
		const float pivotCx = rect->GetWorldPosition().x;
		const float pivotCy = rect->GetWorldPosition().y;
        Rotate(x0, y0, pivotCx, pivotCy, worldAngle);
        Rotate(x1, y1, pivotCx, pivotCy, worldAngle);
        Rotate(x2, y2, pivotCx, pivotCy, worldAngle);
        Rotate(x3, y3, pivotCx, pivotCy, worldAngle);
    }

    // スクリーン → NDC（+y下 のスクリーン座標系を NDC +y上 に反転）
    /*UINT numVP = 1;
    D3D11_VIEWPORT vp{};
    ctx->RSGetViewports(&numVP, &vp);*/
    auto toNdcX = [&](float sx_) { return  2.0f * sx_ / ref.x  - 1.0f; };
    auto toNdcY = [&](float sy_) { return  1.0f - 2.0f * sy_ / ref.y; };

    x0 = toNdcX(x0); y0 = toNdcY(y0);
    x1 = toNdcX(x1); y1 = toNdcY(y1);
    x2 = toNdcX(x2); y2 = toNdcY(y2);
    x3 = toNdcX(x3); y3 = toNdcY(y3);

    // UV
    const float u0 = static_cast<float>(c.x)            / texture2dDesc.Width;
    const float v0 = static_cast<float>(c.y)            / texture2dDesc.Height;
    const float u1 = static_cast<float>(c.x + c.width)  / texture2dDesc.Width;
    const float v1 = static_cast<float>(c.y + c.height) / texture2dDesc.Height;

    // 色（Text::color × CharModifier::colorMul）
    const DirectX::XMFLOAT4 finalColor = {
        color.r * mod.colorMul.r,
        color.g * mod.colorMul.g,
        color.b * mod.colorMul.b,
        color.a * mod.colorMul.a,
    };

    // 2トライアングル (CW)
    vertices.push_back({ { x0, y0, 0 }, finalColor, { u0, v0 } });
    vertices.push_back({ { x1, y1, 0 }, finalColor, { u1, v0 } });
    vertices.push_back({ { x2, y2, 0 }, finalColor, { u0, v1 } });
    vertices.push_back({ { x2, y2, 0 }, finalColor, { u0, v1 } });
    vertices.push_back({ { x1, y1, 0 }, finalColor, { u1, v0 } });
    vertices.push_back({ { x3, y3, 0 }, finalColor, { u1, v1 } });
}

// スクリーン座標系 (+x右 / +y下) の2D回転
void Text::Rotate(float& x, float& y, float cx, float cy, float angleDeg)
{
    x -= cx;
    y -= cy;

    const float c = cosf(DirectX::XMConvertToRadians(angleDeg));
    const float s = sinf(DirectX::XMConvertToRadians(angleDeg));
    const float tx = x, ty = y;
    x = c * tx - s * ty;
    y = s * tx + c * ty;

    x += cx;
    y += cy;
}

Vector2 Text::GetReferenceResolution() const
{
    /*if (Canvas* canvas = GetCanvas())
    {
        if (auto* scaler = canvas->GetOwner()->GetComponent<CanvasScaler>())
        {
            return scaler->GetReferenceResolution();
        }
    }*/
    return Vector2(1920, 1080); // デフォルトの基準解像度
}

// =============================================================================
//  レイアウト
// =============================================================================

void Text::PerformLayout(float maxWidth)
{
    RectTransform* rect = GetRectTransform();
    layoutedGlyphs.clear();
    layoutWidth  = 0.0f;
    layoutHeight = 0.0f;

    const std::vector<std::wstring> lines =
        (horizontalOverflow == HorizontalOverflow::Overflow)
        ? SplitTextToLines()
        : SplitTextToLines(maxWidth);

    // 行ごとにグリフ位置を決定 (座標系: +x右 / +y下, 原点 = UnrotatedTopLeft)
    float y = 0.0f;
    size_t charIndex = 0; // text[] 上のインデックス（\n はカウントして進める）
    for (const auto& line : lines)
    {
        const float lineWidth    = ComputeLineWidth(line);
        const float alignOffsetX = CalcAlignedX(lineWidth);
        layoutWidth = (std::max)(layoutWidth, lineWidth);

        float x = 0.0f;
        for (wchar_t ch : line)
        {
            const Character& glyph = characters[ch];
            layoutedGlyphs.push_back({ x + alignOffsetX, y, ch, &glyph, charIndex });
            x += static_cast<float>(glyph.xadvance) * (fontSize / fntSize);
            ++charIndex;
        }
        // \n の分を進める（最終行以外）
        if (&line != &lines.back()) ++charIndex;
        y += ComputeLineHeight();
    }
    layoutHeight = y;

    // Y 方向の配置オフセットを全グリフに適用
    const float offsetY = CalcAlignedY();
    if (offsetY != 0.0f)
    {
        for (auto& gl : layoutedGlyphs)
            gl.y += offsetY;
    }

    if (autoSize)
    {
        rect->size.x = layoutWidth;
        rect->size.y = layoutHeight;
    }
    else if (bestFit)
    {
        if ((layoutWidth > rect->size.x || layoutHeight > rect->size.y) && fontSize > 1.0f)
        {
            fontSize -= 1.0f;
            PerformLayout(maxWidth);
        }
    }
}

float Text::ComputeLineWidth(const std::wstring& line)
{
    float width = 0.0f;
    for (wchar_t ch : line)
        width += static_cast<float>(characters[ch].xadvance) * (fontSize / fntSize);
    return width;
}

float Text::ComputeLineHeight()
{
    return static_cast<float>(lineHeight) * lineSpacing * (fontSize / fntSize);
}

float Text::CalcAlignedX(float lineWidth)
{
    const float rectWidth = GetRectTransform()->size.x;
    switch (alignment)
    {
    case Alignment::TopLeft:    case Alignment::MiddleLeft:    case Alignment::BottomLeft:
        return 0.0f;
    case Alignment::TopCenter:  case Alignment::MiddleCenter:  case Alignment::BottomCenter:
        return (rectWidth - lineWidth) * 0.5f;
    case Alignment::TopRight:   case Alignment::MiddleRight:   case Alignment::BottomRight:
        return rectWidth - lineWidth;
    default:
        return 0.0f;
    }
}

float Text::CalcAlignedY()
{
    const float rectHeight = GetRectTransform()->size.y;
    switch (alignment)
    {
    case Alignment::TopLeft:    case Alignment::TopCenter:    case Alignment::TopRight:
        return 0.0f;
    case Alignment::MiddleLeft: case Alignment::MiddleCenter: case Alignment::MiddleRight:
        return (rectHeight - layoutHeight) * 0.5f;
    case Alignment::BottomLeft: case Alignment::BottomCenter: case Alignment::BottomRight:
        return rectHeight - layoutHeight;
    default:
        return 0.0f;
    }
}

std::vector<std::wstring> Text::SplitTextToLines() const
{
    std::vector<std::wstring> lines;
    std::wstring current;
    for (wchar_t ch : text)
    {
        if (ch == L'\n') { lines.push_back(std::move(current)); current.clear(); }
        else             { current += ch; }
    }
    lines.push_back(std::move(current));
    return lines;
}

std::vector<std::wstring> Text::SplitTextToLines(float maxWidth)
{
    std::vector<std::wstring> lines;
    std::wstring currentLine;
    float currentWidth = 0.0f;

    for (wchar_t ch : text)
    {
        if (ch == L'\n')
        {
            lines.push_back(std::move(currentLine));
            currentLine.clear();
            currentWidth = 0.0f;
        }
        else
        {
            const float charWidth = static_cast<float>(characters[ch].xadvance) * (fontSize / fntSize);
            if (!currentLine.empty() && currentWidth + charWidth > maxWidth)
            {
                lines.push_back(std::move(currentLine));
                currentLine.clear();
                currentWidth = 0.0f;
            }
            currentLine  += ch;
            currentWidth += charWidth;
        }
    }
    if (!currentLine.empty())
        lines.push_back(std::move(currentLine));
    return lines;
}

// =============================================================================
//  カーソル位置
// =============================================================================

void Text::GetCursorPos(size_t cursorPos, _Out_ float& x, _Out_ float& y)
{
    const Vector2 origin = GetRectTransform()->UnrotatedTopLeft();
    
	// カーソルが末尾 or 空テキストの場合は最後のグリフの位置を基準にする
    if (layoutedGlyphs.empty() || cursorPos == 0)
    {
		x = origin.x;
		y = origin.y + CalcAlignedY();
    }
    else if (cursorPos <= layoutedGlyphs.size())
    {
		const auto& targetGlyph = layoutedGlyphs[cursorPos - 1];
        x = origin.x + targetGlyph.x + static_cast<float>(targetGlyph.glyph->xadvance) * (fontSize / fntSize);
        y = origin.y + targetGlyph.y;
    }
    else
    {
        const auto& lastGlyph = layoutedGlyphs.back();
        x = origin.x + lastGlyph.x + static_cast<float>(lastGlyph.glyph->xadvance) * (fontSize / fntSize);
        y = origin.y + lastGlyph.y;
    }
}

// クリック位置 → カーソルインデックス逆引き
size_t Text::GetCursorIndexFromPoint(float screenX, float screenY)
{
    if (layoutedGlyphs.empty()) return 0;

    const Vector2 origin = GetRectTransform()->UnrotatedTopLeft();
    const float localX = screenX - origin.x;
    const float localY = screenY - origin.y;
    const float lineH = ComputeLineHeight();

    // クリックされた行を特定
    // layoutedGlyphs は行順に並んでいるので、Y が最も近い行を探す
    float bestLineDist = FLT_MAX;
    float targetLineY = 0.0f;
    for (const auto& gl : layoutedGlyphs)
    {
        float dist = std::abs(gl.y + lineH * 0.5f - localY);
        if (dist < bestLineDist)
        {
            bestLineDist = dist;
            targetLineY = gl.y;
        }
    }

    // その行のグリフのうち、X が最も近い文字境界を探す
    size_t bestIndex = 0;
    float  bestDist = FLT_MAX;

    for (const auto& gl : layoutedGlyphs)
    {
        if (gl.y != targetLineY) continue;

        const float advance = static_cast<float>(gl.glyph->xadvance) * (fontSize / fntSize);
        const float glLeft = gl.x;
        const float glRight = gl.x + advance;
        const float mid = gl.x + advance * 0.5f;

        if (localX < mid)
        {
            // 文字の左側をクリック → このグリフの前
            float dist = std::abs(localX - glLeft);
            if (dist < bestDist) { bestDist = dist; bestIndex = gl.charIndex; }
        }
        else
        {
            // 文字の右側をクリック → このグリフの後
            float dist = std::abs(localX - glRight);
            if (dist < bestDist) { bestDist = dist; bestIndex = gl.charIndex + 1; }
        }
    }

    return (std::min)(bestIndex, text.size());
}

// =============================================================================
//  Inspector / シリアライズ
// =============================================================================

void Text::DrawProperty()
{
#ifdef USE_IMGUI
    std::string str = WstringToString(text);
    const size_t bufSize = (characterLimit > 0)
        ? static_cast<size_t>(characterLimit + 1)
        : 2048u;

    std::vector<char> buf(bufSize, '\0');
    strncpy_s(buf.data(), bufSize, str.c_str(), _TRUNCATE);
    if (ImGui::InputTextMultiline("Text", buf.data(), bufSize))
    {
        SetText(StringToWstring(std::string(buf.data())));
    }

    ImGui::Text("FontName: %ls", faceName.c_str());
    ImGui::Text("lineHeight: %d", lineHeight);

    IMGUI_PROPERTY_BEGIN();
    bool edited = false;

    const char* fntFilter = FILTER("Font Files", fnt);
    IMGUI_PROPERTY_STRING_WITH_DIALOG("FontFilePath", fontFilePath, MAX_PATH, fntFilter, edited);
    ImGui::PushID("globalFlag");
	ImGui::SameLine();
	static bool globalFlag = false;
	ImGui::Checkbox("ChangeAll", &globalFlag);
	ImGui::PopID();
    if (edited) 
    {
        if (globalFlag)
        {
			// 全ての Text コンポーネントを列挙して fontFilePath を変更
			std::vector<Text*> allTexts = GetScene()->FindComponents<Text>();
            for (Text* textComp : allTexts)
            {
                textComp->fontFilePath = fontFilePath;
                textComp->Setup(fontFilePath);
            }
        }
        else
        {
            Setup(fontFilePath);
        }
        edited = false;
    }

    IMGUI_PROPERTY_COLOR("Color",          color,          edited);
    IMGUI_PROPERTY_FLOAT("FontSize",       fontSize,       edited);
    IMGUI_PROPERTY_BOOL ("AutoSize",       autoSize,       edited);
    IMGUI_PROPERTY_BOOL ("BestFit",        bestFit,        edited);
    IMGUI_PROPERTY_INT  ("CharacterLimit", characterLimit, edited, 1, 0, 2048);
    if (ImGui::IsItemDeactivatedAfterEdit())
        SetCharacterLimit(characterLimit);

    // Alignment コンボ
    const char* currentLabel = kAlignmentStr[static_cast<int>(alignment)];
    IMGUI_PROPERTY("Alignment");
    if (ImGui::BeginCombo("##Alignment", currentLabel))
    {
        for (int n = 0; n < static_cast<int>(Alignment::COUNT); ++n)
        {
            const bool selected = (n == static_cast<int>(alignment));
            if (ImGui::Selectable(kAlignmentStr[n], selected))
            {
                const int newVal = n;
                const int oldVal = static_cast<int>(alignment);
                auto setter = [this](const int& v) { alignment = static_cast<Alignment>(v); };
                IMGUI_PROPERTY_COMMAND_CUSTOM("Alignment",
                    newVal, oldVal,
                    std::string(kAlignmentStr[newVal]),
                    std::string(kAlignmentStr[oldVal]),
                    setter);
            }
            if (selected) ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }

    // HorizontalOverflow コンボ
    const char* currentOverflowLabel = kHorizontalOverflowStr[static_cast<int>(horizontalOverflow)];
    IMGUI_PROPERTY("HorizontalOverflow");
    if (ImGui::BeginCombo("##HorizontalOverflow", currentOverflowLabel))
    {
        for (int n = 0; n < static_cast<int>(HorizontalOverflow::COUNT); ++n)
        {
            const bool selected = (n == static_cast<int>(horizontalOverflow));
            if (ImGui::Selectable(kHorizontalOverflowStr[n], selected))
            {
                const int newVal = n;
                const int oldVal = static_cast<int>(horizontalOverflow);
                auto setter = [this](const int& v) { horizontalOverflow = static_cast<HorizontalOverflow>(v); };
                IMGUI_PROPERTY_COMMAND_CUSTOM("HorizontalOverflow",
                    newVal, oldVal,
                    std::string(kHorizontalOverflowStr[newVal]),
                    std::string(kHorizontalOverflowStr[oldVal]),
                    setter);
            }
            if (selected) ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }

    IMGUI_PROPERTY_END();


	ImGui::Separator();

	ImGui::Text("Character Modifiers:");

    // Modifierのテスト

	static bool posOffsetEnable = false;
	static bool scaleEnable = false;
	static bool rotationEnable = false;
	static bool colorMulEnable = false;
    ImGui::Checkbox("Position Offset", &posOffsetEnable);
    ImGui::Checkbox("Scale", &scaleEnable);
    ImGui::Checkbox("Rotation", &rotationEnable);
    ImGui::Checkbox("Color Multiply", &colorMulEnable);

    // すべてのテキストをウェーブさせる
	static float waveTime = 0.0f;
    if (ImGui::Button("Apply Wave Modifier"))
    {
		waveTime = 0.0f; // Reset wave time when button is pressed
	}
	// ボタンが押されている間、テキストをウェーブさせる
    if (ImGui::IsItemActive())
    {
        waveTime += ImGui::GetIO().DeltaTime; // Increment wave time
        for (size_t i = 0; i < text.size(); ++i)
        {
            CharModifier mod;
			if (posOffsetEnable)
            {
                mod.posOffset.y = sinf(waveTime * 5.0f + i * 0.5f) * 5.0f; // Wave effect
            }
            if (scaleEnable)
            {
                float scale = 1.0f + 0.5f * sinf(waveTime * 5.0f + i * 0.5f);
                mod.scale = { scale, scale };
			}
            if (rotationEnable)
            {
                mod.rotation = sinf(waveTime * 5.0f + i * 0.5f) * 15.0f; // Rotate effect
			}
            if (colorMulEnable)
            {
                float colorFactor = 0.5f + 0.5f * sinf(waveTime * 5.0f + i * 0.5f);
                mod.colorMul = { colorFactor, colorFactor, colorFactor, 1.0f }; // Color pulsate effect
            }
            SetCharModifier(i, mod);
        }
    }
    else
    {
        // ボタンが押されていないときは、すべてのモディファイアをリセット
        ResetCharModifiers();
	}

    

#endif // USE_IMGUI
}

json Text::Serialize() const
{
    json j;
    j["fontFilePath"]       = fontFilePath;
    j["text"]               = text;
    j["fontSize"]           = fontSize;
    j["color"]              = { color.r, color.g, color.b, color.a };
    j["alignment"]          = static_cast<int>(alignment);
    j["horizontalOverflow"] = static_cast<int>(horizontalOverflow);
    j["autoSize"]           = autoSize;
    j["bestFit"]            = bestFit;
    j["characterLimit"]     = characterLimit;
    return j;
}

void Text::Deserialize(const json& j)
{
    fontFilePath       = j.value("fontFilePath",       fontFilePath);
    text               = j.value("text",               text);
    fontSize           = j.value("fontSize",           fontSize);
    if (j.contains("color") && j["color"].is_array() && j["color"].size() == 4)
    {
        color.r = j["color"][0].get<float>();
        color.g = j["color"][1].get<float>();
        color.b = j["color"][2].get<float>();
        color.a = j["color"][3].get<float>();
    }
    alignment          = static_cast<Alignment>(j.value("alignment",          static_cast<int>(alignment)));
    horizontalOverflow = static_cast<HorizontalOverflow>(j.value("horizontalOverflow", static_cast<int>(horizontalOverflow)));
    autoSize           = j.value("autoSize",           autoSize);
    bestFit            = j.value("bestFit",            bestFit);
    characterLimit     = j.value("characterLimit",     characterLimit);

    Setup(fontFilePath);
    ResizeModifiers();
}
