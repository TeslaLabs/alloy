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
#include "AlloyUndoRedo.h"
namespace aly {
	Action::Action(const std::string& name):history(nullptr),status(ActionStatus::Executable), name(name) {

	}
	ActionHistory::ActionHistory(const std::string& name,int maxUndo,int maxRedo):name(name),maxUndo(maxUndo),maxRedo(maxRedo) {

	}
	bool ActionHistory::execute(const ActionPtr& action) {
		action->history = this;
		if (action->execute()) {
			std::cout <<name<<":: "<< *action << std::endl;
			action->status = ActionStatus::Undoable;
			while ((int)undoHistory.size() >= maxUndo) {
				undoHistory.erase(undoHistory.begin());
			}
			undoHistory.push_back(action);
			return true;
		} else {
			action->history = nullptr;
			return false;
		}
	}
	bool ActionHistory::undo() {
		if (undoHistory.size() > 0) {
			ActionPtr last=undoHistory.back();
			if (last->undo()) {
				std::cout << name << ":: " << *last << std::endl;
				last->status = ActionStatus::Redoable;
				undoHistory.erase(undoHistory.end() - 1);
				while ((int)redoHistory.size() >= maxRedo) {
					redoHistory.erase(redoHistory.begin());
				}
				redoHistory.push_back(last);
				if (onUndo) {
					onUndo(last);
				}
				return true;
			}
			else {
				return false;
			}
		} else {
			return false;
		}
	}
	bool ActionHistory::redo() {
		if (undoHistory.size() > 0) {
			ActionPtr last = redoHistory.back();
			if (last->redo()) {
				std::cout << name << ":: " << *last << std::endl;
				last->status = ActionStatus::Undoable;
				redoHistory.erase(redoHistory.end() - 1);
				while ((int)undoHistory.size() >= maxUndo) {
					undoHistory.erase(undoHistory.begin());
				}
				undoHistory.push_back(last);
				if (onRedo) {
					onRedo(last);
				}
				return true;
			}
			else {
				return false;
			}
		}
		else {
			return false;
		}
	}
	void ActionHistory::clear() {
		for (ActionPtr action : undoHistory) {
			action->history = nullptr;
			action->status = ActionStatus::Executable;
		}
		undoHistory.clear();
		for (ActionPtr action : redoHistory) {
			action->history = nullptr;
			action->status = ActionStatus::Executable;
		}
		redoHistory.clear();
	}
	bool UndoableAction::undo() {
		if (history == nullptr)
			throw std::runtime_error("Cannot undo unattached action.");
		if (undoFunction) {
			return undoFunction();
		}
		else {
			throw std::runtime_error("Undo action not implemented.");
		}
	}
	bool UndoableAction::redo() {
		if (history == nullptr)
			throw std::runtime_error("Cannot redo unattached action.");
		if (redoFunction) {
			return redoFunction();
		}
		else if (executeFunction) {
			return executeFunction();
		}
		else {
			throw std::runtime_error("Redo action not implemented.");
		}
	}
	ActionPtr MakeAction(const std::string& name, const std::function<bool()>& doAction, const std::function<bool()>& undoAction) {
		UndoableAction* ua = new UndoableAction(name);
		ActionPtr action = ActionPtr(ua);
		ua->executeFunction = doAction;
		ua->undoFunction = undoAction;
		return action;
	}
	ActionPtr ActionHistory::getLast() const {
		if (!undoHistory.empty()) {
			return undoHistory.back();
		}
		else {
			return ActionPtr();
		}
	}
	ActionPtr MakeAction(const std::string& name, const std::function<bool()>& doAction, const std::function<bool()>& undoAction, const std::function<bool()>& redoAction) {
		UndoableAction* ua = new UndoableAction(name);
		ActionPtr action = ActionPtr(ua);
		ua->executeFunction = doAction;
		ua->undoFunction = undoAction;
		ua->redoFunction = redoAction;
		return action;
	}
	ActionPtr MakeAction(const std::string& name, const std::function<bool()>& doAction) {
		ActionPtr action = ActionPtr(new Action(name));
		action->executeFunction = doAction;
		return action;
	}

}
