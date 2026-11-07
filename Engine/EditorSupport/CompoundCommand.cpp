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
		for (const auto& command : m_commands) {
			auto result = command->Execute();
			if (!result.success) {
				return result;
			}
		}
		return { true, "" };
	}
	CommandResult CompoundCommand::Undo()
	{
		for (auto it = m_commands.rbegin(); it != m_commands.rend(); ++it) {
			auto result = (*it)->Undo();
			if (!result.success) {
				return result;
			}
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
