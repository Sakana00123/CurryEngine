#pragma once
#include <type_traits>
#include <random>
namespace CurryEngine
{
	template<typename T>
	struct Range
	{
		// 範囲の最小値と最大値
		T min;
		T max;

		// min から max の範囲でランダムな値を取得
		T GetRandom() const
		{
			if (min == max) return min;
			//float t = static_cast<float>(rand()) / RAND_MAX;
			//return min + (max - min) * t;

			// std::mt19937 と std::uniform_real_distribution を使用してランダムな値を生成
			static std::random_device rd;  // 非決定的な乱数生成器
			static std::mt19937 gen(rd()); // メルセンヌ・ツイスターの乱数生成器
			if constexpr (std::is_integral_v<T>)
			{
				std::uniform_int_distribution<T> dis(min, max);
				return dis(gen);
			}
			else if constexpr (std::is_floating_point_v<T>)
			{
				std::uniform_real_distribution<T> dis(min, max);
				return dis(gen);
			}
			else
			{
				// T が整数型でも浮動小数点型でもない場合は、rand() を使用して値を生成
				float t = static_cast<float>(rand()) / RAND_MAX;
				return min + (max - min) * t;
			}
		}
	};
}
