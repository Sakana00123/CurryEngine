#include "pch.h"
#include "CompoundCommand.h"

namespace CurryEngine
{
	void CompoundCommand::AddCommand(std::unique_ptr<IEditorCommand> command)
	{
		m_commands.push_back(std::move(command));
	}

	bool CompoundCommand::IsEmpty() const
	{
		return m_commands.empty();
	}

	CommandResult CompoundCommand::Execute()
	{
		int successCount = 0;
		for (const auto& command : m_commands) {
			auto result = command->Execute();
			if (!result.success) {
				// 失敗した場合、成功したコマンドを元に戻す
				bool undoSuccess = true;
				for (int i = successCount - 1; i >= 0; --i) {
					auto undoResult = m_commands[i]->Undo();
					if (!undoResult.success) {
						// 元に戻すのも失敗した場合、結果のメッセージに追加する
						result.message += ", Failed to undo command: " + m_commands[i]->GetDescription() + " - " + undoResult.message;
						undoSuccess = false;
					}
				}
				if (!undoSuccess) {
					result.message += ", Some commands could not be undone after execution failure.";
				}
				return result;
			}
			++successCount;
		}
		return { true, "" };
	}
	CommandResult CompoundCommand::Undo()
	{
		int successCount = 0;
		for (auto it = m_commands.rbegin(); it != m_commands.rend(); ++it) {
			auto result = (*it)->Undo();
			if (!result.success) {
				// 失敗した場合、成功したコマンドを再実行する
				bool redoSuccess = true;
				for (int i = m_commands.size() - successCount; i < m_commands.size(); ++i) {
					auto executeResult = m_commands[i]->Execute();
					if (!executeResult.success) {
						// 再実行も失敗した場合、結果のメッセージに追加する
						result.message += ", Failed to redo command: " + m_commands[i]->GetDescription() + " - " + executeResult.message;
						redoSuccess = false;
					}
				}
				if (!redoSuccess) {
					result.message += ", Some commands could not be redone after undo failure.";
				}
				return result;
			}
			++successCount;
		}
		return { true, "" };
	}
	std::string CompoundCommand::GetDescription() const
	{
		if (m_commands.empty()) {
			return m_description + ": (No commands)";
		}
		
		return m_description + ": " + m_commands.front()->GetDescription() + " + " + std::to_string(m_commands.size() - 1) + " more";
	}
}
