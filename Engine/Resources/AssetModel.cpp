#include "pch.h"
#include "AssetModel.h"

#include <Engine/Resources/Texture.h>
#include <Engine/Rendering/Material.h>
#include <Engine/Rendering/Pipeline/Graphics.h>
#include <Engine/Editor/Console.h>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <DirectXTex.h>
#include "Engine/Resources/Texture.h"

#include <cassert>
#include "ImportSettings\ModelImportSettings.h"

#include <assimp/ProgressHandler.hpp>
#include <Engine\Editor\ImportSettings\ImportSettingsWindow.h>

namespace
{
    std::u8string StringToU8String(const std::string& str)
    {
        return std::u8string(str.begin(), str.end());
	}

    std::u8string MakeUniqueName(const std::u8string& baseName, std::unordered_set<std::u8string>& existingNames)
    {
        int suffix = 1;
		std::u8string uniqueName = baseName;
        while (existingNames.find(uniqueName) != existingNames.end())
        {
			uniqueName = baseName + u8"_" + StringToU8String(std::to_string(suffix++));
        };
		existingNames.insert(uniqueName); // 新しい名前を既存の名前セットに追加
        return uniqueName;
	}

}

namespace Assimp
{
    class ImporterProgressHandler : public Assimp::ProgressHandler
    {
    public:
		ImporterProgressHandler() = default;
        virtual ~ImporterProgressHandler() = default;
        bool Update(float percentage) override
        {
			// 進捗をエディタのインポート設定ウィンドウに通知する
			CurryEngine::Resources::ImportSettingsWindow::UpdatePreviewProgress(percentage);
            
			// ユーザーがインポートのキャンセルを要求したかどうかをチェックする
            if (CurryEngine::Resources::ImportSettingsWindow::IsPreviewLoadCancelled())
            {
				return false; // falseを返すことでインポートを中止します。
            }
            return true; // trueを返すことでインポートを継続します。
        }
    };
}

// ============================================================
// 内部ヘルパー（無名名前空間）
// ============================================================
namespace
{
    // ----------------------------------------------------------
    // assimp の型 → DirectXMath 変換
    // ----------------------------------------------------------

    inline DirectX::XMFLOAT3 ToFloat3(const aiVector3D& v)
    {
        return { v.x, v.y, v.z };
    }

    inline DirectX::XMFLOAT2 ToFloat2(const aiVector3D& v)
    {
        return { v.x, v.y };
    }

    // assimp は行優先(Row-Major)、DirectXMath は列優先(Column-Major)なので転置する
    inline DirectX::XMFLOAT4X4 ToFloat4x4(const aiMatrix4x4& m)
    {
        return {
            m.a1, m.b1, m.c1, m.d1,
            m.a2, m.b2, m.c2, m.d2,
            m.a3, m.b3, m.c3, m.d3,
            m.a4, m.b4, m.c4, m.d4,
        };
    }

    inline DirectX::XMFLOAT4 ToFloat4Quat(const aiQuaternion& q)
    {
        // DirectXMath の quaternion は { x, y, z, w }
        return { q.x, q.y, q.z, q.w };
    }

    // ----------------------------------------------------------
    // D3D11 バッファ作成ユーティリティ
    // ----------------------------------------------------------

    Microsoft::WRL::ComPtr<ID3D11Buffer> CreateVertexBuffer(
        ID3D11Device* device,
        const void* data,
        UINT          sizeInBytes)
    {
        D3D11_BUFFER_DESC desc{};
        desc.ByteWidth = sizeInBytes;
        desc.Usage = D3D11_USAGE_IMMUTABLE;
        desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

        D3D11_SUBRESOURCE_DATA init{};
        init.pSysMem = data;

        Microsoft::WRL::ComPtr<ID3D11Buffer> buf;
        HRESULT hr = device->CreateBuffer(&desc, &init, buf.GetAddressOf());
        _ASSERT_EXPR(SUCCEEDED(hr), L"AssetModel: Failed to create vertex buffer");
        return buf;
    }

    Microsoft::WRL::ComPtr<ID3D11Buffer> CreateIndexBuffer(
        ID3D11Device* device,
        const void* data,
        UINT          sizeInBytes)
    {
        D3D11_BUFFER_DESC desc{};
        desc.ByteWidth = sizeInBytes;
        desc.Usage = D3D11_USAGE_IMMUTABLE;
        desc.BindFlags = D3D11_BIND_INDEX_BUFFER;

        D3D11_SUBRESOURCE_DATA init{};
        init.pSysMem = data;

        Microsoft::WRL::ComPtr<ID3D11Buffer> buf;
        HRESULT hr = device->CreateBuffer(&desc, &init, buf.GetAddressOf());
        _ASSERT_EXPR(SUCCEEDED(hr), L"AssetModel: Failed to create index buffer");
        return buf;
    }

    // ----------------------------------------------------------
	// 埋め込みテクスチャユーティリティ
    // ----------------------------------------------------------

    /**
     * @brief 埋め込みテクスチャかどうかを判定する。
     * @param path テクスチャパス。
	 * @return 埋め込みテクスチャなら true。
     */
    bool IsEmbeddedTexture(const fs::path& path)
    {
        // 埋め込みテクスチャは "*0" のような形式になる
		return !path.empty() && path.native()[0] == '*';
	}

    /**
     * @brief assimp の埋め込みテクスチャを DirectXTex::ScratchImage に読み込む。
     * @param scene    assimp のシーン。
     * @param texPath  埋め込みテクスチャのパス（"*0" のような形式）。
     * @param outImage 読み込んだ画像を格納する ScratchImage。
	 * @return 読み込みに成功すれば true。
     */
    bool GetAndLoadEmbeddedTexture(const aiScene* scene, const fs::path& texPath, DirectX::ScratchImage& outImage)
    {
        if (!IsEmbeddedTexture(texPath))
        {
            LOG_ERROR(u8"Not an embedded texture: " + texPath.u8string());
            return false;
        }
        // 埋め込みテクスチャのインデックスを取得
        int texIndex = std::stoi(texPath.wstring().substr(1)); // "*0" のような形式なので、先頭の '*' を除去して整数に変換
        if (texIndex < 0 || texIndex >= scene->mNumTextures)
        {
            LOG_ERROR("Invalid embedded texture index: " + std::to_string(texIndex));
            return false;
        }
        const aiTexture* embeddedTex = scene->mTextures[texIndex];
        if (!embeddedTex)
        {
            LOG_ERROR("Embedded texture is null at index: " + std::to_string(texIndex));
            return false;
        }
        // 埋め込みテクスチャのデータを取得
        const unsigned char* data = reinterpret_cast<const unsigned char*>(embeddedTex->pcData);
        size_t dataSize = embeddedTex->mWidth; // mWidth は埋め込みテクスチャのサイズを表す
		// DirectXTex を使って埋め込みテクスチャを読み込む
        HRESULT hr = DirectX::LoadFromWICMemory(data, dataSize, DirectX::WIC_FLAGS_NONE, nullptr, outImage);
        if (FAILED(hr))
        {
			LOG_ERROR("Failed to load embedded texture from memory. HRESULT: " + std::to_string(hr));
            return false;
        }
		return true;
    }

    /**
     * @brief assimp の埋め込みテクスチャをファイルに書き出す。
     * @param scene      assimp のシーン。
     * @param texIndex   埋め込みテクスチャのインデックス（aiScene::mTextures のインデックス）。
     * @param outputPath 書き出すファイルパス（拡張子付き）。
     * @return 書き出しに成功すれば true。
	 */
    bool ExportEmbeddedTexture(const aiScene* scene, int texIndex, const fs::path& outputPath)
    {
        if (texIndex < 0 || texIndex >= scene->mNumTextures)
        {
            Console::LogError("Invalid embedded texture index: " + std::to_string(texIndex));
            return false;
        }
        const aiTexture* embeddedTex = scene->mTextures[texIndex];
        if (!embeddedTex)
        {
            Console::LogError("Embedded texture is null at index: " + std::to_string(texIndex));
            return false;
        }
        // 埋め込みテクスチャのデータを取得
        const unsigned char* data = reinterpret_cast<const unsigned char*>(embeddedTex->pcData);
        size_t dataSize = embeddedTex->mWidth; // mWidth は埋め込みテクスチャのサイズを表す
        
		// DirectXTex を使って埋め込みテクスチャを書き出す
		DirectX::ScratchImage image;
		// 埋め込みテクスチャが圧縮されている場合は、DirectXTex の LoadFromMemory で読み込む
		HRESULT hr = DirectX::LoadFromWICMemory(data, dataSize, DirectX::WIC_FLAGS_NONE, nullptr, image);
        if (FAILED(hr))
        {
            LOG_ERROR("Failed to load embedded texture from memory. HRESULT: " + std::to_string(hr));
			return false;
        }
		// PNG 形式で書き出す
		hr = DirectX::SaveToWICFile(*image.GetImage(0, 0, 0), DirectX::WIC_FLAGS_NONE, DirectX::GetWICCodec(DirectX::WIC_CODEC_PNG), outputPath.wstring().c_str());
        if (FAILED(hr))
        {
            LOG_ERROR("Failed to save embedded texture to file: " + outputPath.string() + ". HRESULT: " + std::to_string(hr));
            return false;
		}
        return true;
	}

    // ----------------------------------------------------------
    // テクスチャパス解決
    // ----------------------------------------------------------

    /**
     * @brief マテリアルから指定タイプのテクスチャパスを取得する。
     * @param mat       assimp マテリアル。
     * @param type      テクスチャタイプ（aiTextureType_DIFFUSE 等）。
     * @param baseDir   モデルファイルのディレクトリ（相対パス解決用）。
     * @param outPath   解決済みの絶対パス（見つからなければ空文字）。
     * @return テクスチャが存在すれば true。
     */
    bool ResolveTexturePath(
        const aiMaterial* mat,
        aiTextureType     type,
        const fs::path& baseDir,
        fs::path& outPath)
    {
        if (mat->GetTextureCount(type) == 0) return false;

        aiString texPathStr;
        if (mat->GetTexture(type, 0, &texPathStr) != AI_SUCCESS) return false;
		std::u8string u8TexPath(texPathStr.C_Str(), texPathStr.C_Str() + texPathStr.length);

        // 埋め込みテクスチャは "*0" のような形式になる
        if (texPathStr.data[0] == '*')
        {
			outPath = u8TexPath; // 埋め込みテクスチャはそのまま返す
			return true;
        }

		// 相対パスを絶対パスに変換して正規化する
		fs::path texPath(u8TexPath);
		texPath = texPath.lexically_normal(); // 正規化して冗長なパス要素を削除
		fs::path resolved = fs::absolute(baseDir) / texPath;
		// ファイルが存在するかチェック
        if (!fs::exists(resolved))
        {
            Console::LogWarning("Texture file not found: " + resolved.string());
            outPath.clear();
            return false;
		}
        outPath = resolved;
        return true;
    }

} // namespace

// ============================================================
// Resource インターフェース
// ============================================================

bool AssetModel::LoadFromFile(const std::string& path)
{
    _path = path;

    Assimp::Importer importer;

    // --- インポートフラグ ---
    // triangulate      : ポリゴンを三角形化
    // genNormals       : 法線がなければ生成
    // calcTangentSpace : タンジェント・バイタンジェントを計算
    // joinIdentical    : 同一頂点を結合してインデックスを最適化
    // flipUVs          : UV の V 軸を反転（DirectX は上が 0）
    // flipWindingOrder : 巻き順を反転（assimp はデフォルト CCW、DirectX は CW）
    constexpr unsigned int importFlags =
        aiProcess_Triangulate |
        aiProcess_GenSmoothNormals |
        aiProcess_CalcTangentSpace |
        aiProcess_JoinIdenticalVertices |
        aiProcess_FlipUVs |
        aiProcess_FlipWindingOrder |
        aiProcess_LimitBoneWeights;     // ボーン影響数を 4 に制限

    const aiScene* scene = importer.ReadFile(path, importFlags);
    if (!scene || (scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) || !scene->mRootNode)
    {
        Console::LogError("AssetModel::LoadFromFile failed: " + std::string(importer.GetErrorString()));
        return false;
    }

    const std::u8string baseDir = fs::path(path).parent_path().u8string();
    return ImportFromScene(scene, baseDir);
}

bool AssetModel::Reload()
{
    // 既存データをクリアして再ロード
    rootNodes.clear();
    nodes.clear();
    meshes.clear();
    materials.clear();
    textures.clear();
    skins.clear();
    animations.clear();
    batchedMeshes.clear();

    return LoadFromFile(_path);
}

bool AssetModel::LoadFromMeta(const CurryEngine::Resources::AssetMeta& meta)
{
	_path = meta.path.string();

    Assimp::Importer importer;
	CurryEngine::Resources::ModelImportSettings settings = meta.GetImportSettings<CurryEngine::Resources::ModelImportSettings>();
	importer.SetPropertyFloat(AI_CONFIG_GLOBAL_SCALE_FACTOR_KEY, settings.scaleFactor); // モデル全体のスケーリングを設定

    Assimp::ProgressHandler* progressHandler = new Assimp::ImporterProgressHandler();
	importer.SetProgressHandler(progressHandler);

	std::u8string u8Path = meta.path.u8string(); // UTF-8 文字列に変換
    const aiScene* scene = importer.ReadFile(reinterpret_cast<const char*>(u8Path.c_str()), settings.importFlags);
    if (!scene || (scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) || !scene->mRootNode)
    {
		std::string errorMsg = importer.GetErrorString();
		std::u8string u8ErrorMsg(errorMsg.begin(), errorMsg.end());
        LOG_ERROR(u8"AssetModel::LoadFromFile failed: " + u8ErrorMsg);
        return false;
    }

    const std::u8string baseDir = fs::path(_path).parent_path().u8string();
    return ImportFromScene(scene, baseDir);
}

// ============================================================
// ImportFromScene — assimp → AssetModel 変換のメインルーティン
// ============================================================

bool AssetModel::ImportFromScene(const aiScene* scene, const std::u8string& baseDir)
{
    // 処理順：テクスチャ → マテリアル → メッシュ → ノードツリー → スキン → アニメーション
    ImportTextures(scene, baseDir);
    ImportMaterials(scene);
    ImportMeshes(scene);
    ImportNodes(scene);
    ImportSkins(scene);
    ImportAnimations(scene);

    CumulateTransforms();
    return true;
}

// ============================================================
// テクスチャ
// ============================================================

void AssetModel::ImportTextures(const aiScene* scene, const std::u8string& baseDir)
{
    // assimp はテクスチャをマテリアルごとに保持しているため、
    // ここでは全マテリアルを走査して使われているパスを収集し、
    // 重複なくロードする。
    // texturePaths[絶対パス] = textures インデックス
    std::unordered_map<fs::path, int> pathToIndex;

    // 埋め込みテクスチャの書き出しパスを保持する
	std::vector<fs::path> embeddedTexturePaths;

    auto getOrLoad = [&](const fs::path& absPath) -> int
        {
            auto it = pathToIndex.find(absPath);
            if (it != pathToIndex.end()) return it->second;

            // TODO: ResourceManager 経由でキャッシュを使うように変更予定
            auto tex = std::make_shared<AssetTexture>();
            if (!tex->Load(Graphics::GetDevice(), absPath))
            {
                LOG_WARNING("AssetModel: texture not found: " + absPath.string());
                return -1;
            }
            const int index = static_cast<int>(textures.size());
            textures.push_back(std::move(tex));
            pathToIndex[absPath] = index;
            return index;
        };

    // テクスチャタイプごとに走査（後の ImportMaterials で index を引くための準備）
    // 実際の紐付けは ImportMaterials() 内で行う。ここでは textures[] を埋めるだけ。
    for (unsigned int i = 0; i < scene->mNumMaterials; ++i)
    {
        const aiMaterial* mat = scene->mMaterials[i];
        static const aiTextureType types[] = {
            aiTextureType_DIFFUSE,
            aiTextureType_NORMALS,
            aiTextureType_METALNESS,
            aiTextureType_DIFFUSE_ROUGHNESS,
            aiTextureType_EMISSIVE,
            aiTextureType_LIGHTMAP, // AO
        };
        for (aiTextureType type : types)
        {
            fs::path absPath;
            if (ResolveTexturePath(mat, type, baseDir, absPath))
            {
                if (IsEmbeddedTexture(absPath))
                {
					embeddedTexturePaths.push_back(absPath); // 埋め込みテクスチャは後で書き出す
                }
                else
                {
                    getOrLoad(absPath);
                }
            }
        }
    }
	// 埋め込みテクスチャのロード
    for (const fs::path& embeddedPath : embeddedTexturePaths)
    {
#if 0
        DirectX::ScratchImage embeddedScratchImage;
        if (GetAndLoadEmbeddedTexture(scene, embeddedPath, embeddedScratchImage))
        {
            // 埋め込みテクスチャから AssetTexture を作成
            auto tex = std::make_shared<AssetTexture>();

            // SRV を作成して AssetTexture に設定
            Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv;
            HRESULT hr = LoadTextureFromMemory(Graphics::GetDevice(), embeddedScratchImage.GetPixels(), embeddedScratchImage.GetPixelsSize(), srv.GetAddressOf());
            _ASSERT_EXPR(SUCCEEDED(hr), HrTrace(hr));

            Microsoft::WRL::ComPtr<ID3D11Texture2D> resource;
            D3D11_TEXTURE2D_DESC desc{};
            srv->GetResource(reinterpret_cast<ID3D11Resource**>(resource.GetAddressOf()));
            resource->GetDesc(&desc);

            // AssetTexture に設定
            tex->SetSRV(srv.Get(), desc);
        }
#else
		int texIndex = std::stoi(embeddedPath.wstring().substr(1));
		std::string texName = scene->mTextures[texIndex]->mFilename.C_Str();
        if (texName.empty())
        {
            texName = "embedded_texture_" + std::to_string(texIndex);
		}
        fs::path outputPath = baseDir / fs::path(texName);
		outputPath.replace_extension(".png"); // PNG 形式で書き出す
        if (ExportEmbeddedTexture(scene, texIndex, outputPath))
        {
            // 書き出した埋め込みテクスチャをロード
			getOrLoad(outputPath);
        }
#endif // 0
	}

    // pathToIndex を後の ImportMaterials で使えるようにメンバに保存
    m_texturePathToIndex = std::move(pathToIndex);
}

// ============================================================
// マテリアル
// ============================================================

void AssetModel::ImportMaterials(const aiScene* scene)
{
    auto device = Graphics::GetDevice();

    for (unsigned int i = 0; i < scene->mNumMaterials; ++i)
    {
        const aiMaterial* aiMat = scene->mMaterials[i];
        auto mat = std::make_shared<Material>();

        // --- ベースカラー ---
        aiColor4D baseColor(1, 1, 1, 1);
        if (aiMat->Get(AI_MATKEY_COLOR_DIFFUSE, baseColor) == AI_SUCCESS)
        {
            DirectX::XMFLOAT4 color = { baseColor.r, baseColor.g, baseColor.b, baseColor.a };
            mat->SetValue("baseColorFactor", color);
        }

        // --- Metallic / Roughness ---
        float metallic = 0.0f, roughness = 0.5f;
        aiMat->Get(AI_MATKEY_METALLIC_FACTOR, metallic);
        aiMat->Get(AI_MATKEY_ROUGHNESS_FACTOR, roughness);
        mat->SetValue("metallicFactor", metallic);
        mat->SetValue("roughnessFactor", roughness);

        // --- Emissive ---
        aiColor3D emissive(0, 0, 0);
        if (aiMat->Get(AI_MATKEY_COLOR_EMISSIVE, emissive) == AI_SUCCESS)
        {
            DirectX::XMFLOAT3 e = { emissive.r, emissive.g, emissive.b };
            mat->SetValue("emissiveFactor", e);
        }

        // --- テクスチャのバインド ---
		fs::path baseDir = fs::path(_path).parent_path();
        auto bindTex = [&](aiTextureType type, const std::string& slotName)
            {
				mat->SetValue("has_" + slotName, false); // デフォルトは false
                fs::path absPath;
                if (!ResolveTexturePath(aiMat, type, baseDir, absPath)) return;
                // 埋め込みテクスチャの場合はインデックスから探して代入
                int texIndex = -1;
                if (IsEmbeddedTexture(absPath))
                {
                    texIndex = std::stoi(absPath.wstring().substr(1));
                }
                else
                {
                    auto it = m_texturePathToIndex.find(absPath);
                    if (it == m_texturePathToIndex.end()) return;
                    texIndex = it->second;
                }
				_ASSERT_EXPR(texIndex >= 0 && texIndex < static_cast<int>(textures.size()), L"Invalid texture index");

                mat->SetTexture(slotName, textures.at(texIndex));
				mat->SetValue("has_" + slotName, true); // シェーダー側でテクスチャ有無を判定するためのフラグ)
            };

        // スロット名はシェーダー側の変数名に合わせる
        // TODO: シェーダー変数名が確定したら定数化する
        bindTex(aiTextureType_DIFFUSE, "baseColorTexture");
        bindTex(aiTextureType_NORMALS, "normalTexture");
        bindTex(aiTextureType_METALNESS, "metallicRoughnessTexture");
        bindTex(aiTextureType_EMISSIVE, "emissiveTexture");
        bindTex(aiTextureType_LIGHTMAP, "occlusionTexture");

        // --- レンダーステート ---
        // 両面描画
        int twoSided = 0;
        if (aiMat->Get(AI_MATKEY_TWOSIDED, twoSided) == AI_SUCCESS && twoSided)
        {
            mat->SetRasterizerState(RasterizerState::SolidCullNone);
        }

		// ブレンドモード
		int blendMode = 0;
        if (aiMat->Get(AI_MATKEY_BLEND_FUNC, blendMode) == AI_SUCCESS)
        {
            if (blendMode == aiBlendMode_Default)
            {
                mat->SetBlendState(BlendState::Transparency);
            }
            else if (blendMode == aiBlendMode_Additive)
            {
                mat->SetBlendState(BlendState::Additive);
			}
        }

        materials.push_back(std::move(mat));
    }
}

// ============================================================
// メッシュ
// ============================================================

void AssetModel::ImportMeshes(const aiScene* scene)
{
    // assimp の aiMesh を MeshData にフラット変換する。
    // 旧設計の Mesh > Primitive 二層構造はここで解消し、
    // MeshData 1 件 = 1 ドローコールとする。
    // Node → aiMesh のマッピングは ImportNodes() 内で meshIndexRemap を使って解決する。

    meshes.reserve(scene->mNumMeshes);

    for (unsigned int mi = 0; mi < scene->mNumMeshes; ++mi)
    {
        const aiMesh* aiM = scene->mMeshes[mi];
        MeshData mesh;
        mesh.name = StringToU8String(aiM->mName.C_Str());
        mesh.materialIndex = static_cast<int>(aiM->mMaterialIndex);
        mesh.isSkinned = aiM->HasBones();
        mesh.hasTangent = aiM->HasTangentsAndBitangents();

        // --- インデックス ---
        mesh.indices.reserve(static_cast<size_t>(aiM->mNumFaces) * 3);
        for (unsigned int fi = 0; fi < aiM->mNumFaces; ++fi)
        {
            const aiFace& face = aiM->mFaces[fi];
            // aiProcess_Triangulate により必ず 3 頂点
            mesh.indices.push_back(face.mIndices[0]);
            mesh.indices.push_back(face.mIndices[1]);
            mesh.indices.push_back(face.mIndices[2]);
        }
        mesh.indexCount = static_cast<UINT>(mesh.indices.size());

        // --- 頂点 ---
        if (mesh.isSkinned)
        {
            mesh.skinnedVertices.resize(aiM->mNumVertices);
            for (unsigned int vi = 0; vi < aiM->mNumVertices; ++vi)
            {
                SkinnedVertex& v = mesh.skinnedVertices[vi];
                v.position = ToFloat3(aiM->mVertices[vi]);
                if (aiM->HasNormals())
                    v.normal = ToFloat3(aiM->mNormals[vi]);
                if (aiM->HasTangentsAndBitangents())
                {
                    v.tangent = {
                        aiM->mTangents[vi].x,
                        aiM->mTangents[vi].y,
                        aiM->mTangents[vi].z,
                        1.0f // handedness（必要なら mBitangents との外積で判定）
                    };
                }
                if (aiM->HasTextureCoords(0))
                    v.texcoord = ToFloat2(aiM->mTextureCoords[0][vi]);
            }
            mesh.vertexStride = sizeof(SkinnedVertex);

            // ボーンウェイト（joints / weights）
            // aiMesh::mBones を頂点インデックスで逆引きして詰める
            for (unsigned int bi = 0; bi < aiM->mNumBones; ++bi)
            {
                const aiBone* bone = aiM->mBones[bi];
                for (unsigned int wi = 0; wi < bone->mNumWeights; ++wi)
                {
                    const aiVertexWeight& vw = bone->mWeights[wi];
                    SkinnedVertex& v = mesh.skinnedVertices[vw.mVertexId];

                    // joints / weights の空きスロット（weight == 0）に詰める
                    for (int slot = 0; slot < 4; ++slot)
                    {
                        if (reinterpret_cast<float*>(&v.weights)[slot] == 0.0f)
                        {
                            // joints は XMUINT4 なのでキャストで格納
                            reinterpret_cast<uint32_t*>(&v.joints)[slot] = bi;
                            reinterpret_cast<float*>(&v.weights)[slot] = vw.mWeight;
                            break;
                        }
                    }
                    // aiProcess_LimitBoneWeights により最大 4 なので溢れない前提
                }
            }

            // ボーン名 → グローバルノードインデックスのマッピングを保存
            // ImportSkins() で参照するため mesh に持たせる
            mesh.boneNames.reserve(aiM->mNumBones);
            mesh.boneOffsetMatrices.reserve(aiM->mNumBones);
            for (unsigned int bi = 0; bi < aiM->mNumBones; ++bi)
            {
                mesh.boneNames.push_back(StringToU8String(aiM->mBones[bi]->mName.C_Str()));
                mesh.boneOffsetMatrices.push_back(ToFloat4x4(aiM->mBones[bi]->mOffsetMatrix));
            }
        }
        else
        {
            mesh.staticVertices.resize(aiM->mNumVertices);
            for (unsigned int vi = 0; vi < aiM->mNumVertices; ++vi)
            {
                StaticVertex& v = mesh.staticVertices[vi];
                v.position = ToFloat3(aiM->mVertices[vi]);
                if (aiM->HasNormals())
                    v.normal = ToFloat3(aiM->mNormals[vi]);
                if (aiM->HasTangentsAndBitangents())
                {
                    v.tangent = {
                        aiM->mTangents[vi].x,
                        aiM->mTangents[vi].y,
                        aiM->mTangents[vi].z,
                        1.0f
                    };
                }
                if (aiM->HasTextureCoords(0))
                    v.texcoord = ToFloat2(aiM->mTextureCoords[0][vi]);
            }
            mesh.vertexStride = sizeof(StaticVertex);
        }

        meshes.push_back(std::move(mesh));
    }
}

// ============================================================
// ノードツリー
// ============================================================

void AssetModel::ImportNodes(const aiScene* scene)
{
    // aiNode を再帰的に走査して AssetModel::Node のフラット配列に変換する。
    // aiNode::mMeshes は aiScene::mMeshes へのインデックスを複数持てるため、
    // Node::meshIndices（vector<int>）に全件マップする。

    // ノード名 → インデックス（スキン解決で使う）
    m_nodeNameToIndex.clear();

	std::unordered_set<std::u8string> nodeNames; // 重複ノード名チェック用

    std::function<int(const aiNode*, int)> traverse = [&](const aiNode* aiN, int parentIndex) -> int
        {
            Node node;
            node.name = MakeUniqueName(StringToU8String(aiN->mName.C_Str()), nodeNames);
            node.parent = parentIndex;

            // TRS 分解（aiMatrix4x4::Decompose を使う）
            aiVector3D    pos, scl;
            aiQuaternion  rot;
            aiN->mTransformation.Decompose(scl, rot, pos);
            node.translation = ToFloat3(pos);
            node.scale = ToFloat3(scl);
            node.rotation = ToFloat4Quat(rot);

            // メッシュ参照（1ノードに複数メッシュがぶら下がるケースに対応）
            if (aiN->mNumMeshes > 0)
            {
                node.meshIndices.reserve(aiN->mNumMeshes);
                for (unsigned int mi = 0; mi < aiN->mNumMeshes; ++mi)
                {
                    node.meshIndices.push_back(static_cast<int>(aiN->mMeshes[mi]));
                }
            }

            const int myIndex = static_cast<int>(nodes.size());
            nodes.push_back(node); // push してからインデックスを確定
            m_nodeNameToIndex[node.name] = myIndex;

            // 子を再帰処理し、children を記録
            for (unsigned int ci = 0; ci < aiN->mNumChildren; ++ci)
            {
                const int childIndex = traverse(aiN->mChildren[ci], myIndex);
                nodes[myIndex].children.push_back(childIndex);
            }

            return myIndex;
        };

    const int rootIndex = traverse(scene->mRootNode, -1);
    rootNodes.push_back(rootIndex);
}

// ============================================================
// スキン
// ============================================================

void AssetModel::ImportSkins(const aiScene* /*scene*/)
{
    // assimp はスキンを GLTF のように独立した "skin" オブジェクトとして
    // 持たないため、ImportMeshes() で収集した boneNames を使って
    // 各スキンメッシュごとに Skin を構築する。

    for (int mi = 0; mi < static_cast<int>(meshes.size()); ++mi)
    {
        MeshData& mesh = meshes[mi];
        if (!mesh.isSkinned || mesh.boneNames.empty()) continue;

        Skin skin;
        skin.name = mesh.name + u8"_Skin";
        skin.joints.reserve(mesh.boneNames.size());
        skin.inverseBindMatrices.reserve(mesh.boneNames.size());

        for (size_t bi = 0; bi < mesh.boneNames.size(); ++bi)
        {
            const std::u8string& boneName = mesh.boneNames[bi];
            auto it = m_nodeNameToIndex.find(boneName);
            if (it == m_nodeNameToIndex.end())
            {
                // ボーンに対応するノードが見つからない場合はダミーインデックスを積む
                LOG_WARNING(u8"AssetModel: bone '" + boneName + u8"' has no matching node.");
                skin.joints.push_back(-1);
            }
            else
            {
                skin.joints.push_back(it->second);
            }
            skin.inverseBindMatrices.push_back(mesh.boneOffsetMatrices[bi]);
        }

        const int skinIndex = static_cast<int>(skins.size());
        skins.push_back(std::move(skin));

        // メッシュを参照するノードに skinIndex を設定する
        // （1ノードが複数メッシュを持つ場合があるため、meshIndices の
        //   何番目のスロットかを skinIndices の同じスロットに対応させる）
        for (Node& node : nodes)
        {
            for (size_t slot = 0; slot < node.meshIndices.size(); ++slot)
            {
                if (node.meshIndices[slot] != mi) continue;

                if (node.skinIndices.size() != node.meshIndices.size())
                {
                    node.skinIndices.assign(node.meshIndices.size(), -1);
                }
                node.skinIndices[slot] = skinIndex;
                // break しない：同じメッシュを複数ノードが参照するケースにも対応
            }
        }
    }
}

// ============================================================
// アニメーション
// ============================================================

void AssetModel::ImportAnimations(const aiScene* scene)
{
    animations.reserve(scene->mNumAnimations);

    for (unsigned int ai = 0; ai < scene->mNumAnimations; ++ai)
    {
        const aiAnimation* aiAnim = scene->mAnimations[ai];
        Animation anim;
        anim.name = StringToU8String(aiAnim->mName.C_Str());

        // duration はティック単位 → 秒に変換
        const double ticksPerSecond = aiAnim->mTicksPerSecond > 0
            ? aiAnim->mTicksPerSecond : 25.0;
        anim.duration = static_cast<float>(aiAnim->mDuration / ticksPerSecond);

        anim.channels.reserve(aiAnim->mNumChannels);
        anim.samplers.reserve(static_cast<size_t>(aiAnim->mNumChannels * 3)); // TRS で最大 3 サンプラー/チャンネル

        for (unsigned int ci = 0; ci < aiAnim->mNumChannels; ++ci)
        {
            const aiNodeAnim* aiChan = aiAnim->mChannels[ci];
            const std::u8string nodeName = StringToU8String(aiChan->mNodeName.C_Str());

            auto nodeIt = m_nodeNameToIndex.find(nodeName);
            if (nodeIt == m_nodeNameToIndex.end())
            {
                LOG_WARNING(u8"AssetModel: animation channel targets unknown node: " + nodeName);
                continue;
            }
            const int nodeIndex = nodeIt->second;

            // --- Translation ---
            if (aiChan->mNumPositionKeys > 0)
            {
                Animation::Sampler sampler;
                sampler.interpolation = "LINEAR"; // assimp は補間種別を per-key で持つが簡略化
                sampler.timelines.reserve(aiChan->mNumPositionKeys);
                sampler.translations.reserve(aiChan->mNumPositionKeys);
                for (unsigned int k = 0; k < aiChan->mNumPositionKeys; ++k)
                {
                    sampler.timelines.push_back(
                        static_cast<float>(aiChan->mPositionKeys[k].mTime / ticksPerSecond));
                    sampler.translations.push_back(ToFloat3(aiChan->mPositionKeys[k].mValue));
                }
                const int samplerIndex = static_cast<int>(anim.samplers.size());
                anim.samplers.push_back(std::move(sampler));

                Animation::Channel channel;
                channel.nodeIndex = nodeIndex;
                channel.targetPath = "translation";
                channel.samplerIndex = samplerIndex;
                anim.channels.push_back(channel);
            }

            // --- Rotation ---
            if (aiChan->mNumRotationKeys > 0)
            {
                Animation::Sampler sampler;
                sampler.interpolation = "LINEAR";
                sampler.timelines.reserve(aiChan->mNumRotationKeys);
                sampler.rotations.reserve(aiChan->mNumRotationKeys);
                for (unsigned int k = 0; k < aiChan->mNumRotationKeys; ++k)
                {
                    sampler.timelines.push_back(
                        static_cast<float>(aiChan->mRotationKeys[k].mTime / ticksPerSecond));
                    sampler.rotations.push_back(ToFloat4Quat(aiChan->mRotationKeys[k].mValue));
                }
                const int samplerIndex = static_cast<int>(anim.samplers.size());
                anim.samplers.push_back(std::move(sampler));

                Animation::Channel channel;
                channel.nodeIndex = nodeIndex;
                channel.targetPath = "rotation";
                channel.samplerIndex = samplerIndex;
                anim.channels.push_back(channel);
            }

            // --- Scale ---
            if (aiChan->mNumScalingKeys > 0)
            {
                Animation::Sampler sampler;
                sampler.interpolation = "LINEAR";
                sampler.timelines.reserve(aiChan->mNumScalingKeys);
                sampler.scales.reserve(aiChan->mNumScalingKeys);
                for (unsigned int k = 0; k < aiChan->mNumScalingKeys; ++k)
                {
                    sampler.timelines.push_back(
                        static_cast<float>(aiChan->mScalingKeys[k].mTime / ticksPerSecond));
                    sampler.scales.push_back(ToFloat3(aiChan->mScalingKeys[k].mValue));
                }
                const int samplerIndex = static_cast<int>(anim.samplers.size());
                anim.samplers.push_back(std::move(sampler));

                Animation::Channel channel;
                channel.nodeIndex = nodeIndex;
                channel.targetPath = "scale";
                channel.samplerIndex = samplerIndex;
                anim.channels.push_back(channel);
            }
        }

        animations.push_back(std::move(anim));
    }
}

// ============================================================
// トランスフォーム累積
// ============================================================

void AssetModel::CumulateTransforms()
{
    const DirectX::XMFLOAT4X4 identity = { 1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1 };
    for (int rootIndex : rootNodes)
    {
        CumulateTransforms(rootIndex, identity);
    }
}

void AssetModel::CumulateTransforms(int nodeIndex, const DirectX::XMFLOAT4X4& parentTransform)
{
    Node& node = nodes[nodeIndex];

    // TRS → ローカル行列
    DirectX::XMMATRIX localMat =
        DirectX::XMMatrixScalingFromVector(DirectX::XMLoadFloat3(&node.scale)) *
        DirectX::XMMatrixRotationQuaternion(DirectX::XMLoadFloat4(&node.rotation)) *
        DirectX::XMMatrixTranslationFromVector(DirectX::XMLoadFloat3(&node.translation));

    // グローバル = ローカル × 親グローバル
    DirectX::XMStoreFloat4x4(
        &node.globalTransform,
        localMat * DirectX::XMLoadFloat4x4(&parentTransform));

    for (int childIndex : node.children)
    {
        CumulateTransforms(childIndex, node.globalTransform);
    }
}

// ============================================================
// GPU アップロード
// ============================================================

void AssetModel::UploadToGPU(ID3D11Device* device)
{
    for (MeshData& mesh : meshes)
    {
        UploadMesh(device, mesh);
    }

    // TODO: staticBatching が true のとき batchedMeshes を構築してアップロードする
    //       （ModelBatcher クラスに委譲予定）
}

void AssetModel::UploadMesh(ID3D11Device* device, MeshData& mesh)
{
    if (mesh.IsUploaded()) return;

    // 頂点バッファ
    if (mesh.isSkinned && !mesh.skinnedVertices.empty())
    {
        const UINT vbSize = static_cast<UINT>(
            mesh.skinnedVertices.size() * sizeof(SkinnedVertex));
        mesh.vertexBuffer = CreateVertexBuffer(device, mesh.skinnedVertices.data(), vbSize);
    }
    else if (!mesh.staticVertices.empty())
    {
        const UINT vbSize = static_cast<UINT>(
            mesh.staticVertices.size() * sizeof(StaticVertex));
        mesh.vertexBuffer = CreateVertexBuffer(device, mesh.staticVertices.data(), vbSize);
    }

    // インデックスバッファ
    if (!mesh.indices.empty())
    {
        const UINT ibSize = static_cast<UINT>(mesh.indices.size() * sizeof(uint32_t));
        mesh.indexBuffer = CreateIndexBuffer(device, mesh.indices.data(), ibSize);
    }
}
