#include "pch.h"
#include "Engine/Core/Misc.h"
#include "SkinnedMesh.h"
#include <sstream>
#include <functional>
using namespace DirectX;

#include "Engine/Resources/Texture.h"
#include "Engine/Resources/Shader.h"
#include <filesystem>
#include <fstream>

#include "AssetDatabase.h"
#include "ImportSettings/FbxImportSettings.h"

#include "Engine/Resources/Importers/ImporterRegistry.h"
#include "Engine/Resources/Importers/TextureImporter.h"

#include "Engine/Rendering/Pipeline/Graphics.h"

inline XMFLOAT4X4 ToXMFLOAT4X4(const FbxAMatrix& fbxamatrix)
{
	XMFLOAT4X4 xmfloat4x4;
	for (int row = 0; row < 4; ++row)
	{
		for (int column = 0; column < 4; ++column)
		{
			xmfloat4x4.m[row][column] = static_cast<float>(fbxamatrix[row][column]);
		}
	}
	return xmfloat4x4;
}
inline XMFLOAT3 ToXMFLOAT3(const FbxDouble3& fbxdouble3)
{
	XMFLOAT3 xmfloat3;
	xmfloat3.x = static_cast<float>(fbxdouble3[0]);
	xmfloat3.y = static_cast<float>(fbxdouble3[1]);
	xmfloat3.z = static_cast<float>(fbxdouble3[2]);
	return xmfloat3;
}
inline XMFLOAT4 ToXMFLOAT4(const FbxDouble4& fbxdouble4)
{
	XMFLOAT4 xmfloat4;
	xmfloat4.x = static_cast<float>(fbxdouble4[0]);
	xmfloat4.y = static_cast<float>(fbxdouble4[1]);
	xmfloat4.z = static_cast<float>(fbxdouble4[2]);
	xmfloat4.w = static_cast<float>(fbxdouble4[3]);
	return xmfloat4;
}

namespace CurryEngine::Resources
{
	void FetchBoneInfluences(const FbxMesh * fbxMesh,
		std::vector<BoneInfluencesPerControlPoint>&boneInfluences)
	{
		const int controlPointsCount = { fbxMesh->GetControlPointsCount() };
		boneInfluences.resize(controlPointsCount);

		const int skinCount{ fbxMesh->GetDeformerCount(FbxDeformer::eSkin) };
		for (int skinIndex = 0; skinIndex < skinCount; ++skinIndex)
		{
			const FbxSkin* fbxSkin{ static_cast<FbxSkin*>(fbxMesh->GetDeformer(skinIndex, FbxDeformer::eSkin)) };

			const int clusterCount{ fbxSkin->GetClusterCount() };
			for (int clusterIndex = 0; clusterIndex < clusterCount; ++clusterIndex)
			{
				const FbxCluster* fbxCluster{ fbxSkin->GetCluster(clusterIndex) };

				const int controlPointIndicesCount{ fbxCluster->GetControlPointIndicesCount() };
				for (int controlPointIndicesIndex = 0; controlPointIndicesIndex < controlPointIndicesCount; ++controlPointIndicesIndex)
				{
					int controlPointIndex{ fbxCluster->GetControlPointIndices()[controlPointIndicesIndex] };
					double controlPointWeight{ fbxCluster->GetControlPointWeights()[controlPointIndicesIndex] };
					BoneInfluence& boneInfluence{ boneInfluences.at(controlPointIndex).emplace_back() };
					boneInfluence.boneIndex = static_cast<uint32_t>(clusterIndex);
					boneInfluence.boneWeight = static_cast<float>(controlPointWeight);
				}
			}
		}
	}

	bool SkinnedMesh::LoadFromFile(const std::string& path)
	{
		auto device = Graphics::GetDevice();

		AssetId assetId = AssetDatabase::GetOrImport(path)->id;
		if (assetId.IsValid())
		{
			return LoadFromAssetId(device, assetId);
		}
		else
		{
			LOG_ERROR(std::format("Failed to load skinned mesh from file: {}", path));
			return false;
		}
	}

	bool SkinnedMesh::LoadFromFBX(ID3D11Device* device, const char* fbx_filename, const char* artifactStem, FbxImportSettings* settings)
	{
		_path = fbx_filename;

		std::filesystem::path artifactPath(EnginePaths::ArtifactDir / artifactStem);
		artifactPath.replace_extension("");
		if (std::filesystem::exists(artifactPath))
		{
			std::ifstream ifs(artifactPath, std::ios::binary);
			cereal::BinaryInputArchive deserialization(ifs);
			deserialization(scene_view, meshes, materials, animationClips);
		}
		else
		{
			FbxManager* fbxManager{ FbxManager::Create() };
			FbxScene* fbxScene{ FbxScene::Create(fbxManager, "") };

			FbxImporter* fbxImporter{ FbxImporter::Create(fbxManager, "") };
			bool importStatus{ false };
			importStatus = fbxImporter->Initialize(fbx_filename);
			_ASSERT_EXPR_A(importStatus, fbxImporter->GetStatus().GetErrorString());

			importStatus = fbxImporter->Import(fbxScene);
			_ASSERT_EXPR_A(importStatus, fbxImporter->GetStatus().GetErrorString());

			FbxGeometryConverter fbxConverter(fbxManager);
			fbxConverter.Triangulate(fbxScene, true/*replace*/, false/*legacy*/);
			fbxConverter.RemoveBadPolygonsFromMeshes(fbxScene);

			float samplingRate{ 60.0f };

			// インポート設定の適用
			if (settings)
			{
				// 座標系のスケーリングを変換する
				if (settings->scale != FbxImportScaleMode::None)
				{
					FbxSystemUnit systemUnit;
					switch (settings->scale)
					{
					case FbxImportScaleMode::Millimeters: systemUnit = FbxSystemUnit::mm; break;
					case FbxImportScaleMode::Centimeters: systemUnit = FbxSystemUnit::cm; break;
					case FbxImportScaleMode::Meters: systemUnit = FbxSystemUnit::m; break;
					case FbxImportScaleMode::Kilometers: systemUnit = FbxSystemUnit::km; break;
					case FbxImportScaleMode::Inches: systemUnit = FbxSystemUnit::Inch; break;
					case FbxImportScaleMode::Feet: systemUnit = FbxSystemUnit::Foot; break;
					case FbxImportScaleMode::Yards: systemUnit = FbxSystemUnit::Yard; break;
					case FbxImportScaleMode::Miles: systemUnit = FbxSystemUnit::Mile; break;
					default: systemUnit = FbxSystemUnit::m; break; // デフォルトはメートル
					}
					systemUnit.ConvertScene(fbxScene);
				}

				// 座標系を変換する
				FbxAxisSystem axisSystem = FbxAxisSystem::DirectX;
				axisSystem.DeepConvertScene(fbxScene);

				// アニメーションのサンプリングレートを設定する
				samplingRate = settings->animationSamplingRate;
			}

			std::function<void(FbxNode*)> traverse{ [&](FbxNode* fbxNode) {
				Scene::Node& node = scene_view.nodes.emplace_back();
				node.attribute = fbxNode->GetNodeAttribute() ?
					fbxNode->GetNodeAttribute()->GetAttributeType() : FbxNodeAttribute::EType::eUnknown;
				node.name = fbxNode->GetName();
				node.id = fbxNode->GetUniqueID();
				node.parentIndex = scene_view.IndexOf(fbxNode->GetParent() ?
					fbxNode->GetParent()->GetUniqueID() : 0);
				for (int childIndex = 0; childIndex < fbxNode->GetChildCount(); ++childIndex)
				{
					traverse(fbxNode->GetChild(childIndex));
				}
			} };
			traverse(fbxScene->GetRootNode());

			FetchMeshes(fbxScene, meshes);
			FetchMaterials(fbxScene, materials);
			FetchAnimations(fbxScene, animationClips, samplingRate);

			fbxManager->Destroy();

			// シリアライズしてアーティファクトとして保存
			if (std::filesystem::exists(artifactPath.parent_path()) == false)
			{
				std::filesystem::create_directories(artifactPath.parent_path());
			}
			std::ofstream ofs(artifactPath, std::ios::binary);
			cereal::BinaryOutputArchive serialization(ofs);
			serialization(scene_view, meshes, materials, animationClips);
		}
		CreateComObjects(device, fbx_filename);

		return true;
	}

	bool SkinnedMesh::LoadFromAssetId(ID3D11Device* device, const AssetId& assetId)
	{
		const AssetMeta* meta = AssetDatabase::Find(assetId);
		if (!meta)
		{
			LOG_ERROR(std::format("Failed to find asset meta for assetId: {}", assetId.ToString()));
			return false;
		}

		FbxImportSettings importSettings = meta->GetImportSettings<FbxImportSettings>();

		return LoadFromFBX(device, meta->path.string().c_str(), assetId.id.c_str(), &importSettings);
	}


	bool SkinnedMesh::AppendAnimations(const char* animationFilename, float samplingRate)
	{
		std::filesystem::path cerealFilename(animationFilename);
		cerealFilename.replace_extension("cereal");
		if (std::filesystem::exists(cerealFilename.c_str()))
		{
			std::ifstream ifs(cerealFilename.c_str(), std::ios::binary);
			cereal::BinaryInputArchive deserialization(ifs);
			deserialization(animationClips);
		}
		else
		{
			FbxManager* fbxManager{ FbxManager::Create() };
			FbxScene* fbxScene{ FbxScene::Create(fbxManager, "") };

			FbxImporter* fbxImporter{ FbxImporter::Create(fbxManager, "") };
			bool importStatus{ false };
			importStatus = fbxImporter->Initialize(animationFilename);
			_ASSERT_EXPR_A(importStatus, fbxImporter->GetStatus().GetErrorString());
			importStatus = fbxImporter->Import(fbxScene);
			_ASSERT_EXPR_A(importStatus, fbxImporter->GetStatus().GetErrorString());

			FetchAnimations(fbxScene, animationClips, samplingRate);

			fbxManager->Destroy();

			std::ofstream ofs(cerealFilename.c_str(), std::ios::binary);
			cereal::BinaryOutputArchive serialization(ofs);
			serialization(animationClips);
		}

		return true;
	}

	void SkinnedMesh::BlendAnimations(const Animation::KeyFrame * keyframes[2], float factor,
		Animation::KeyFrame & keyframe)
	{
		size_t nodeCount{ keyframes[0]->nodes.size() };
		keyframe.nodes.resize(nodeCount);
		for (size_t nodeIndex = 0; nodeIndex < nodeCount; ++nodeIndex)
		{
			XMVECTOR S[2]{
				XMLoadFloat3(&keyframes[0]->nodes.at(nodeIndex).scaling),
				XMLoadFloat3(&keyframes[1]->nodes.at(nodeIndex).scaling)
			};
			XMStoreFloat3(&keyframe.nodes.at(nodeIndex).scaling, XMVectorLerp(S[0], S[1], factor));

			XMVECTOR R[2]{
				XMLoadFloat4(&keyframes[0]->nodes.at(nodeIndex).rotation),
				XMLoadFloat4(&keyframes[1]->nodes.at(nodeIndex).rotation)
			};
			XMStoreFloat4(&keyframe.nodes.at(nodeIndex).rotation, XMQuaternionSlerp(R[0], R[1], factor));

			XMVECTOR T[2]{
				XMLoadFloat3(&keyframes[0]->nodes.at(nodeIndex).translation),
				XMLoadFloat3(&keyframes[1]->nodes.at(nodeIndex).translation)
			};
			XMStoreFloat3(&keyframe.nodes.at(nodeIndex).translation, XMVectorLerp(T[0], T[1], factor));
		}
	}

	void SkinnedMesh::FetchMeshes(FbxScene * fbxScene, std::vector<Mesh>&meshes)
	{
		for (const Scene::Node& node : scene_view.nodes)
		{
			if (node.attribute != FbxNodeAttribute::EType::eMesh)
			{
				continue;
			}

			FbxNode* fbxNode{ fbxScene->FindNodeByName(node.name.c_str()) };
			FbxMesh* fbxMesh{ fbxNode->GetMesh() };

			Mesh& mesh{ meshes.emplace_back() };
			mesh.id = fbxMesh->GetNode()->GetUniqueID();
			mesh.name = fbxMesh->GetNode()->GetName();
			mesh.nodeIndex = scene_view.IndexOf(mesh.id);
			mesh.defaultGlobalTransform = ToXMFLOAT4X4(fbxMesh->GetNode()->EvaluateGlobalTransform());

			std::vector<BoneInfluencesPerControlPoint> boneInfluences;
			FetchBoneInfluences(fbxMesh, boneInfluences);
			FetchSkeleton(fbxMesh, mesh.bindPose);

			std::vector<Mesh::Subset>& subsets{ mesh.subsets };
			const int materialCount{ fbxMesh->GetNode()->GetMaterialCount() };
			subsets.resize(materialCount > 0 ? materialCount : 1);
			for (int materialIndex = 0; materialIndex < materialCount; ++materialIndex)
			{
				const FbxSurfaceMaterial* fbxMaterial{ fbxMesh->GetNode()->GetMaterial(materialIndex) };
				subsets.at(materialIndex).materialName = fbxMaterial->GetName();
				subsets.at(materialIndex).materialId = fbxMaterial->GetUniqueID();
			}
			if (materialCount > 0)
			{
				const int polygonCount{ fbxMesh->GetPolygonCount() };
				for (int polygonIndex = 0; polygonIndex < polygonCount; ++polygonIndex)
				{
					const int materialIndex{ fbxMesh->GetElementMaterial()->GetIndexArray().GetAt(polygonIndex) };
					subsets.at(materialIndex).indexCount += 3;
				}
				uint32_t offset{ 0 };
				for (Mesh::Subset& subset : subsets)
				{
					subset.startIndexLocation = offset;
					offset += subset.indexCount;
					// This will be used as counter in the following procedures, reset to zero
					subset.indexCount = 0;
				}
			}

			const int polygonCount{ fbxMesh->GetPolygonCount() };
			mesh.vertices.resize(polygonCount * 3LL);
			mesh.indices.resize(polygonCount * 3LL);

			FbxStringList uvNames;
			fbxMesh->GetUVSetNames(uvNames);
			const FbxVector4* controlPoints{ fbxMesh->GetControlPoints() };
			for (int polygonIndex = 0; polygonIndex < polygonCount; ++polygonIndex)
			{
				const int materialIndex{ materialCount > 0 ?
				fbxMesh->GetElementMaterial()->GetIndexArray().GetAt(polygonIndex) : 0 };
				Mesh::Subset& subset{ subsets.at(materialIndex) };
				const uint32_t offset{ subset.startIndexLocation + subset.indexCount };

				for (int positionInPolygon = 0; positionInPolygon < 3; ++positionInPolygon)
				{
					const int vertexIndex{ polygonIndex * 3 + positionInPolygon };

					Vertex vertex;
					const int polygonVertex{ fbxMesh->GetPolygonVertex(polygonIndex, positionInPolygon) };
					vertex.position.x = static_cast<float>(controlPoints[polygonVertex][0]);
					vertex.position.y = static_cast<float>(controlPoints[polygonVertex][1]);
					vertex.position.z = static_cast<float>(controlPoints[polygonVertex][2]);

					const BoneInfluencesPerControlPoint& influencesPerControlPoint
					{ boneInfluences.at(polygonVertex) };

					for (size_t influenceIndex = 0; influenceIndex < influencesPerControlPoint.size();
						++influenceIndex)
					{
						if (influenceIndex < MAX_BONE_INFLUENCES)
						{
							vertex.boneWeights[influenceIndex] =
								influencesPerControlPoint.at(influenceIndex).boneWeight;
							vertex.boneIndices[influenceIndex] =
								influencesPerControlPoint.at(influenceIndex).boneIndex;
						}
					}

					if (fbxMesh->GetElementNormalCount() > 0)//法線ベクトル取得
					{
						FbxVector4 normal;
						fbxMesh->GetPolygonVertexNormal(polygonIndex, positionInPolygon, normal);
						vertex.normal.x = static_cast<float>(normal[0]);
						vertex.normal.y = static_cast<float>(normal[1]);
						vertex.normal.z = static_cast<float>(normal[2]);
					}
					if (fbxMesh->GetElementUVCount() > 0)//テクスチャ座標取得
					{
						FbxVector2 uv;
						bool unmappedUV;
						fbxMesh->GetPolygonVertexUV(polygonIndex, positionInPolygon,
							uvNames[0], uv, unmappedUV);
						vertex.texcoord.x = static_cast<float>(uv[0]);
						vertex.texcoord.y = 1.0f - static_cast<float>(uv[1]);
					}
					if (fbxMesh->GenerateTangentsData(0, false))//法線ベクトル値取得
					{
						const FbxGeometryElementTangent* tangent = fbxMesh->GetElementTangent(0);
						vertex.tangent.x = static_cast<float>(tangent->GetDirectArray().GetAt(vertexIndex)[0]);
						vertex.tangent.y = static_cast<float>(tangent->GetDirectArray().GetAt(vertexIndex)[1]);
						vertex.tangent.z = static_cast<float>(tangent->GetDirectArray().GetAt(vertexIndex)[2]);
						vertex.tangent.w = static_cast<float>(tangent->GetDirectArray().GetAt(vertexIndex)[3]);
					}

					mesh.vertices.at(vertexIndex) = std::move(vertex);
					mesh.indices.at(static_cast<size_t>(offset) + positionInPolygon) = vertexIndex;
					subset.indexCount++;
				}
			}
			for (const Vertex& v : mesh.vertices)
			{
				mesh.boundingBox[0].x = std::min<float>(mesh.boundingBox[0].x, v.position.x);
				mesh.boundingBox[0].y = std::min<float>(mesh.boundingBox[0].y, v.position.y);
				mesh.boundingBox[0].z = std::min<float>(mesh.boundingBox[0].z, v.position.z);
				mesh.boundingBox[1].x = std::max<float>(mesh.boundingBox[1].x, v.position.x);
				mesh.boundingBox[1].y = std::max<float>(mesh.boundingBox[1].y, v.position.y);
				mesh.boundingBox[1].z = std::max<float>(mesh.boundingBox[1].z, v.position.z);
			}
		}
	}

	void SkinnedMesh::FetchMaterials(FbxScene * fbxScene, std::unordered_map<uint64_t, Material>&materials)
	{
		const size_t nodeCount{ scene_view.nodes.size() };
		for (size_t nodeIndex = 0; nodeIndex < nodeCount; ++nodeIndex)
		{
			const Scene::Node& node{ scene_view.nodes.at(nodeIndex) };
			const FbxNode* fbxNode{ fbxScene->FindNodeByName(node.name.c_str()) };

			const int materialCount{ fbxNode->GetMaterialCount() };
			for (int materialIndex = 0; materialIndex < materialCount; ++materialIndex)
			{
				const FbxSurfaceMaterial* fbxMaterial{ fbxNode->GetMaterial(materialIndex) };

				Material material;
				material.name = fbxMaterial->GetName();
				material.id = fbxMaterial->GetUniqueID();
				FbxProperty fbxProperty;
				fbxProperty = fbxMaterial->FindProperty(FbxSurfaceMaterial::sDiffuse);
				if (fbxProperty.IsValid())
				{
					const FbxDouble3 color{ fbxProperty.Get<FbxDouble3>() };
					material.Kd.x = static_cast<float>(color[0]);
					material.Kd.y = static_cast<float>(color[1]);
					material.Kd.z = static_cast<float>(color[2]);
					material.Kd.w = 1.0f;

					const FbxFileTexture* fbxTexture{ fbxProperty.GetSrcObject<FbxFileTexture>() };
					material.texture_filenames[0] =
						fbxTexture ? fbxTexture->GetRelativeFileName() : "";
				}
				fbxProperty = fbxMaterial->FindProperty(FbxSurfaceMaterial::sNormalMap);
				if (fbxProperty.IsValid())
				{
					const FbxFileTexture* fileTexture{ fbxProperty.GetSrcObject<FbxFileTexture>() };
					material.texture_filenames[1] = fileTexture ? fileTexture->GetRelativeFileName() : "";
				}

				materials.emplace(material.id, std::move(material));
			}
			if (materials.size() == 0)
			{
				materials.emplace();
			}
		}
	}

	void SkinnedMesh::FetchSkeleton(FbxMesh * fbxMesh, Skeleton & bindPose)
	{
		const int deformerCount = fbxMesh->GetDeformerCount(FbxDeformer::eSkin);
		for (int deformerIndex = 0; deformerIndex < deformerCount; ++deformerIndex)
		{
			FbxSkin* skin = static_cast<FbxSkin*>(fbxMesh->GetDeformer(deformerIndex, FbxDeformer::eSkin));
			const int clusterCount = skin->GetClusterCount();
			bindPose.bones.resize(clusterCount);
			for (int clusterIndex = 0; clusterIndex < clusterCount; ++clusterIndex)
			{
				FbxCluster* cluster = skin->GetCluster(clusterIndex);

				Skeleton::Bone& bone{ bindPose.bones.at(clusterIndex) };
				bone.name = cluster->GetLink()->GetName();
				bone.id = cluster->GetLink()->GetUniqueID();
				bone.parentIndex = bindPose.IndexOf(cluster->GetLink()->GetParent()->GetUniqueID());
				bone.nodeIndex = scene_view.IndexOf(bone.id);

				// "referenceGlobalInitPosition" is used to convert from local space of model(mesh) to 
				// global space of scene.
				FbxAMatrix referenceGlobalInitPosition;
				cluster->GetTransformMatrix(referenceGlobalInitPosition);

				// 'clusterGlobalInitPosition' is used to convert from local space of bone to
				// global space of scene
				FbxAMatrix clusterGlobalInitPosition;
				cluster->GetTransformLinkMatrix(clusterGlobalInitPosition);

				// Matrices are defined using the Column Major scheme. When a FbxAMatrix represents a transformation 
				// (translation, rotation and scale), the last row of the matrix represents the translation part of 
				// the transformation.
				// Compose 'bone.offsetTransform' matrix that transforms position from mesh space to bone space.
				// This matrix is called the offset matrix.
				bone.offsetTransform
					= ToXMFLOAT4X4(clusterGlobalInitPosition.Inverse() * referenceGlobalInitPosition);
			}
		}
	}

	void SkinnedMesh::FetchAnimations(FbxScene * fbxScene, std::vector<Animation>&animationClips,
		float samplingRate /*If this value is 0, the animation data will be sampled at the default frame rate.*/)
	{
		FbxArray<FbxString*> animationStackNames;
		fbxScene->FillAnimStackNameArray(animationStackNames);
		const int animationStackCount{ animationStackNames.GetCount() };
		for (int animationStackIndex = 0; animationStackIndex < animationStackCount; ++animationStackIndex)
		{
			Animation& animationClip{ animationClips.emplace_back() };
			animationClip.name = animationStackNames[animationStackIndex]->Buffer();

			FbxAnimStack* animationStack{ fbxScene->FindMember<FbxAnimStack>(animationClip.name.c_str()) };
			fbxScene->SetCurrentAnimationStack(animationStack);

			const FbxTime::EMode timeMode{ fbxScene->GetGlobalSettings().GetTimeMode() };
			FbxTime oneSecond;
			oneSecond.SetTime(0, 0, 1, 0, 0, timeMode);
			animationClip.samplingRate = samplingRate > 0 ?
				samplingRate : static_cast<float>(oneSecond.GetFrameRate(timeMode));
			const FbxTime samplingInterval
			{ static_cast<FbxLongLong>(oneSecond.Get() / animationClip.samplingRate) };
			const FbxTakeInfo* takeInfo{ fbxScene->GetTakeInfo(animationClip.name.c_str()) };
			const FbxTime startTime{ takeInfo->mLocalTimeSpan.GetStart() };
			const FbxTime stopTime{ takeInfo->mLocalTimeSpan.GetStop() };
			for (FbxTime time = startTime; time < stopTime; time += samplingInterval)
			{
				Animation::KeyFrame& keyframe{ animationClip.sequence.emplace_back() };

				const size_t nodeCount{ scene_view.nodes.size() };
				keyframe.nodes.resize(nodeCount);
				for (size_t nodeIndex = 0; nodeIndex < nodeCount; ++nodeIndex)
				{
					FbxNode* fbxNode{ fbxScene->FindNodeByName(scene_view.nodes.at(nodeIndex).name.c_str()) };
					if (fbxNode)
					{
						Animation::KeyFrame::Node& node{ keyframe.nodes.at(nodeIndex) };
						// 'globalTransform' is a transformation matrix of a node with respect to
						// the scene's global coordinate system.
						node.globalTransform = ToXMFLOAT4X4(fbxNode->EvaluateGlobalTransform(time));

						// 'localTransform' is a transformation matrix of a node with respect to
						// its parent's local coordinate system.
						const FbxAMatrix& localTransform{ fbxNode->EvaluateLocalTransform(time) };
						node.scaling = ToXMFLOAT3(localTransform.GetS());
						node.rotation = ToXMFLOAT4(localTransform.GetQ());
						node.translation = ToXMFLOAT3(localTransform.GetT());
					}
				}
			}
		}
		for (int animationStackIndex = 0; animationStackIndex < animationStackCount; ++animationStackIndex)
		{
			delete animationStackNames[animationStackIndex];
		}
	}

	void SkinnedMesh::CreateComObjects(ID3D11Device * device, const char* fbxFilename)
	{
		for (Mesh& mesh : meshes)
		{
			HRESULT hr{ S_OK };
			D3D11_BUFFER_DESC buffer_desc{};
			D3D11_SUBRESOURCE_DATA subresource_data{};
			buffer_desc.ByteWidth = static_cast<UINT>(sizeof(Vertex) * mesh.vertices.size());
			buffer_desc.Usage = D3D11_USAGE_DEFAULT;
			buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
			buffer_desc.CPUAccessFlags = 0;
			buffer_desc.MiscFlags = 0;
			buffer_desc.StructureByteStride = 0;
			subresource_data.pSysMem = mesh.vertices.data();
			subresource_data.SysMemPitch = 0;
			subresource_data.SysMemSlicePitch = 0;
			hr = device->CreateBuffer(&buffer_desc, &subresource_data,
				mesh.vertexBuffer.ReleaseAndGetAddressOf());
			_ASSERT_EXPR(SUCCEEDED(hr), HrTrace(hr));

			buffer_desc.ByteWidth = static_cast<UINT>(sizeof(uint32_t) * mesh.indices.size());
			buffer_desc.Usage = D3D11_USAGE_DEFAULT;
			buffer_desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
			subresource_data.pSysMem = mesh.indices.data();
			hr = device->CreateBuffer(&buffer_desc, &subresource_data,
				mesh.indexBuffer.ReleaseAndGetAddressOf());
			_ASSERT_EXPR(SUCCEEDED(hr), HrTrace(hr));
#if 0
			mesh.vertices.clear();
			mesh.indices.clear();
#endif
		}

		HRESULT hr = S_OK;
		D3D11_INPUT_ELEMENT_DESC input_element_desc[]
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT },
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT },
			{ "TANGENT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT },
			{ "WEIGHTS", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT },
			{ "BONES", 0, DXGI_FORMAT_R32G32B32A32_UINT, 0, D3D11_APPEND_ALIGNED_ELEMENT },
		};
		std::string vsPath = std::string(EnginePaths::ShadersDataDir) + ("skinned_mesh_vs.cso");
		CreateVertexShaderFromCSO(device, vsPath.c_str(), vertex_shader.ReleaseAndGetAddressOf(),
			input_layout.ReleaseAndGetAddressOf(), input_element_desc, ARRAYSIZE(input_element_desc));
		std::string psPath = std::string(EnginePaths::ShadersDataDir) + ("skinned_mesh_ps.cso");
		CreatePixelShaderFromCSO(device, psPath.c_str(), pixel_shader.ReleaseAndGetAddressOf());

		//シェーダーリソースビューオブジェクト生成
		{
			for (std::unordered_map<uint64_t, Material>::iterator iterator = materials.begin();
				iterator != materials.end(); ++iterator)
			{
				for (size_t textureIndex = 0; textureIndex < 2; ++textureIndex)
				{
					if (iterator->second.texture_filenames[textureIndex].size() > 0)
					{
						std::filesystem::path path(fbxFilename);
						path.replace_filename(iterator->second.texture_filenames[textureIndex]);
#if 0
						D3D11_TEXTURE2D_DESC texture2d_desc;
						LoadTextureFromFile(device, path.c_str(),
							iterator->second.shader_resource_views[textureIndex].GetAddressOf(), &texture2d_desc);
#else
						// Load texture from asset database
						AssetMeta* meta = AssetDatabase::GetOrImport(path);
						if (meta)
						{
							if (TextureImporter* textureImporter = dynamic_cast<TextureImporter*>(ImporterRegistry::Find(*meta)))
							{
								if (auto textureResource = std::dynamic_pointer_cast<AssetTexture>(textureImporter->Import(*meta)))
								{
									iterator->second.shader_resource_views[textureIndex] = textureResource->GetSRV();
								}
							}
						}
#endif // 0

					}
					else
					{
						MakeDummyTexture(device, iterator->second.shader_resource_views[textureIndex].GetAddressOf(),
							textureIndex == 1 ? 0xFFFF7F7F : 0xFFFFFFFF, 16);
					}
				}
			}
		}

		D3D11_BUFFER_DESC buffer_desc{};
		buffer_desc.ByteWidth = sizeof(Constants);
		buffer_desc.Usage = D3D11_USAGE_DEFAULT;
		buffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		hr = device->CreateBuffer(&buffer_desc, nullptr, constant_buffer.ReleaseAndGetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), HrTrace(hr));
	}

	void SkinnedMesh::UpdateAnimation(Animation::KeyFrame & keyframe)
	{
		size_t nodeCount{ keyframe.nodes.size() };
		for (size_t nodeIndex = 0; nodeIndex < nodeCount; ++nodeIndex)
		{
			Animation::KeyFrame::Node& node{ keyframe.nodes.at(nodeIndex) };
			XMMATRIX S{ XMMatrixScaling(node.scaling.x, node.scaling.y, node.scaling.z) };
			XMMATRIX R{ XMMatrixRotationQuaternion(XMLoadFloat4(&node.rotation)) };
			XMMATRIX T{ XMMatrixTranslation(node.translation.x, node.translation.y, node.translation.z) };

			int64_t parentIndex{ scene_view.nodes.at(nodeIndex).parentIndex };
			XMMATRIX P{ parentIndex < 0 ? XMMatrixIdentity() :
				XMLoadFloat4x4(&keyframe.nodes.at(parentIndex).globalTransform) };

			XMStoreFloat4x4(&node.globalTransform, S * R * T * P);
		}
	}

	void SkinnedMesh::Render(ID3D11DeviceContext * immediate_context,
		const XMFLOAT4X4 & world, const XMFLOAT4 & material_color,
		const Animation::KeyFrame * keyframe)
	{
		for (const Mesh& mesh : meshes)
		{
			uint32_t stride{ sizeof(Vertex) };
			uint32_t offset{ 0 };
			immediate_context->IASetVertexBuffers(0, 1, mesh.vertexBuffer.GetAddressOf(), &stride, &offset);
			immediate_context->IASetIndexBuffer(mesh.indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
			immediate_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			immediate_context->IASetInputLayout(input_layout.Get());

			immediate_context->VSSetShader(vertex_shader.Get(), nullptr, 0);
			immediate_context->PSSetShader(pixel_shader.Get(), nullptr, 0);

			Constants data;

			if (keyframe && keyframe->nodes.size() > 0)
			{
				const Animation::KeyFrame::Node& meshNode{ keyframe->nodes.at(mesh.nodeIndex) };
				XMStoreFloat4x4(&data.world, XMLoadFloat4x4(&meshNode.globalTransform) * XMLoadFloat4x4(&world));

				const size_t boneCount{ mesh.bindPose.bones.size() };
				_ASSERT_EXPR(boneCount < MAX_BONES, L"The value of the 'boneCount' has exceeded MAX_BONES.");

				for (size_t boneIndex = 0; boneIndex < boneCount; ++boneIndex)
				{
					const Skeleton::Bone& bone{ mesh.bindPose.bones.at(boneIndex) };
					const Animation::KeyFrame::Node& boneNode{ keyframe->nodes.at(bone.nodeIndex) };
					XMStoreFloat4x4(&data.bone_transforms[boneIndex],
						XMLoadFloat4x4(&bone.offsetTransform) *
						XMLoadFloat4x4(&boneNode.globalTransform) *
						XMMatrixInverse(nullptr, XMLoadFloat4x4(&mesh.defaultGlobalTransform))
					);
				}
			}
			else
			{
				XMStoreFloat4x4(&data.world, XMLoadFloat4x4(&mesh.defaultGlobalTransform) * XMLoadFloat4x4(&world));
				for (size_t boneIndex = 0; boneIndex < MAX_BONES; ++boneIndex)
				{
					data.bone_transforms[boneIndex] = { 1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1 };
				}
			}
			for (const Mesh::Subset& subset : mesh.subsets)
			{
				const Material& material{ materials.at(subset.materialId) };
				XMStoreFloat4(&data.material_color, XMLoadFloat4(&material_color) * XMLoadFloat4(&material.Kd));
				immediate_context->UpdateSubresource(constant_buffer.Get(), 0, 0, &data, 0, 0);
				immediate_context->VSSetConstantBuffers(0, 1, constant_buffer.GetAddressOf());

				//シェーダーリソースビューをバインド
				immediate_context->PSSetShaderResources(0, 1, material.shader_resource_views[0].GetAddressOf());
				immediate_context->PSSetShaderResources(1, 1, material.shader_resource_views[1].GetAddressOf());


				immediate_context->DrawIndexed(subset.indexCount, subset.startIndexLocation, 0);
			}
		}
	}
}
