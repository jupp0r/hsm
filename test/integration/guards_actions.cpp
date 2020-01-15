#include "hsm/hsm.h"

#include <boost/hana.hpp>
#include <gtest/gtest.h>

#include <future>
#include <memory>

namespace {

// States
struct S1 {
};
struct S2 {
};

// Events
struct e1 {
    e1(const std::shared_ptr<std::promise<void>>& called)
        : called(called)
    {
    }
    std::shared_ptr<std::promise<void>> called;
};
struct e2 {
};
struct e3 {
};
struct e4 {
};

// Guards
const auto g1 = [](auto) { return true; };
const auto g2 = [](auto) { return false; };
const auto g3 = [](auto) { return true; };

// Actions
const auto a1 = [](auto /*event*/) {};
const auto a2 = [](auto event) { event.called->set_value(); };

using namespace ::testing;
using namespace boost::hana;

struct SubState {
    constexpr auto make_transition_table()
    {
        // clang-format off
        return hsm::transition_table(
            hsm::transition(S1 {}, hsm::event<e1> {}, g1, a2, S1 {})
        );
        // clang-format on
    }

    constexpr auto initial_state()
    {
        return hsm::initial(S1 {});
    }
};

struct MainState {
    constexpr auto make_transition_table()
    {
        // clang-format off
        return hsm::transition_table(
            hsm::transition(S1 {}, hsm::event<e1> {}, g1, a2, S1 {}),
            hsm::transition(S1 {}, hsm::event<e2> {}, g1, a1, SubState {}),
            hsm::transition(S1 {}, hsm::event<e3> {}, g2, a1, S2 {}),
            hsm::transition(S1 {}, hsm::event<e4> {}, g3, a1, S2 {})
        );
        // clang-format on
    }

    constexpr auto initial_state()
    {
        return hsm::initial(S1 {});
    }
};

}

class GuardsActionsTests : public Test {
  protected:
    hsm::sm<MainState> sm;
};

TEST_F(GuardsActionsTests, should_call_action)
{
    auto actionCalled = std::make_shared<std::promise<void>>();

    sm.process_event(e1 { actionCalled });

    ASSERT_EQ(
        std::future_status::ready, actionCalled->get_future().wait_for(std::chrono::seconds(1)));
}

TEST_F(GuardsActionsTests, should_call_substate_action)
{
    auto actionCalled = std::make_shared<std::promise<void>>();

    sm.process_event(e2 {});
    sm.process_event(e1 { actionCalled });

    ASSERT_EQ(
        std::future_status::ready, actionCalled->get_future().wait_for(std::chrono::seconds(1)));
}

TEST_F(GuardsActionsTests, should_block_transition_guard)
{
    ASSERT_TRUE(sm.is(S1 {}));
    sm.process_event(e3 {});
    ASSERT_TRUE(sm.is(S1 {}));
}

TEST_F(GuardsActionsTests, should_not_block_transition_by_guard)
{
    ASSERT_TRUE(sm.is(S1 {}));
    sm.process_event(e4 {});
    ASSERT_TRUE(sm.is(S2 {}));
}