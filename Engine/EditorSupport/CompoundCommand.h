#pragma once
#include "IEditorCommand.h"
#include <vector>
#include <memory>

namespace CurryEngine
{
	class CompoundCommand : public IEditorCommand
	{
	public:
		/// <summary>
		/// 複数のコマンドをまとめて実行/元に戻すためのコマンド。Executeで追加された順番でコマンドを実行し、Undoで逆順でコマンドを元に戻す。
		/// </summary>
		/// <param name="description">コマンドの説明。Undo/Redoスタックで表示される。</param>
		CompoundCommand(const std::string& description = "Compound Command") : m_description(description) {}
		~CompoundCommand() override = default;

		/**
		 * @brief コマンドを追加する
		 * @param command 追加するコマンド。所有権はCompoundCommandに移る。
		 */
		void AddCommand(std::unique_ptr<IEditorCommand> command);

		/**
		 * @brief コマンドが空かどうかを返す
		 * @return コマンドが空ならtrue、そうでなければfalse
		 */
		bool IsEmpty() const;

		void Execute() override;
		
		void Undo() override;

		std::string GetDescription() const override;

	private:
		std::vector<std::unique_ptr<IEditorCommand>> m_commands;
		std::string m_description;
	};
}