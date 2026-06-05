#pragma once
#include "IEditorCommand.h"
#include <functional>

namespace CurryEngine
{
	template<typename T>
	class SetValueCommand : public IEditorCommand
	{
	public:
		/// <summary>
		/// 値を変更するコマンド。Executeで新しい値をセットし、Undoで古い値をセットする。
		/// </summary>
		/// <param name="description">コマンドの説明。Undo/Redoスタックで表示される。</param>
		/// <param name="setter">値をセットする関数。ExecuteとUndoの両方で呼び出される。</param>
		/// <param name="oldValue">古い値。Undoでセットされる。</param>
		/// <param name="newValue">新しい値。Executeでセットされる。</param>
		SetValueCommand(const std::string& description, std::function<void(const T&)> setter, T oldValue, T newValue)
			: m_description(description)
			, m_setter(std::move(setter))
			, m_oldValue(std::move(oldValue))
			, m_newValue(std::move(newValue)) { }

		// IEditorCommandの実装
		void Execute() override { m_setter(m_newValue); }
		void Undo() override { m_setter(m_oldValue); }
		std::string GetDescription() const override { return m_description; }
	private:
		std::string m_description;
		std::function<void(const T&)> m_setter;
		T m_oldValue;
		T m_newValue;
	};
}