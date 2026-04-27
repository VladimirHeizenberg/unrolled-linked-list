#include <iostream>

#include <unrolled_list.h>
#include <vector>
#include <list>

struct Obj {
    Obj() = default;
    Obj(const Obj& other) = delete;
    Obj(Obj&& other) noexcept = default;
};

int main(int argc, char** argv) {
    unrolled_list<int> unrolled_lst = {1, 2, 3, 4, 5, 6, 7, 8};
    std::list<int> std_list = {1, 2, 3, 4, 5, 6, 7, 8};

    auto it1 = unrolled_lst.begin();
    auto it2 = std_list.begin();

    std::advance(it1, unrolled_lst.size() / 2);
    std::advance(it2, std_list.size() / 2);
    unrolled_lst.insert(it1, {2, 3, 4, 5, 6});
    std_list.insert(it2, {2, 3, 4, 5, 6});
}
