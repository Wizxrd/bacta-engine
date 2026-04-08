#pragma once

#include "State.hpp"

#include <memory>
#include <vector>

class StateManager {
	public:
		void push(std::unique_ptr<State> state);
		void pop();
		void clear();
		bool empty() const;
		State& current();

	private:
		std::vector<std::unique_ptr<State>> mStates;
};
