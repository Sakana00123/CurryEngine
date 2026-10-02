#pragma once
#include <string>

namespace CurryEngine
{
	/** @brief IEditorCommand を表すクラスです。 */
	class IEditorCommand
	{
	public:
		/**
		 * @brief IEditorCommand を破棄します。
		 */
		virtual ~IEditorCommand() = default;
		/**
		 * @brief 処理を実行します。
		 */
		virtual void Execute() = 0;
		/**
		 * @brief Undo の処理を行います。
		 */
		virtual void Undo() = 0;
		virtual std::string GetDescription() const { return "No description provided."; }
	};
}