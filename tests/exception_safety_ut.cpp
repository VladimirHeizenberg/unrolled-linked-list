#include <unrolled_list.h>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <list>

class NodeTag {};

class SomeObj2 {
public:
    static inline int CopiesCount = 0;
    static inline int DestructorCalled = 0;

    SomeObj2() = default;

    SomeObj2(SomeObj2&&) noexcept {}

    SomeObj2(const SomeObj2&) {
        ++CopiesCount;
        if (CopiesCount == 3) {
            throw std::runtime_error("");
        }
    }

    ~SomeObj2() {
        ++DestructorCalled;
    }
};

struct Bad {};
struct Good {
    std::string Name;
};

struct BadOrGood {
    BadOrGood() = default;

    BadOrGood(Bad b) {
        throw std::runtime_error("");
    }

    BadOrGood(Good g)
        : Name(g.Name) {}

    std::string Name;
};

template<typename T>
class TestAllocator {
public:
    using value_type = T;
    using pointer = T*;
    using size_type = size_t;
    using is_always_equal = std::true_type;

    static inline int DeallocationCount = 0;
    static inline int ElementsDeallocated = 0;

    static inline int AllocationCount = 0;
    static inline int ElementsAllocated = 0;

    TestAllocator() = default;

    template<typename U>
    TestAllocator(const TestAllocator<U>& other) {
    }

    void deallocate(T* p, std::size_t n) {
        if constexpr (std::is_same_v<T, SomeObj2>) {
            ++TestAllocator<SomeObj2>::DeallocationCount;
            TestAllocator<SomeObj2>::ElementsDeallocated += n;
        } else {
            ++TestAllocator<NodeTag>::DeallocationCount;
            TestAllocator<NodeTag>::ElementsDeallocated += n;
        }
        delete[] reinterpret_cast<char*>(p);
    }

    pointer allocate(size_type sz) {
        if constexpr (std::is_same_v<T, SomeObj2>) {
            ++TestAllocator<SomeObj2>::AllocationCount;
            TestAllocator<SomeObj2>::ElementsAllocated += sz;
        } else {
            ++TestAllocator<NodeTag>::AllocationCount;
            TestAllocator<NodeTag>::ElementsAllocated += sz;
        }
        return reinterpret_cast<pointer>(new char[sz * sizeof(value_type)]);
    }

};

class ExceptionSafetyTest : public testing::Test {
public:
    void SetUp() override {
        SomeObj2::CopiesCount = 0;
        SomeObj2::DestructorCalled = 0;

        TestAllocator<SomeObj2>::AllocationCount = 0;
        TestAllocator<SomeObj2>::ElementsAllocated = 0;

        TestAllocator<NodeTag>::AllocationCount = 0;
        TestAllocator<NodeTag>::ElementsAllocated = 0;

        TestAllocator<SomeObj2>::DeallocationCount = 0;
        TestAllocator<SomeObj2>::ElementsDeallocated = 0;

        TestAllocator<NodeTag>::DeallocationCount = 0;
        TestAllocator<NodeTag>::ElementsDeallocated = 0;
    }

};

TEST_F(ExceptionSafetyTest, failesAtConstruct) {
    std::list<SomeObj2> std_list;
    for (int i = 0; i < 5; ++i) {
        std_list.push_back(SomeObj2{});
    }
    SomeObj2::DestructorCalled = 0;

    TestAllocator<SomeObj2> allocator;
    using unrolled_list_type = unrolled_list<SomeObj2, 16, TestAllocator<SomeObj2>>;

    ASSERT_ANY_THROW(
        unrolled_list_type ul(std_list.begin(), std_list.end(), allocator)
    );

    ASSERT_EQ(SomeObj2::DestructorCalled, 2);

    ASSERT_EQ(TestAllocator<NodeTag>::AllocationCount, TestAllocator<NodeTag>::DeallocationCount);
    ASSERT_EQ(TestAllocator<NodeTag>::ElementsAllocated, TestAllocator<NodeTag>::ElementsDeallocated);
}

TEST_F(ExceptionSafetyTest, failesAtPushFront) {
    unrolled_list<BadOrGood> unrolled_list;

    unrolled_list.push_front(Good{.Name = "first"});
    unrolled_list.push_front(Good{.Name = "second"});
    ASSERT_ANY_THROW(unrolled_list.push_front(Bad{}));

    ASSERT_EQ(unrolled_list.size(), 2);
    ASSERT_EQ(unrolled_list.begin()->Name, std::string("second"));
    ASSERT_EQ((++unrolled_list.begin())->Name, std::string("first"));
}

TEST_F(ExceptionSafetyTest, noFailsAtPushFront) {
    unrolled_list<BadOrGood> unrolled_list;

    unrolled_list.push_front(Good{.Name = "first"});
    unrolled_list.push_front(Good{.Name = "second"});
    unrolled_list.push_front(Good{.Name = "first"});
    unrolled_list.push_front(Good{.Name = "second"});
    unrolled_list.push_front(Good{.Name = "first"});
    unrolled_list.push_front(Good{.Name = "second"});

    ASSERT_EQ(unrolled_list.size(), 6);
    ASSERT_EQ(unrolled_list.begin()->Name, std::string("second"));
    ASSERT_EQ((++unrolled_list.begin())->Name, std::string("first"));
}

TEST_F(ExceptionSafetyTest, failesAtPushBack) {
    unrolled_list<BadOrGood> unrolled_list;

    unrolled_list.push_back(Good{.Name = "first"});
    unrolled_list.push_back(Good{.Name = "second"});
    ASSERT_ANY_THROW(unrolled_list.push_back(Bad{}));

    ASSERT_EQ(unrolled_list.size(), 2);
    ASSERT_EQ(unrolled_list.begin()->Name, std::string("first"));
    ASSERT_EQ((++unrolled_list.begin())->Name, std::string("second"));
}
