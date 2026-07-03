#pragma once
struct PropertyInfo;
struct PropertyDrawContext;

namespace CurryEngine
{
	/**
	 * @brief プロパティ描画のインターフェースを定義するクラス。プロパティの型ごとにこのインターフェースを実装したクラスを作成し、プロパティ描画関数を提供します。
	 * @details IPropertyDrawer は、プロパティの型に応じた描画関数を提供するためのインターフェースです。各プロパティタイプ（int、float、Vector3 など）に対して、このインターフェースを実装したクラスを作成し、プロパティ描画関数を実装します。
	 */
	class IPropertyDrawer
	{
	public:
		virtual ~IPropertyDrawer() = default;
		/**
		 * @brief プロパティを描画するための純粋仮想関数。派生クラスでこの関数をオーバーライドして、特定のプロパティタイプの描画処理を実装します。
		 * @param prop 描画するプロパティのメタ情報。getter や setter を使用してプロパティ値を取得・設定できます。
		 * @param context プロパティ描画のコンテキスト。複数選択されているオブジェクトのリストやその他の描画に必要な情報が含まれます。
		 */
		virtual void Draw(const PropertyInfo& prop, const PropertyDrawContext& context) = 0;
	};
}