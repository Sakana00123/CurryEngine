#pragma once
#include "IEditorCommand.h"
#include <stack>
#include <memory>

namespace CurryEngine
{
	class UndoRedoStack
	{
	public:
		// コマンドを実行し、Undoスタックに追加する
		void ExecuteCommand(std::shared_ptr<IEditorCommand> command)
		{
			command->Execute();
			m_undoStack.push(std::move(command));
			ClearRedo(); // 新しいコマンドが実行されたときにRedoスタックをクリア
		}
		void Undo()
		{
			if (!CanUndo())
				return;
			auto command = std::move(m_undoStack.top());
			m_undoStack.pop();
			command->Undo();
			m_redoStack.push(std::move(command));
		}
		void Redo()
		{
			if (!CanRedo())
				return;
			auto command = std::move(m_redoStack.top());
			m_redoStack.pop();
			command->Execute();
			m_undoStack.push(std::move(command));
		}

		bool CanUndo() const { return !m_undoStack.empty(); }
		bool CanRedo() const { return !m_redoStack.empty(); }

		/// <summary>
		/// Undo/Redoスタックの内容を文字列のリストとして取得する。主にUIで履歴を表示するために使用される。
		/// </summary>
		/// <param name="undoDescriptions">Undoスタックのコマンドの説明を格納するリスト。最も新しいコマンドが先頭に来る。</param>
		/// <param name="redoDescriptions">Redoスタックのコマンドの説明を格納するリスト。最も新しいコマンドが先頭に来る。</param>
		/// <returns>Undoスタックのコマンドの数。</returns>
		int GetUndoRedoDescriptions(std::vector<std::string>& undoDescriptions, std::vector<std::string>& redoDescriptions) const
		{
			undoDescriptions = GetUndoHistory();
			redoDescriptions = GetRedoHistory();
			return static_cast<int>(undoDescriptions.size());
		}

	private:
		// Redoスタックをクリアする
		void ClearRedo()
		{
			while (!m_redoStack.empty())
				m_redoStack.pop();
		}

		std::vector<std::string> GetUndoHistory() const
		{
			std::vector<std::string> history;
			std::stack<std::shared_ptr<IEditorCommand>> tempStack = m_undoStack;
			while (!tempStack.empty())
			{
				history.push_back(tempStack.top()->GetDescription());
				tempStack.pop();
			}
			return history;
		}
		std::vector<std::string> GetRedoHistory() const
		{
			std::vector<std::string> history;
			std::stack<std::shared_ptr<IEditorCommand>> tempStack = m_redoStack;
			while (!tempStack.empty())
			{
				history.push_back(tempStack.top()->GetDescription());
				tempStack.pop();
			}
			return history;
		}

		std::stack<std::shared_ptr<IEditorCommand>> m_undoStack; // Undoスタック。コマンドが実行されるとここに追加される。(shared_ptrを使用しているのは、Undo/Redoスタックがコマンドの所有権を持ち、コマンドが複数の場所で共有される可能性があるため。)
		std::stack<std::shared_ptr<IEditorCommand>> m_redoStack; // Redoスタック。Undoされたコマンドがここに追加される。新しいコマンドが実行されるとクリアされる。 (shared_ptrを使用しているのは、Undo/Redoスタックがコマンドの所有権を持ち、コマンドが複数の場所で共有される可能性があるため。)
	};
}