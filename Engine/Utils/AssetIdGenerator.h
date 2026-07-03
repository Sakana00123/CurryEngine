#pragma once
#include <string>


namespace CurryEngine
{
	namespace Utils
	{
		namespace IdGenerator
		{
			/**
			 * @brief アセットIDを生成する関数。
			 * @details この関数は、アセットの一意なIDを生成します。
			 * @return 生成されたアセットID。
			 */
			std::string GenerateAssetId();
		}
	}
}