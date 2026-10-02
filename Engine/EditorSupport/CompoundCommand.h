#pragma once
#include "IEditorCommand.h"
#include <vector>
#include <memory>

namespace CurryEngine
{
	/** @brief CompoundCommand を表すクラスです。 */
	class CompoundCommand : public IEditorCommand
	{
	public:
		/// <summary>
		/// 複数のコマンドをまとめて実行/元に戻すためのコマンド。Executeで追加された順番でコマンドを実行し、Undoで逆順でコマンドを元に戻す。
		/// </summary>
		/// <param name="description">コマンドの説明。Undo/Redoスタックで表示される。</param>
		CompoundCommand(const std::string& description = "Compound Command") : m_description(description) {}
		/**
		 * @brief CompoundCommand を破棄します。
		 */
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

		/**
		 * @brief 処理を実行します。
		 */
		void Execute() override;
		
		/**
		 * @brief Undo の処理を行います。
		 */
		void Undo() override;

		/**
		 * @brief GetDescription に対応する値を取得します。
		 * @return 処理結果を返します。
		 */
		std::string GetDescription() const override;

	private:
		std::vector<std::unique_ptr<IEditorCommand>> m_commands;
		std::string m_description;
	};
}