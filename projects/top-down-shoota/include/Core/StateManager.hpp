#pragma once

#include "Core/State.hpp"

#include <vector>
#include <memory>

namespace Core{
	class StateManager{
		public:
			void Push(std::unique_ptr<State> state);
			void Pop();
			void Replace(std::unique_ptr<State> state);
			void Clear();
			bool IsEmpty() const;
			State& GetCurrent();
		private:
			std::vector<std::unique_ptr<State>> mStates;
	};
}