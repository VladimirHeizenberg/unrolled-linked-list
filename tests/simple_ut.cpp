#include <unrolled_list.h>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <vector>
#include <list>
#include <cstdlib>


TEST(UnrolledLinkedList, pushBack) {
    std::list<int> std_list;
    unrolled_list<int> unrolled_list;

    for (int i = 0; i < 1000; ++i) {
        std_list.push_back(i);
        unrolled_list.push_back(i);
    }

    ASSERT_THAT(unrolled_list, ::testing::ElementsAreArray(std_list));
}

TEST(UnrolledLinkedList, pushFront) {
    std::list<int> std_list;
    unrolled_list<int> unrolled_list;

    for (int i = 0; i < 1000; ++i) {
        std_list.push_front(i);
        unrolled_list.push_front(i);
    }

    ASSERT_THAT(unrolled_list, ::testing::ElementsAreArray(std_list));
}

TEST(UnrolledLinkedList, pushMixed) {
    std::list<int> std_list;
    unrolled_list<int> unrolled_list;

    for (int i = 0; i < 1000; ++i) {
        if (i % 2 == 0) {
            std_list.push_front(i);
            unrolled_list.push_front(i);
        } else {
            std_list.push_back(i);
            unrolled_list.push_back(i);
        }
    }

    ASSERT_THAT(unrolled_list, ::testing::ElementsAreArray(std_list));
}

TEST(UnrolledLinkedList, insertAndPushMixed) {
    std::list<int> std_list;
    unrolled_list<int> unrolled_list;

    for (int i = 0; i < 1000; ++i) {
        if (i % 3 == 0) {
            std_list.push_front(i);
            unrolled_list.push_front(i);
        } else if (i % 3 == 1) {
            std_list.push_back(i);
            unrolled_list.push_back(i);
        } else {
            auto std_it = std_list.begin();
            auto unrolled_it = unrolled_list.begin();
            std::advance(std_it, std_list.size() / 2);
            std::advance(unrolled_it, std_list.size() / 2);
            std_list.insert(std_it, i);
            unrolled_list.insert(unrolled_it, i);
        }
    }

    ASSERT_THAT(unrolled_list, ::testing::ElementsAreArray(std_list));
}

TEST(UnrolledLinkedList, popFrontBack) {
    std::list<int> std_list;
    unrolled_list<int> unrolled_list;

    for (int i = 0; i < 1000; ++i) {
        std_list.push_back(i);
        unrolled_list.push_back(i);
    }

    for (int i = 0; i < 500; ++i) {
        if (i % 2 == 0) {
            std_list.pop_back();
            unrolled_list.pop_back();
        } else {
            std_list.pop_front();
            unrolled_list.pop_front();
        }
    }

    ASSERT_THAT(unrolled_list, ::testing::ElementsAreArray(std_list));

    for (int i = 0; i < 500; ++i) {
        std_list.pop_back();
        unrolled_list.pop_back();
    }

    ASSERT_TRUE(unrolled_list.empty());
}


TEST(UnrolledLinkedList, allOperations) {
    std::list<int> std_list;
    unrolled_list<int> unrolled_list;
    std::srand(std::time({}));
    for (int i = 0; i < 10000; ++i) {
        if (i % 3 == 0) {
            std_list.push_front(i);
            unrolled_list.push_front(i);
        } else if (i % 3 == 1) {
            std_list.push_back(i);
            unrolled_list.push_back(i);
        } else {
            auto std_it = std_list.begin();
            auto unrolled_it = unrolled_list.begin();
            std::advance(std_it, std_list.size() / 2);
            std::advance(unrolled_it, std_list.size() / 2);
            std_list.insert(std_it, i);
            unrolled_list.insert(unrolled_it, i);
        }
    }

    for (int i = 0; i < 100; ++i) {
        if (i % 3 == 0) {
            std_list.pop_front();
            unrolled_list.pop_front();
        } else if (i % 3 == 1) {
            std_list.pop_back();
            unrolled_list.pop_back();
        } else {
            auto std_it = std_list.begin();
            auto unrolled_it = unrolled_list.begin();
            int x = std::rand() % std_list.size();
            std::advance(std_it, x);
            std::advance(unrolled_it, x);
            std_list.erase(std_it);
            unrolled_list.erase(unrolled_it);
        }
    }
    ASSERT_THAT(unrolled_list, ::testing::ElementsAreArray(std_list));
}

TEST(UnrolledLinkedList, initializerListsConstructorsAndAssigns) {
    unrolled_list<int> unrolled_lst = {1, 2, 3, 4, 5, 6, 7, 8};
    std::list<int> std_list = {1, 2, 3, 4, 5, 6, 7, 8};
    ASSERT_THAT(unrolled_lst, ::testing::ElementsAreArray(std_list));
    unrolled_list<int> unrolled_lst2({2, 3, 9, 5, 6, 6, 3, 0});
    std_list = {2, 3, 9, 5, 6, 6, 3, 0};
    ASSERT_THAT(unrolled_lst2, ::testing::ElementsAreArray(std_list));
}

TEST(UnrolledLinkedList, moveConstructorAndAssignments) {
    unrolled_list<int> unrolled_lst = {1, 2, 3, 4, 5, 6, 7, 8};
    std::list<int> std_list = {1, 2, 3, 4, 5, 6, 7, 8};
    unrolled_list<int> a(std::move(unrolled_lst));
    ASSERT_THAT(a, ::testing::ElementsAreArray(std_list));
    ASSERT_TRUE(unrolled_lst.empty());
    unrolled_list<int> b = std::move(a);
    ASSERT_THAT(b, ::testing::ElementsAreArray(std_list));
    ASSERT_TRUE(a.empty());
}

TEST(UnrolledLinkedList, differentItersOperations) {
    unrolled_list<int> unrolled_lst = {1, 2, 3, 4, 5, 6, 7, 8};
    std::list<int> std_list = {1, 2, 3, 4, 5, 6, 7, 8};

    auto it1 = unrolled_lst.begin();
    auto it2 = std_list.begin();

    std::advance(it1, unrolled_lst.size() / 2);
    std::advance(it2, std_list.size() / 2);
    it1 = unrolled_lst.insert(it1, {2, 3, 4, 5, 6});
    it2 = std_list.insert(it2, {2, 3, 4, 5, 6});
    ASSERT_THAT(unrolled_lst, ::testing::ElementsAreArray(std_list));

    auto it11 = it1;
    for (int i = 0; i < 5; i++)
        ++it11;
    auto it22 = it2;
    for (int i = 0; i < 5; i++)
        ++it22;
    it1 = unrolled_lst.erase(it1, it11);
    it2 = std_list.erase(it2, it22);

    ASSERT_THAT(unrolled_lst, ::testing::ElementsAreArray(std_list));

    unrolled_lst.assign(239, 566);
    std_list.assign(239, 566);
    ASSERT_THAT(unrolled_lst, ::testing::ElementsAreArray(std_list));
}
