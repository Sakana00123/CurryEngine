#pragma once
#include <string>

namespace CurryEngine
{
	struct CommandResult
	{
		bool success;
		std::string message;
	};

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
		virtual CommandResult Execute() = 0;
		/**
		 * @brief Undo の処理を行います。
		 */
		virtual CommandResult Undo() = 0;

		virtual std::string GetDescription() const { return "No description provided."; }
	};
}
