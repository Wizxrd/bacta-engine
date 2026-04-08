#include "Core/StateManager.hpp"

namespace Core{
	void StateManager::Push(std::unique_ptr<State> state){
		mStates.push_back(std::move(state));
	}

	void StateManager::Pop(){
		if (!mStates.empty()){
			mStates.pop_back();
		}
	}

	void StateManager::Replace(std::unique_ptr<State> state){
		if (!IsEmpty()){
			Pop();
		}
		Push(std::move(state));
	}

	void StateManager::Clear(){
		mStates.clear();
	}

	bool StateManager::IsEmpty() const{
		return mStates.empty();
	}

	State& StateManager::GetCurrent(){
		return *mStates.back();
	}
}