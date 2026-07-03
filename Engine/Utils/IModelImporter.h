#pragma once
#include <string>
#include <vector>
#include "Engine/Resources/ModelAsset.h"

namespace CurryEngine
{
	namespace Utils
	{
		/**
		 * @brief モデルインポーターのインターフェース。モデルファイルを読み込み、`ModelAsset` に変換するための抽象クラス。
		 * @details 具体的なモデルフォーマット（例: OBJ, FBX, GLTF）ごとにこのインターフェースを実装することで、異なるフォーマットのモデルを統一的に扱うことができます。
		 */
		class IModelImporter
		{
		public:
			virtual ~IModelImporter() = default;

			/**
			 * @brief モデルファイルを読み込み、`ModelAsset` に変換する純粋仮想関数。
			 * @param path 読み込むモデルファイルのパス。
			 * @param asset 読み込んだモデルデータを格納する `ModelAsset` オブジェクトへの参照。
			 */
			virtual bool Import(const std::string& path, ModelAsset& asset) = 0;

			/**
			 * @brief このインポーターがサポートするファイル拡張子のリストを返す純粋仮想関数。
			 * @return サポートするファイル拡張子のリスト（例: {".obj", ".fbx", ".gltf"}）。
			 */
			virtual std::vector<std::string> GetSupportedExtensions() const = 0;
		};
	}
}