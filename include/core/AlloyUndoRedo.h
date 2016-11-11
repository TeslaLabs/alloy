/*
* Copyright(C) 2015, Blake C. Lucas, Ph.D. (img.science@gmail.com)
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*/
#include "AlloyAny.h"
#include "AlloyEnum.h"
#include <memory>
#include <stack>
#include <vector>
namespace aly {
	class ActionHistory;
	
	class Action {
		
	protected:
		ActionHistory* history;
		ActionStatus status;
		std::string name;
	public:
		std::string getName() const {
			return name;
		}
		ActionStatus getStatus() const {
			return status;
		}
		friend class ActionHistory;
		std::function<bool()> executeFunction;
		ActionHistory* getHistory() {
			return history;
		}
		Action(const std::string& name="");
		virtual bool execute() {
			if(history==nullptr)
				throw std::runtime_error("Cannot execute unattached action.");
			if (executeFunction) {
				return executeFunction();
			}
			else {
				throw std::runtime_error("Execute action not implemented.");
			}
		}
		virtual bool undo() { return false; }
		virtual bool redo() { return false; }
		virtual ~Action() {}
	};
	class UndoableAction : public Action {
	public:
		friend class ActionHistory;
		UndoableAction(const std::string& name):Action(name) {

		}
		std::function<bool()> undoFunction;
		std::function<bool()> redoFunction;
		virtual bool undo() override;
		virtual bool redo() override;
		virtual ~UndoableAction() {}
	};
	typedef std::shared_ptr<Action> ActionPtr;
	typedef std::shared_ptr<UndoableAction> UndoableActionPtr;
	class ActionHistory {
	protected:
		std::string name;
		const int maxUndo;
		const int maxRedo;
		std::vector<ActionPtr> undoHistory;
		std::vector<ActionPtr> redoHistory;
	public:
		std::function<void(const ActionPtr& action)> onUndo;
		std::function<void(const ActionPtr& action)> onRedo;
		bool execute(const std::shared_ptr<Action>& action);
		bool undo();
		bool redo();
		void clear();
		bool isUndoable() const {
			return (!undoHistory.empty());
		}
		bool isRedoable() const {
			return (!redoHistory.empty());
		}
		std::string getName() const {
			return name;
		}
		ActionPtr getLast() const;
		size_t size() const {
			return (undoHistory.size() + redoHistory.size());
		}
		ActionHistory(const std::string& name="",int maxUndo=16,int maxRedo=16);
	};
	ActionPtr MakeAction(const std::string& name, const std::function<bool()>& doAction, const std::function<bool()>& undoAction);
	ActionPtr MakeAction(const std::string& name, const std::function<bool()>& doAction, const std::function<bool()>& undoAction, const std::function<bool()>& redoAction);
	ActionPtr MakeAction(const std::string& name, const std::function<bool()>& doAction);

	template<class C, class R> std::basic_ostream<C, R> & operator <<(
		std::basic_ostream<C, R> & ss, const Action& action) {
		return ss << action.getName() << " [" << action.getStatus() << "]";
	}
}
