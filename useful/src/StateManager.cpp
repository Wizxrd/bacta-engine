#include "StateManager.hpp"

void StateManager::push(std::unique_ptr<State> state) {
	mStates.push_back(std::move(state));
}

void StateManager::pop() {
	if (!mStates.empty()) {
		mStates.pop_back();
	}
}

void StateManager::clear() {
	mStates.clear();
}

bool StateManager::empty() const {
	return mStates.empty();
}

State& StateManager::current() {
	return *mStates.back();
}
