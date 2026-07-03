#include "pch.h"
#include "AssetIdGenerator.h"
#include <random>

namespace CurryEngine
{
	namespace Utils
	{
		namespace IdGenerator
		{
			std::string GenerateAssetId()
			{
				// グローバルな一意のIDを生成する
				static std::random_device rd;
				static std::mt19937 gen(rd());
				static std::uniform_int_distribution<> dis(0, 15);
				
				// 32桁の16進数のIDを生成
				std::string id;
				for (int i = 0; i < 32; ++i) {
					id += "0123456789abcdef"[dis(gen)];
				}

				// 生成されたIDを返す
				return id;
			}
		}
	}
}