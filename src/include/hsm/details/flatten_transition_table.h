#pragma once

#include <boost/hana.hpp>

namespace hsm
{
    
namespace bh{
    using namespace boost::hana;    
}    

template <class T> constexpr auto flatten_sub_transition_table(T&& state);

const auto flatten_transition_table = [](auto state){
        auto transitionTable = state.make_transition_table();
        auto collectedTransitions = bh::fold_left(transitionTable, bh::make_tuple(), [](auto transitions, auto transition) {
            return bh::concat(
                bh::append(transitions, transition),
                flatten_sub_transition_table(bh::back(transition)));
        });
    return collectedTransitions;

};

template <class T> constexpr 
auto flatten_sub_transition_table(T&& state)
{
    return bh::if_(
        has_transition_table(state),
        [](auto& stateWithTransitionTable) {
            return flatten_transition_table(stateWithTransitionTable);
        },
        [](auto&) { return bh::make_tuple(); })(state);
};

} // namespace hsm