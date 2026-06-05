#pragma once
#include "Engine/EditorSupport/IEditorCommand.h"
#include "Engine/EditorSupport/UndoRedoStack.h"
#include <vector>

namespace CurryEngine
{
	class History
	{
	public:
		// コマンドを実行し、履歴に追加する
		static void ExecuteCommand(std::shared_ptr<IEditorCommand> command)
		{
			GetUndoRedoStack().ExecuteCommand(std::move(command));
		}
		static void Undo() { GetUndoRedoStack().Undo(); }
		static void Redo() { GetUndoRedoStack().Redo(); }
		static bool CanUndo() { return GetUndoRedoStack().CanUndo(); }
		static bool CanRedo() { return GetUndoRedoStack().CanRedo(); }

		static void GetUndoRedoDescriptions(std::vector<std::string>& undoDescriptions, std::vector<std::string>& redoDescriptions)
		{
			GetUndoRedoStack().GetUndoRedoDescriptions(undoDescriptions, redoDescriptions);
		}
	private:
		static UndoRedoStack& GetUndoRedoStack();
	};
}