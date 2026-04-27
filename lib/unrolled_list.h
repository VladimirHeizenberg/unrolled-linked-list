#include <memory>
#include <type_traits>
#include <algorithm>

struct BaseNode {
    BaseNode() noexcept = default;
    virtual ~BaseNode() noexcept = default;
    BaseNode* prev = nullptr;
    BaseNode* next = nullptr;
    size_t size = 0;
};

template<typename T, size_t NodeMaxSize = 10>
struct Node : BaseNode {
    Node() noexcept = default;

    T& operator[](size_t index) noexcept {
        return reinterpret_cast<T*>(array)[index];
    }

    T* pointer(size_t index) noexcept {
        return reinterpret_cast<T*>(array) + index;
    }

    alignas(alignof(T)) std::byte array[sizeof(T) * NodeMaxSize];
};

template<class UnrolledList>
class ConstUnrolledListIterator {
public:
    using value_type           = typename UnrolledList::value_type;
    using pointer              = typename UnrolledList::const_pointer;
    using reference            = const value_type&;

    using iterator_category    = std::bidirectional_iterator_tag;

    using BaseNodePtr          = typename UnrolledList::BaseNodePtr;
    using node_ptr             = typename UnrolledList::node_ptr;
    using difference_type      = typename UnrolledList::difference_type;

    ConstUnrolledListIterator(BaseNodePtr node, size_t index)
        : node_(node)
        , index_in_node_(index) {}

    virtual ~ConstUnrolledListIterator() = default;

    [[nodiscard]] pointer operator->() const noexcept {
        return dynamic_cast<node_ptr>(node_)->pointer(index_in_node_);
    }

    [[nodiscard]] reference operator*() const noexcept {
        return (*dynamic_cast<node_ptr>(node_))[index_in_node_];
    }

    node_ptr node() {
        return dynamic_cast<node_ptr>(node_);
    }

    size_t index() {
        return index_in_node_;
    }

    ConstUnrolledListIterator& operator++() noexcept {
        if (index_in_node_ == node_->size - 1) {
            node_ = node_->next;
            index_in_node_ = 0;
        }
        else {
            ++index_in_node_;
        }
        return *this;
    }

    ConstUnrolledListIterator operator++(int) noexcept {
        ConstUnrolledListIterator copy(*this);
        ++(*this);
        return copy;
    }

    ConstUnrolledListIterator& operator--() noexcept {
        if (index_in_node_ == 0) {
            node_ = node_->prev;
            index_in_node_ = node_->size - 1;
        }
        else if (index_in_node_ != 0) {
            --index_in_node_;
        }
        return *this;
    }

    ConstUnrolledListIterator operator--(int) noexcept {
        ConstUnrolledListIterator copy(*this);
        --(*this);
        return copy;
    }

    bool operator==(const ConstUnrolledListIterator& other) const noexcept {
        return (
            node_ == other.node_ &&
            index_in_node_ == other.index_in_node_
            );
    }

    bool operator!=(const ConstUnrolledListIterator& other) const noexcept {
        return !(*this == other);
    }

protected:
    BaseNodePtr node_;
    size_t index_in_node_;
};

template<class UnrolledList>
class UnrolledListIterator : public ConstUnrolledListIterator<UnrolledList> {
public:
    using base                  = ConstUnrolledListIterator<UnrolledList>;
    using value_type            = typename UnrolledList::value_type;
    using pointer               = typename UnrolledList::pointer;
    using reference             = value_type&;

    using iterator_category     = std::bidirectional_iterator_tag;

    using BaseNodePtr           = typename UnrolledList::BaseNodePtr;
    using node_ptr              = typename UnrolledList::node_ptr;

    UnrolledListIterator(BaseNodePtr node, size_t index)
        : base(node, index) {}

    [[nodiscard]] pointer operator->() const noexcept {
        return dynamic_cast<node_ptr>(this->node_)->pointer(this->index_in_node_);
    }

    [[nodiscard]] reference operator*() const noexcept {
        return (*dynamic_cast<node_ptr>(this->node_))[this->index_in_node_];
    }

    UnrolledListIterator& operator++() noexcept {
        base::operator++();
        return *this;
    }

    UnrolledListIterator operator++(int) noexcept {
        UnrolledListIterator copy(*this);
        ++(*this);
        return copy;
    }

    UnrolledListIterator& operator--() noexcept {
        base::operator--();
        return *this;
    }

    UnrolledListIterator operator--(int) noexcept {
        UnrolledListIterator copy(*this);
        --(*this);
        return copy;
    }

    bool operator==(const UnrolledListIterator& other) const noexcept {
        return (
            this->node_ == other.node_ &&
            this->index_in_node_ == other.index_in_node_
            );
    }

    bool operator!=(const UnrolledListIterator& other) const noexcept {
        return !(*this == other);
    }
};

template<typename T, size_t NodeMaxSize = 10, typename Allocator = std::allocator<T>>
class unrolled_list {
public:
    using unrolled_list_type        = unrolled_list<T, NodeMaxSize, Allocator>;

    using value_type                = T;
    using pointer                   = T*;
    using const_pointer             = const T*;
    using reference                 = T&;
    using const_reference           = const T&;
    using size_type                 = size_t;
    using node_ptr                  = Node<value_type, NodeMaxSize>*;
    using BaseNodePtr               = BaseNode*;

    using alloc_traits              = std::allocator_traits<Allocator>;
    using allocator_type            = Allocator;
    using node_allocator            = alloc_traits::template rebind_alloc<Node<value_type, NodeMaxSize>>;
    using alloc_traits_node         = std::allocator_traits<node_allocator>;

    using iterator                  = UnrolledListIterator<unrolled_list_type>;
    using const_iterator            = ConstUnrolledListIterator<unrolled_list_type>;
    using reverse_iterator          = std::reverse_iterator<iterator>;
    using const_reverse_iterator    = std::reverse_iterator<const_iterator>;
    using difference_type           = int;

    static_assert(std::is_same_v<value_type, typename Allocator::value_type>);
    static_assert(NodeMaxSize != 0);

    unrolled_list()
        : unrolled_list(allocator_type()) {}

    unrolled_list(const allocator_type& allocator)
        : fake_node_itself(BaseNode())
        , fake_node_(&fake_node_itself)
        , size_(0)
        , nodes_cnt_(0)
        , T_allocator_(allocator)
        , node_allocator_(allocator) {}

    unrolled_list(const unrolled_list& other)
        : fake_node_itself(BaseNode())
        , fake_node_(&fake_node_itself)
        , size_(0)
        , nodes_cnt_(0)
        , T_allocator_(other.T_allocator_)
        , node_allocator_(other.node_allocator_) {
        ConstructFromRange(other.begin(), other.end());
    }

    unrolled_list(const unrolled_list& other, const allocator_type& allocator)
        : unrolled_list(allocator) {
        ConstructFromRange(other.begin(), other.end());
    }

    unrolled_list(unrolled_list&& other)
        : fake_node_itself(BaseNode())
        , fake_node_(&fake_node_itself)
        , size_(other.size_)
        , nodes_cnt_(other.nodes_cnt_)
        , T_allocator_(std::move(other.T_allocator_))
        , node_allocator_(std::move(other.node_allocator_)) {
        fake_node_->next = other.fake_node_->next;
        fake_node_->prev = other.fake_node_->prev;
        fake_node_->next->prev = fake_node_;
        fake_node_->prev->next = fake_node_;
        other.fake_node_->prev = other.fake_node_->next = other.fake_node_;
        other.size_ = other.nodes_cnt_ = 0;
    }

    unrolled_list(unrolled_list&& other, const allocator_type& allocator)
        : unrolled_list(allocator) {
        if (allocator == other.T_allocator_) {
            size_ = other.size_;
            nodes_cnt_ = other.nodes_cnt_;
            fake_node_->next = other.fake_node_->next;
            fake_node_->prev = other.fake_node_->prev;
            other.fake_node_->prev = other.fake_node_->next = other.fake_node_;
            T_allocator_ = std::move(other.T_allocator_);
            node_allocator_ = std::move(other.node_allocator_);
            other.size_ = other.nodes_cnt_ = 0;
        }
        else {
            ConstructFromRange(other.begin(), other.end());
        }
    }

    template<class Iter>
    unrolled_list(Iter it1, Iter it2)
        : unrolled_list() {
        ConstructFromRange(it1, it2);
    }

    template<class Iter>
    unrolled_list(Iter it1, Iter it2, const allocator_type& allocator)
        : unrolled_list(allocator) {
        ConstructFromRange(it1, it2);
    }

    unrolled_list(const_reference value, size_type n)
        : unrolled_list() {
        ConstructWithNElements(value, n);
    }

    unrolled_list(const_reference value, size_type n, const allocator_type& allocator)
        : unrolled_list(allocator) {
        ConstructWithNElements(value, n);
    }

    unrolled_list(std::initializer_list<value_type> il)
        : unrolled_list() {
        ConstructFromRange(il.begin(), il.end());
    }

    unrolled_list(std::initializer_list<value_type> il, const allocator_type& allocator)
        : unrolled_list(allocator) {
        ConstructFromRange(il.begin(), il.end());
    }

    unrolled_list& operator=(const unrolled_list& other) {
        if (this == std::addressof(other)) {
            return *this;
        }
        if constexpr (std::allocator_traits<allocator_type>::propagate_on_container_copy_assignment::value) {
            T_allocator_ = other.T_allocator_;
            node_allocator_ = other.node_allocator_;
        }
        clear();
        ConstructFromRange(other.begin(), other.end());
        return *this;
    }

    unrolled_list& operator=(std::initializer_list<T> il) {
        clear();
        ConstructFromRange(il.begin(), il.end());
        return *this;
    }

    unrolled_list& operator=(unrolled_list&& other) noexcept(std::allocator_traits<Allocator>::is_always_equal::value) {
        if (this == std::addressof(other)) {
            return *this;
        }
        clear();
        if constexpr (std::allocator_traits<allocator_type>::propagate_on_container_move_assignment::value) {
            T_allocator_ = std::move(other.T_allocator_);
            node_allocator_ = std::move(other.node_allocator_);
            fake_node_->next = other.fake_node_->next;
            fake_node_->prev = other.fake_node_->prev;
            other.fake_node_->prev = other.fake_node_->next = other.fake_node_;
            size_ = other.size_;
            nodes_cnt_ = other.nodes_cnt_;
            other.size_ = 0;
            other.nodes_cnt_ = 0;
        }
        else {
            ConstructFromRange(other.begin(), other.end());
        }
        return *this;
    }

    ~unrolled_list() {
        clear();
    }

    // iterators

    [[nodiscard]] iterator begin() noexcept {
        return iterator(fake_node_->next, 0);
    }

    [[nodiscard]] iterator end() noexcept {
        return iterator(fake_node_, 0);
    }

    [[nodiscard]] const_iterator begin() const noexcept {
        return const_iterator(fake_node_->next, 0);
    }

    [[nodiscard]] const_iterator end() const noexcept {
        return const_iterator(fake_node_, 0);
    }

    [[nodiscard]] const_iterator cbegin() const noexcept {
        return const_iterator(fake_node_->next, 0);
    }

    [[nodiscard]] const_iterator cend() const noexcept {
        return const_iterator(fake_node_, 0);
    }

    [[nodiscard]] reverse_iterator rbegin() noexcept {
        return reverse_iterator(end());
    }

    [[nodiscard]] reverse_iterator rend() noexcept {
        return reverse_iterator(begin());
    };

    [[nodiscard]]const_reverse_iterator rbegin() const noexcept {
        return const_reverse_iterator(end());
    }

    [[nodiscard]] const_reverse_iterator rend() const noexcept {
        return const_reverse_iterator(begin());
    }

    [[nodiscard]] const_reverse_iterator crbegin() const noexcept {
        return const_reverse_iterator(end());
    }

    [[nodiscard]] const_reverse_iterator crend() const noexcept {
        return const_reverse_iterator(begin());
    }


    // size

    [[nodiscard]] size_type size() noexcept {
        return size_;
    }

    [[nodiscard]] size_type max_size() noexcept {
        return nodes_cnt_ * NodeMaxSize;
    }

    [[nodiscard]] bool empty() noexcept {
        // return begin() == end();
        return size_ == 0;
    }

    // access to elements and allocs

    [[nodiscard]] reference front() {
        return *begin();
    }

    [[nodiscard]] const_reference front() const {
        return *begin();
    }

    [[nodiscard]] reference back() {
        auto tmp = end();
        --tmp;
        return *tmp;
    }

    [[nodiscard]] const_reference back() const {
        auto tmp = end();
        --tmp;
        return *tmp;
    }

    [[nodiscard]] allocator_type get_allocator() const noexcept {
        return T_allocator_;
    }

    // modifiers
    void swap(unrolled_list& other) noexcept {
        if (this != std::addressof(other)) {
            if constexpr (std::allocator_traits<allocator_type>::propagate_on_container_swap::value) {
                std::swap(T_allocator_, other.T_allocator_);
                std::swap(node_allocator_, other.node_allocator_);
                std::swap(fake_node_, other.fake_node_);
                std::swap(size_, other.size_);
                std::swap(nodes_cnt_, other.nodes_cnt_);
            }
            else {
                static_assert("not swappable");
            }
        }
    }

    void push_back(const_reference value) {
        if (size_ == 0) {
            node_ptr new_node = CreateNodeWithElem(value);
            fake_node_->next = fake_node_->prev = new_node;
            new_node->prev = fake_node_;
            new_node->next = fake_node_;
            ++nodes_cnt_;
            ++size_;
            return;
        }
        node_ptr tail = dynamic_cast<node_ptr>(fake_node_->prev);
        if (tail->size == NodeMaxSize) {
            node_ptr new_node = CreateNodeWithElem(value);
            new_node->prev = tail;
            new_node->next = fake_node_;
            tail->next = new_node;
            fake_node_->prev = new_node;
            ++nodes_cnt_;
        }
        else {
            alloc_traits::construct(
                T_allocator_,
                reinterpret_cast<pointer>(tail->array) + tail->size,
                value
            );
            ++(tail->size);
        }
        ++size_;
    }

    void push_front(const_reference value) {
        if (size_ == 0) {
            node_ptr new_node = CreateNodeWithElem(value);
            fake_node_->prev = fake_node_->next = new_node;
            new_node->prev = new_node->next = fake_node_;
            ++size_;
            return;
        }
        node_ptr head = dynamic_cast<node_ptr>(fake_node_->next);
        if (head->size == NodeMaxSize) {
            node_ptr new_node = CreateNodeWithElem(value);
            head->prev = new_node;
            new_node->next = head;
            new_node->prev = fake_node_;
            fake_node_->next = new_node;
            ++size_;
            return;
        }
        if constexpr (std::is_nothrow_move_constructible_v<value_type>) {
            for (int i = head->size; i > 0; --i) {
                alloc_traits::construct(T_allocator_, head->pointer(i), std::move((*head)[i - 1]));
                alloc_traits::destroy(T_allocator_, head->pointer(i - 1));
            }
            try {
                alloc_traits::construct(T_allocator_, head->pointer(0), value);
            }
            catch (std::exception&) {
                for (int i = 0; i < head->size; ++i) {
                    alloc_traits::construct(T_allocator_, head->pointer(i), std::move((*head)[i + 1]));
                    alloc_traits::destroy(T_allocator_, head->pointer(i + 1));
                }
                throw;
            }
            ++size_;
            ++head->size;
        }
        else {
            node_ptr new_node = CopyNodeAndInsertValue(head, 0, value);
            new_node->prev = head->prev;
            new_node->next = head->next;
            head->next->prev = new_node;
            head->prev->next = new_node;
            ++size_;
            ClearNode(head);
        }
    }

    iterator insert(const_iterator it, const_reference value) {
        node_ptr node = dynamic_cast<node_ptr>(it.node());
        size_type index = it.index();
        if (it == begin()) {
            push_front(value);
            return begin();
        }
        else if (it == end()) {
            push_back(value);
            return --end();
        }
        if (node->size == NodeMaxSize) {
            node_ptr new_node_1 = CreateNode();
            node_ptr new_node_2 = nullptr;
            try {
                new_node_2 = CreateNode();
            }
            catch (std::exception&) {
                ClearNode(new_node_1);
                throw;
            }
            if constexpr (std::is_nothrow_move_constructible_v<value_type>) {
                for (size_type i = 0; i < NodeMaxSize / 2; ++i) {
                    alloc_traits::construct(T_allocator_, new_node_1->pointer(i), std::move((*node)[i]));
                }
                size_type index_tmp = 0;
                for (size_type i = NodeMaxSize / 2; i < NodeMaxSize; ++i) {
                    alloc_traits::construct(T_allocator_, new_node_2->pointer(index_tmp++), std::move((*node)[i]));
                }
                auto prev = node->prev;
                auto next = node->next;
                prev->next = new_node_1;
                new_node_1->prev = prev;
                new_node_1->next = new_node_2;
                new_node_2->prev = new_node_1;
                new_node_2->next = next;
                next->prev = new_node_2;
                ++nodes_cnt_;
                new_node_1->size = NodeMaxSize / 2;
                new_node_2->size = NodeMaxSize - NodeMaxSize / 2;
                ClearNode(node);
                if (index < NodeMaxSize / 2) {
                    return insert(const_iterator(new_node_1, index), value);
                }
                else {
                    return insert(const_iterator(new_node_2, index - NodeMaxSize / 2), value);
                }
            }
            else {
                for (size_type i = 0; i < NodeMaxSize / 2; ++i) {
                    alloc_traits::construct(T_allocator_, new_node_1->pointer(i), (*node)[i]);
                }
                size_type index_tmp = 0;
                for (size_type i = NodeMaxSize / 2; i < NodeMaxSize; ++i) {
                    alloc_traits::construct(T_allocator_, new_node_2->pointer(index_tmp++), (*node)[i]);
                }
                auto prev = node->prev;
                auto next = node->next;
                prev->next = new_node_1;
                new_node_1->prev = prev;
                new_node_1->next = new_node_2;
                new_node_2->prev = new_node_1;
                new_node_2->next = next;
                next->prev = new_node_2;
                ++nodes_cnt_;
                new_node_1->size = NodeMaxSize / 2;
                new_node_2->size = NodeMaxSize - NodeMaxSize / 2;
                ClearNode(node);
                if (index < NodeMaxSize / 2) {
                    return insert(const_iterator(new_node_1, index), value);
                }
                else {
                    return insert(const_iterator(new_node_2, index - NodeMaxSize / 2), value);
                }
            }
        }
        else {
            if constexpr (std::is_nothrow_move_constructible_v<value_type>) {
                for (size_type i = node->size; i > index; --i) {
                    alloc_traits::construct(T_allocator_, node->pointer(i), std::move((*node)[i - 1]));
                    alloc_traits::destroy(T_allocator_, node->pointer(i));
                }
                try {
                    alloc_traits::construct(T_allocator_, node->pointer(index), value);
                }
                catch (std::exception&) {
                    for (size_type i = index; i < node->size; ++i) {
                        alloc_traits::construct(T_allocator_, node->pointer(i), std::move((*node)[i + 1]));
                        alloc_traits::destroy(T_allocator_, node->pointer(i + 1));
                    }
                    throw;
                }
                ++node->size;
                ++size_;
                return iterator(node, index);
            }
            else {
                node_ptr copy_node = CopyNodeAndInsertValue(node, index, value);
                copy_node->prev = node->prev;
                copy_node->next = node->next;
                node->prev->next = copy_node;
                node->next->prev = copy_node;
                ++size_;
                ClearNode(node);
                return iterator(copy_node, index);
            }
        }
    }

    template<class InputIter>
    iterator insert(const_iterator it, InputIter i, InputIter j) {
        if (i == j) {
            return iterator(it.node(), it.index());
        }
        size_type cnt = 0;
        while (i != j) {
            it = insert(it, *i);
            ++it;
            ++i;
            ++cnt;
        }
        for (size_type i = 0; i < cnt; ++i) {
            --it;
        }
        return iterator(it.node(), it.index());
    }

    iterator insert(const_iterator it, size_type n, const_reference value) {
        for (size_type i = 0; i < n; i++) {
            it = insert(it, value);
        }
        return iterator(it.node(), it.index());
    }

    iterator insert(const_iterator it, std::initializer_list<value_type> il) {
        return insert(it, il.begin(), il.end());
    }

    template<class InputIter> requires std::input_iterator<InputIter>
    void assign(InputIter i, InputIter j) {
        clear();
        ConstructFromRange(i, j);
    }

    void assign(std::initializer_list<value_type> il) {
        assign(il.begin(), il.end());
    }

    void assign(size_type n, const_reference t) {
        clear();
        ConstructWithNElements(t, n);
    }

    iterator erase(const_iterator it) noexcept(std::is_nothrow_move_constructible_v<value_type>) {
        if (size_ == 0) {
            return end();
        }
        if (it == --end()) {
            pop_back();
            return end();
        }
        node_ptr node = dynamic_cast<node_ptr>(it.node());
        auto prev = node->prev;
        auto next = node->next;
        size_type index = it.index();
        if (node->size == 1) {
            ClearNode(node);
            prev->next = next;
            next->prev = prev;
            --size_;
            --nodes_cnt_;
            return iterator(next, 0);
        }
        if constexpr (std::is_nothrow_move_constructible_v<value_type>) {
            alloc_traits::destroy(T_allocator_, node->pointer(index));
            for (size_type i = index; i < node->size - 1; ++i) {
                alloc_traits::construct(T_allocator_, node->pointer(i), std::move((*node)[i + 1]));
                alloc_traits::destroy(T_allocator_, node->pointer(i + 1));
            }
            --node->size;
            --size_;
            return iterator(node, index);
        }
        else {
            node_ptr copy_node = CreateNode();
            size_type last_constructed = -1;
            try {
                for (size_type i = 0; i < index; ++i) {
                    alloc_traits::construct(T_allocator_, copy_node->pointer(i), (*node)[i]);
                    last_constructed = i;
                }
            }
            catch (std::exception&) {
                for (size_type i = 0; i <= last_constructed; ++i) {
                    alloc_traits::destroy(T_allocator_, copy_node->pointer(i));
                }
                alloc_traits_node::destroy(node_allocator_, copy_node);
                alloc_traits_node::deallocate(node_allocator_, copy_node, 1);
                throw;
            }
            last_constructed = -1;
            try {
                for (size_type i = index; i < node->size - 1; ++i) {
                    alloc_traits::construct(T_allocator_, copy_node->pointer(i), (*node)[i + 1]);
                    last_constructed = i;
                }
            }
            catch (std::exception&) {
                for (size_type i = 0; i <= last_constructed; ++i) {
                    alloc_traits::destroy(T_allocator_, copy_node->pointer(i));
                }
                alloc_traits_node::destroy(node_allocator_, copy_node);
                alloc_traits_node::deallocate(node_allocator_, copy_node, 1);
                throw;
            }
            copy_node->next = next;
            copy_node->prev = prev;
            next->prev = copy_node;
            prev->next = copy_node;
            --size_;
            copy_node->size = node->size - 1;
            for (size_type i = 0; i <= node->size; ++i) {
                alloc_traits::destroy(T_allocator_, node->pointer(i));
            }
            alloc_traits_node::destroy(node_allocator_, node);
            alloc_traits_node::deallocate(node_allocator_, node, 1);
            return iterator(copy_node, index);
        }
    }

    iterator erase(const_iterator it1, const_iterator it2) noexcept(std::is_nothrow_move_constructible_v<value_type>) {
        while (it2 != it1) {
            --it2;
            it2 = erase(it2);
        }
        return iterator(it2.node(), it2.index());
    }

    void pop_back() noexcept {
        if (size_ == 0) {
            return;
        }
        node_ptr tail = dynamic_cast<node_ptr>(fake_node_->prev);
        alloc_traits::destroy(T_allocator_, tail->pointer(tail->size - 1));
        --(tail->size);
        if (tail->size == 0) {
            BaseNodePtr prev_to_tail = tail->prev;
            prev_to_tail->next = fake_node_;
            fake_node_->prev = prev_to_tail;
            alloc_traits_node::destroy(node_allocator_, tail);
            alloc_traits_node::deallocate(node_allocator_, tail, 1);
        }
        --size_;
    }

    void pop_front() noexcept(std::is_nothrow_move_constructible_v<value_type>) {
        erase(begin());
    }

    void clear() noexcept {
        while (size_ > 0) {
            pop_back();
        }
    }

    bool operator==(const unrolled_list& other) {
        return std::equal(
            begin(), end(),
            other.begin(), other.end()
        );
    }

    bool operator!=(const unrolled_list& other) {
        return !(*this == other);
    }

private:

    void ClearNode(node_ptr& node) {
        for (size_type i = 0; i < node->size; ++i) {
            alloc_traits::destroy(T_allocator_, node->pointer(i));
        }
        alloc_traits_node::destroy(node_allocator_, node);
        alloc_traits_node::deallocate(node_allocator_, node, 1);
        node = nullptr;
    }

    // pred: node->size != NodeMaxSize
    node_ptr CopyNodeAndInsertValue(node_ptr node, size_type index, const_reference value) {
        node_ptr copy_node = CreateNode();
        size_type last_constructed = -1;
        try {
            for (size_type i = node->size; i > index; --i) {
                alloc_traits::construct(T_allocator_, copy_node->pointer(i), (*node)[i - 1]);
                last_constructed = i;
            }
        }
        catch (std::exception&) {
            for (size_type i = node->size; i >= last_constructed; --i) {
                alloc_traits::destroy(T_allocator_, copy_node->pointer(i));
            }
            alloc_traits_node::destroy(node_allocator_, copy_node);
            alloc_traits_node::deallocate(node_allocator_, copy_node, 1);
            throw;
        }
        last_constructed = -1;
        try {
            for (size_type i = 0; i < index; ++i) {
                alloc_traits::construct(T_allocator_, copy_node->pointer(i), (*node)[i]);
                last_constructed = i;
            }
            alloc_traits::construct(T_allocator_, node->pointer(index), value);
        }
        catch (std::exception&) {
            for (size_type i = node->size; i > index; --i) {
                alloc_traits::destroy(T_allocator_, copy_node->pointer(i));
            }
            for (size_type i = 0; i <= last_constructed; ++i) {
                alloc_traits::destroy(T_allocator_, copy_node->pointer(i));
            }
            alloc_traits_node::destroy(node_allocator_, copy_node);
            alloc_traits_node::deallocate(node_allocator_, copy_node, 1);
            throw;
        }
        copy_node->size = node->size + 1;
        return copy_node;
    }

    node_ptr CreateNode() {
        node_ptr new_node = alloc_traits_node::allocate(node_allocator_, 1);
        alloc_traits_node::construct(node_allocator_, new_node); // no try-catch cause node constructor is noexcept
        return new_node;
    }

    node_ptr CreateNodeWithElem(const_reference elem) {
        node_ptr new_node = CreateNode();
        try {
            alloc_traits::construct(T_allocator_, new_node->pointer(0), elem);
        }
        catch (std::exception&) {
            alloc_traits_node::destroy(node_allocator_, new_node);
            alloc_traits_node::deallocate(node_allocator_, new_node, 1);
            new_node = nullptr;
            throw;
        }
        new_node->size = 1;
        return new_node;
    }

    template<class Iter>
    void ConstructFromRange(Iter it1, Iter it2) {
        try {
            while (it1 != it2) {
                push_back(*it1);
                ++it1;
            }
        }
        catch (std::exception&) {
            if (size_ > 0) {
                clear();
            }
            throw;
        }
    }

    void ConstructWithNElements(const_reference value, size_type n) {
        try {
            for (size_type i = 0; i < n; i++) {
                push_back(value);
            }
        }
        catch (std::exception&) {
            if (size_ > 0) {
                clear();
            }
            throw;
        }
    }

    BaseNode fake_node_itself;
    BaseNode* fake_node_;
    size_type size_;
    size_type nodes_cnt_;
    allocator_type T_allocator_;
    node_allocator node_allocator_;
};
