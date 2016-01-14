#include "gtest/gtest.h"
#include "../cockatrice/src/chatview.h"

class QWidget;

namespace {
    class FooTest : public ::testing::Test {

    };

    class MockTabSupervisor : public TabSupervisor {
        public:
        MockTabSupervisor(
                AbstractClient *_client = nullptr,
                QWidget *parent = nullptr) : TabSupervisor(_client, parent) { }
    };

    TEST(DummyTest, Works) {
        ASSERT_EQ(1, 1) << "One is not equal to one";
    }

    TEST(ChatViewTest, HandlesUrlTag) {
        bool showTimestamps = true;
        auto const *tabSupervisor = new MockTabSupervisor();
        auto chatView = new ChatView(tabSupervisor, nullptr, showTimestamps);
        // TODO Add some chat messages with [tags] and make sure the result has links in it.
    }
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}