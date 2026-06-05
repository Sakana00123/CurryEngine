#pragma once
#include <string>

namespace CurryEngine
{
	class IEditorCommand
	{
	public:
		virtual ~IEditorCommand() = default;
		virtual void Execute() = 0;
		virtual void Undo() = 0;
		virtual std::string GetDescription() const { return "No description provided."; }
	};
}