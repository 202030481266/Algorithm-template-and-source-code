/*
最后修改:
20230928
测试环境:
gcc11.2,c++11
clang12.0,C++11
msvc14.2,C++14
*/
#ifndef __OY_PERSISTENTFHQTREAP__
#define __OY_PERSISTENTFHQTREAP__

#include <algorithm>
#include <cstdint>
#include <functional>
#include <numeric>
#include <random>

namespace OY {
    namespace PerFHQ {
        using size_type = uint32_t;
        using priority_type = uint32_t;
        std::mt19937 treap_rand;
        struct Ignore {};
        template <typename Tp, typename Compare = std::less<Tp>>
        struct BaseNodeWrapper {
            template <typename Node>
            struct type {
                using key_type = Tp;
                key_type m_key;
                static bool comp(const key_type &x, const key_type &y) { return Compare()(x, y); }
                void set(const key_type &key) { m_key = key; }
                const key_type &get() const { return m_key; }
            };
        };
        template <typename Tp, typename Operation, typename Compare = std::less<Tp>>
        struct CustomNodeWrapper {
            template <typename Node>
            struct type {
                using key_type = Tp;
                static Operation s_op;
                static bool comp(const key_type &x, const key_type &y) { return Compare()(x, y); }
                key_type m_key, m_info;
                void set(const key_type &key) { m_key = key; }
                const key_type &get() const { return m_key; }
                void pushup(Node *lchild, Node *rchild) { m_info = lchild->is_null() ? (rchild->is_null() ? get() : s_op(get(), rchild->m_info)) : (rchild->is_null() ? s_op(lchild->m_info, get()) : s_op(s_op(lchild->m_info, get()), rchild->m_info)); }
            };
        };
        template <typename Tp, typename Operation, typename Compare>
        template <typename Node>
        Operation CustomNodeWrapper<Tp, Operation, Compare>::type<Node>::s_op;
        template <typename Tp, typename ModifyType, typename Operation, typename Mapping, typename Composition, bool InitClearLazy>
        struct CustomLazyNodeWrapper {
            template <typename Node>
            struct type {
                using key_type = Tp;
                static Operation s_op;
                static Mapping s_map;
                static Composition s_com;
                static ModifyType s_default_modify;
                key_type m_key, m_info;
                ModifyType m_modify;
                void set(const key_type &key) {
                    m_key = key;
                    if constexpr (InitClearLazy) m_modify = s_default_modify;
                }
                const key_type &get() const { return m_key; }
                void modify(const ModifyType &modify) {
                    m_key = s_map(modify, m_key, 1);
                    m_info = s_map(modify, m_info, ((Node *)this)->m_size);
                    m_modify = s_com(modify, m_modify);
                }
                void pushup(Node *lchild, Node *rchild) { m_info = lchild->is_null() ? (rchild->is_null() ? get() : s_op(get(), rchild->m_info)) : (rchild->is_null() ? s_op(lchild->m_info, get()) : s_op(s_op(lchild->m_info, get()), rchild->m_info)); }
                void pushdown(Node *lchild, Node *rchild) {
                    if (!lchild->is_null()) lchild->modify(m_modify);
                    if (!rchild->is_null()) rchild->modify(m_modify);
                    m_modify = s_default_modify;
                };
            };
        };
        template <typename Tp, typename ModifyType, typename Operation, typename Mapping, typename Composition, bool InitClearLazy>
        template <typename Node>
        Operation CustomLazyNodeWrapper<Tp, ModifyType, Operation, Mapping, Composition, InitClearLazy>::type<Node>::s_op;
        template <typename Tp, typename ModifyType, typename Operation, typename Mapping, typename Composition, bool InitClearLazy>
        template <typename Node>
        Mapping CustomLazyNodeWrapper<Tp, ModifyType, Operation, Mapping, Composition, InitClearLazy>::type<Node>::s_map;
        template <typename Tp, typename ModifyType, typename Operation, typename Mapping, typename Composition, bool InitClearLazy>
        template <typename Node>
        Composition CustomLazyNodeWrapper<Tp, ModifyType, Operation, Mapping, Composition, InitClearLazy>::type<Node>::s_com;
        template <typename Tp, typename ModifyType, typename Operation, typename Mapping, typename Composition, bool InitClearLazy>
        template <typename Node>
        ModifyType CustomLazyNodeWrapper<Tp, ModifyType, Operation, Mapping, Composition, InitClearLazy>::type<Node>::s_default_modify;
#ifdef __cpp_lib_void_t
        template <typename... Tp>
        using void_t = std::void_t<Tp...>;
#else
        template <typename... Tp>
        struct make_void {
            using type = void;
        };
        template <typename... Tp>
        using void_t = typename make_void<Tp...>::type;
#endif
        template <typename Tp, typename NodePtr, typename = void>
        struct Has_pushup : std::false_type {};
        template <typename Tp, typename NodePtr>
        struct Has_pushup<Tp, NodePtr, void_t<decltype(std::declval<Tp>().pushup(std::declval<NodePtr>(), std::declval<NodePtr>()))>> : std::true_type {};
        template <typename Tp, typename NodePtr, typename = void>
        struct Has_pushdown : std::false_type {};
        template <typename Tp, typename NodePtr>
        struct Has_pushdown<Tp, NodePtr, void_t<decltype(std::declval<Tp>().pushdown(std::declval<NodePtr>(), std::declval<NodePtr>()))>> : std::true_type {};
        template <typename Tp, typename Fp, typename = void>
        struct Has_comp : std::false_type {};
        template <typename Tp, typename Fp>
        struct Has_comp<Tp, Fp, void_t<decltype(Tp::comp(std::declval<Fp>(), std::declval<Fp>()))>> : std::true_type {};
        template <typename Ostream, typename Tp, typename = void>
        struct Has_ostream : std::false_type {};
        template <typename Ostream, typename Tp>
        struct Has_ostream<Ostream, Tp, void_t<decltype(std::declval<Ostream>() << std::declval<Tp>())>> : std::true_type {};
        template <template <typename> typename NodeWrapper, bool Lock = false, size_type MAX_NODE = 1 << 20>
        struct Multiset {
            struct node : NodeWrapper<node> {
                priority_type m_prior;
                size_type m_size, m_lchild, m_rchild;
                bool is_null() const { return this == s_buffer; }
                node *lchild() const { return s_buffer + m_lchild; }
                node *rchild() const { return s_buffer + m_rchild; }
                template <typename Ostream>
                friend Ostream &operator<<(Ostream &out, const node &x) {
                    if constexpr (Has_ostream<Ostream &, NodeWrapper<node>>::value)
                        return out << NodeWrapper<node>(x);
                    else
                        return out << x.get();
                }
            };
            using key_type = typename node::key_type;
            using tree_type = Multiset<NodeWrapper, Lock, MAX_NODE>;
            static node s_buffer[MAX_NODE];
            static bool s_lock;
            struct ValueLessJudger {
                const key_type &m_key;
                ValueLessJudger(const key_type &key) : m_key(key) {}
                bool operator()(size_type x) const { return _comp(m_key, s_buffer[x].get()); }
            };
            struct ValueLessEqualJudger {
                const key_type &m_key;
                ValueLessEqualJudger(const key_type &key) : m_key(key) {}
                bool operator()(size_type x) const { return !_comp(s_buffer[x].get(), m_key); }
            };
            struct RankJudger {
                size_type m_rank;
                RankJudger(size_type k) : m_rank(k) {}
                bool operator()(size_type x) {
                    if (m_rank <= s_buffer[x].lchild()->m_size) return true;
                    m_rank -= s_buffer[x].lchild()->m_size + 1;
                    return false;
                }
            };
            template <typename SumType>
            struct SumJudger {
                SumType m_sum, m_ceil;
                SumJudger(SumType ceil) : m_sum(), m_ceil(ceil) {}
                bool try_lchild(node *x) {
                    SumType tmp = x->lchild()->m_info + m_sum;
                    if (tmp > m_ceil) return false;
                    return m_sum = tmp, true;
                }
                bool try_rchild(node *x) {
                    SumType tmp = m_sum + x->rchild()->m_info;
                    if (tmp > m_ceil) return false;
                    return m_sum = tmp, true;
                }
                bool try_mid(node *x) {
                    SumType tmp = m_sum + x->get();
                    if (tmp > m_ceil) return false;
                    return m_sum = tmp, true;
                }
            };
            static size_type s_use_count;
            size_type m_root{};
            template <typename Modify = Ignore>
            static size_type _create(const key_type &key, Modify &&modify = Modify()) {
                s_buffer[s_use_count].set(key);
                s_buffer[s_use_count].m_prior = treap_rand();
                s_buffer[s_use_count].m_size = 1;
                if constexpr (!std::is_same<typename std::decay<Modify>::type, Ignore>::value) modify(s_buffer + s_use_count);
                return s_use_count++;
            }
            static size_type _create(node *x) {
                s_buffer[s_use_count] = *x;
                return s_use_count++;
            }
            static bool _comp(const key_type &x, const key_type &y) {
                if constexpr (Has_comp<node, key_type>::value)
                    return node::comp(x, y);
                else
                    return x < y;
            }
            static void _update_size(size_type x) { s_buffer[x].m_size = s_buffer[x].lchild()->m_size + s_buffer[x].rchild()->m_size + 1; }
            static void _pushdown(size_type x) {
                if constexpr (!Lock) {
                    if (s_buffer[x].m_lchild) s_buffer[x].m_lchild = _create(s_buffer[x].lchild());
                    if (s_buffer[x].m_rchild) s_buffer[x].m_rchild = _create(s_buffer[x].rchild());
                } else {
                    if (!s_lock) {
                        if (s_buffer[x].m_lchild) s_buffer[x].m_lchild = _create(s_buffer[x].lchild());
                        if (s_buffer[x].m_rchild) s_buffer[x].m_rchild = _create(s_buffer[x].rchild());
                    }
                }
                if constexpr (Has_pushdown<node, node *>::value) s_buffer[x].pushdown(s_buffer[x].lchild(), s_buffer[x].rchild());
            }
            static void _pushdown_l(size_type x) {
                if constexpr (Has_pushdown<node, node *>::value) return _pushdown_if_lazy(x);
                if constexpr (!Lock) {
                    if (s_buffer[x].m_lchild) s_buffer[x].m_lchild = _create(s_buffer[x].lchild());
                } else if (!s_lock && s_buffer[x].m_lchild)
                    s_buffer[x].m_lchild = _create(s_buffer[x].lchild());
            }
            static void _pushdown_r(size_type x) {
                if constexpr (Has_pushdown<node, node *>::value) return _pushdown_if_lazy(x);
                if constexpr (!Lock) {
                    if (s_buffer[x].m_rchild) s_buffer[x].m_rchild = _create(s_buffer[x].rchild());
                } else if (!s_lock && s_buffer[x].m_rchild)
                    s_buffer[x].m_rchild = _create(s_buffer[x].rchild());
            }
            static void _pushdown_if_lazy(size_type x) {
                if constexpr (!Lock) {
                    if (s_buffer[x].m_lchild) s_buffer[x].m_lchild = _create(s_buffer[x].lchild());
                    if (s_buffer[x].m_rchild) s_buffer[x].m_rchild = _create(s_buffer[x].rchild());
                } else {
                    if (!s_lock) {
                        if (s_buffer[x].m_lchild) s_buffer[x].m_lchild = _create(s_buffer[x].lchild());
                        if (s_buffer[x].m_rchild) s_buffer[x].m_rchild = _create(s_buffer[x].rchild());
                    }
                }
                s_buffer[x].pushdown(s_buffer[x].lchild(), s_buffer[x].rchild());
            }
            static void _pushup(size_type x) {
                if constexpr (Has_pushup<node, node *>::value) s_buffer[x].pushup(s_buffer[x].lchild(), s_buffer[x].rchild());
            }
            template <typename Judger>
            static void _split(size_type rt, size_type *x, size_type *y, Judger &&judger) {
                if (!rt) return (void)(*x = *y = 0);
                if (judger(rt))
                    _pushdown_l(rt), *y = rt, _split(s_buffer[rt].m_lchild, x, &s_buffer[rt].m_lchild, judger);
                else
                    _pushdown_r(rt), *x = rt, _split(s_buffer[rt].m_rchild, &s_buffer[rt].m_rchild, y, judger);
                _update_size(rt);
                _pushup(rt);
            }
            static void _join(size_type *rt, size_type x, size_type y) {
                if (!x || !y) return (void)(*rt = x | y);
                if (s_buffer[x].m_prior > s_buffer[y].m_prior)
                    _pushdown_l(x), _join(&s_buffer[ *rt = x].m_rchild, s_buffer[x].m_rchild, y);
                else
                    _pushdown_r(y), _join(&s_buffer[ *rt = y].m_lchild, x, s_buffer[y].m_lchild);
                _update_size(*rt);
                _pushup(*rt);
            }
            template <typename Func>
            static void _merge(size_type *rt, size_type x, size_type y, Func &&func) {
                if (!x || !y) return (void)(*rt = x | y);
                if (s_buffer[x].m_prior < s_buffer[y].m_prior) std::swap(x, y);
                _pushdown(x);
                size_type a, b, c;
                _split(y, &a, &b, ValueLessEqualJudger(s_buffer[x].get()));
                _split(b, &b, &c, ValueLessJudger(s_buffer[x].get()));
                _merge(&s_buffer[x].m_lchild, s_buffer[x].m_lchild, a, func);
                _merge(&s_buffer[x].m_rchild, s_buffer[x].m_rchild, c, func);
                if constexpr (std::is_same<typename std::decay<Func>::type, Ignore>::value)
                    _join(&s_buffer[x].m_rchild, b, s_buffer[x].m_rchild);
                else if (b)
                    func(s_buffer + x, s_buffer + b);
                _update_size(*rt = x);
                _pushup(*rt);
            }
            template <typename Judger>
            static void _insert(size_type *rt, size_type x, Judger &&judger) {
                if (s_buffer[x].m_prior < s_buffer[*rt].m_prior) {
                    ++s_buffer[*rt].m_size;
                    if (judger(*rt))
                        _pushdown_l(*rt), _insert(&s_buffer[*rt].m_lchild, x, judger);
                    else
                        _pushdown_r(*rt), _insert(&s_buffer[*rt].m_rchild, x, judger);
                } else {
                    _pushdown(*rt);
                    _split(*rt, &s_buffer[x].m_lchild, &s_buffer[x].m_rchild, judger);
                    _update_size(*rt = x);
                }
                _pushup(*rt);
            }
            static bool _erase_by_key(size_type *rt, const key_type &key) {
                if (!*rt) return false;
                if (_comp(key, s_buffer[*rt].get())) {
                    _pushdown_l(*rt);
                    if (_erase_by_key(&s_buffer[*rt].m_lchild, key)) {
                        --s_buffer[*rt].m_size, _pushup(*rt);
                        return true;
                    }
                    return false;
                } else if (_comp(s_buffer[*rt].get(), key)) {
                    _pushdown_r(*rt);
                    if (_erase_by_key(&s_buffer[*rt].m_rchild, key)) {
                        --s_buffer[*rt].m_size, _pushup(*rt);
                        return true;
                    }
                    return false;
                } else {
                    _pushdown(*rt), _join(rt, s_buffer[*rt].m_lchild, s_buffer[*rt].m_rchild);
                    return true;
                }
            }
            static void _erase_by_rank(size_type *rt, size_type k) {
                if (k != s_buffer[*rt].lchild()->m_size) {
                    --s_buffer[*rt].m_size;
                    if (k <= s_buffer[*rt].lchild()->m_size)
                        _pushdown_l(*rt), _erase_by_rank(&s_buffer[*rt].m_lchild, k);
                    else
                        _pushdown_r(*rt), _erase_by_rank(&s_buffer[*rt].m_rchild, k - s_buffer[*rt].lchild()->m_size - 1);
                    _pushup(*rt);
                } else
                    _pushdown(*rt), _join(rt, s_buffer[*rt].m_lchild, s_buffer[*rt].m_rchild);
            }
            template <typename Modify>
            static bool _modify_by_key(size_type rt, const key_type &key, Modify &&modify) {
                bool res = false;
                if (!rt) return res;
                if (_comp(s_buffer[rt].get(), key))
                    _pushdown_r(rt), res = _modify_by_key(s_buffer[rt].m_rchild, key, modify);
                else if (_comp(key, s_buffer[rt].get()))
                    _pushdown_l(rt), res = _modify_by_key(s_buffer[rt].m_lchild, key, modify);
                else
                    _pushdown(rt), modify(s_buffer + rt), res = true;
                if (res) _pushup(rt);
                return res;
            }
            template <typename Modify>
            static void _modify_by_rank(size_type rt, size_type k, Modify &&modify) {
                if (k < s_buffer[rt].lchild()->m_size)
                    _pushdown_l(rt), _modify_by_rank(s_buffer[rt].m_lchild, k, modify);
                else if (k -= s_buffer[rt].lchild()->m_size)
                    _pushdown_r(rt), _modify_by_rank(s_buffer[rt].m_rchild, k - 1, modify);
                else
                    _pushdown(rt), modify(s_buffer + rt);
                _pushup(rt);
            }
            static size_type _kth(size_type rt, size_type k) {
                _pushdown(rt);
                if (k < s_buffer[rt].lchild()->m_size) return _kth(s_buffer[rt].m_lchild, k);
                if (k -= s_buffer[rt].lchild()->m_size) return _kth(s_buffer[rt].m_rchild, k - 1);
                return rt;
            }
            template <typename Judger>
            static size_type _min_left(size_type rt, Judger &&judge) {
                if (!rt) return 0;
                _pushdown(rt);
                if (!judge.try_rchild(s_buffer + rt)) return s_buffer[rt].lchild()->m_size + 1 + _min_left(s_buffer[rt].m_rchild, judge);
                if (!judge.try_mid(s_buffer + rt)) return s_buffer[rt].lchild()->m_size + 1;
                return _min_left(s_buffer[rt].m_lchild, judge);
            }
            template <typename Judger>
            static size_type _max_right(size_type rt, Judger &&judge) {
                if (!rt) return -1;
                _pushdown(rt);
                if (!judge.try_lchild(s_buffer + rt)) return _max_right(s_buffer[rt].m_lchild, judge);
                if (!judge.try_mid(s_buffer + rt)) return s_buffer[rt].lchild()->m_size - 1;
                return s_buffer[rt].lchild()->m_size + 1 + _max_right(s_buffer[rt].m_rchild, judge);
            }
            static size_type _rank(size_type rt, const key_type &key) {
                if (!rt) return 0;
                _pushdown(rt);
                if (!_comp(s_buffer[rt].get(), key)) return _rank(s_buffer[rt].m_lchild, key);
                return s_buffer[rt].lchild()->m_size + 1 + _rank(s_buffer[rt].m_rchild, key);
            }
            static size_type _smaller_bound(size_type rt, const key_type &key) {
                if (!rt) return 0;
                _pushdown(rt);
                if (!_comp(s_buffer[rt].get(), key)) return _smaller_bound(s_buffer[rt].m_lchild, key);
                size_type res = _smaller_bound(s_buffer[rt].m_rchild, key);
                return res ? res : rt;
            }
            static size_type _lower_bound(size_type rt, const key_type &key) {
                if (!rt) return 0;
                _pushdown(rt);
                if (_comp(s_buffer[rt].get(), key)) return _lower_bound(s_buffer[rt].m_rchild, key);
                size_type res = _lower_bound(s_buffer[rt].m_lchild, key);
                return res ? res : rt;
            }
            static size_type _upper_bound(size_type rt, const key_type &key) {
                if (!rt) return 0;
                _pushdown(rt);
                if (!_comp(key, s_buffer[rt].get())) return _upper_bound(s_buffer[rt].m_rchild, key);
                size_type res = _upper_bound(s_buffer[rt].m_lchild, key);
                return res ? res : rt;
            }
            static void lock() { s_lock = true; }
            static void unlock() { s_lock = false; }
            tree_type copy() {
                tree_type other;
                if (m_root) other.m_root = _create(root());
                return other;
            }
            template <typename Modify = Ignore>
            void insert_by_key(const key_type &key, Modify &&modify = Modify()) { _insert(&m_root, _create(key, modify), ValueLessJudger(key)); }
            template <typename Modify = Ignore>
            void insert_by_rank(const key_type &key, size_type k, Modify &&modify = Modify()) { _insert(&m_root, _create(key, modify), RankJudger(k)); }
            bool erase_by_key(const key_type &key) { return _erase_by_key(&m_root, key); }
            void erase_by_rank(size_type k) { _erase_by_rank(&m_root, k); }
            template <typename Modify>
            bool modify_by_key(const key_type &key, Modify &&modify) { return _modify_by_key(m_root, key, modify); }
            template <typename Modify>
            void modify_by_rank(size_type k, Modify &&modify) { _modify_by_rank(m_root, k, modify); }
            tree_type split_by_key(const key_type &key) {
                tree_type other;
                _split(m_root, &m_root, &other.m_root, ValueLessEqualJudger(key));
                return other;
            }
            tree_type split_by_rank(size_type k) {
                tree_type other;
                _split(m_root, &m_root, &other.m_root, RankJudger(k));
                return other;
            }
            void join(tree_type other) { _join(&m_root, m_root, other.m_root); }
            template <typename Func = Ignore>
            void merge(tree_type other, Func func = Func()) { _merge(&m_root, m_root, other.m_root, func); }
            node *root() const { return s_buffer + m_root; }
            size_type size() const { return s_buffer[m_root].m_size; }
            node *kth(size_type k) const { return s_buffer + _kth(m_root, k); }
            template <typename Judger>
            size_type min_left(size_type right, Judger &&judger) {
                tree_type other = split_by_rank(right + 1);
                size_type res = _min_left(m_root, judger);
                join(other);
                return res;
            }
            template <typename Judger>
            size_type max_right(size_type left, Judger &&judger) {
                tree_type other = split_by_rank(left);
                size_type res = _max_right(other.m_root, judger);
                join(other);
                return left + res;
            }
            size_type rank(const key_type &key) const { return _rank(m_root, key); }
            node *smaller_bound(const key_type &key) const { return s_buffer + _smaller_bound(m_root, key); }
            node *lower_bound(const key_type &key) const { return s_buffer + _lower_bound(m_root, key); }
            node *upper_bound(const key_type &key) const { return s_buffer + _upper_bound(m_root, key); }
        };
        template <typename Ostream, template <typename> typename NodeWrapper, bool Lock, size_type MAX_NODE>
        Ostream &operator<<(Ostream &out, const Multiset<NodeWrapper, Lock, MAX_NODE> &x) {
            out << "{";
            for (size_type i = 0; i < x.size(); i++) {
                if (i) out << ", ";
                out << *x.kth(i);
            }
            return out << "}";
        }
        template <template <typename> typename NodeWrapper, bool Lock, size_type MAX_NODE>
        typename Multiset<NodeWrapper, Lock, MAX_NODE>::node Multiset<NodeWrapper, Lock, MAX_NODE>::s_buffer[MAX_NODE];
        template <template <typename> typename NodeWrapper, bool Lock, size_type MAX_NODE>
        size_type Multiset<NodeWrapper, Lock, MAX_NODE>::s_use_count = 1;
        template <template <typename> typename NodeWrapper, bool Lock, size_type MAX_NODE>
        bool Multiset<NodeWrapper, Lock, MAX_NODE>::s_lock = true;
    }
    template <typename Tp, typename Compare = std::less<Tp>, bool Lock = false, PerFHQ::size_type MAX_NODE = 1 << 20, typename Operation, typename TreeType = PerFHQ::Multiset<PerFHQ::CustomNodeWrapper<Tp, Operation, Compare>::template type, Lock, MAX_NODE>>
    auto make_PerFHQTreap(Operation op) -> TreeType { return TreeType::node::s_op = op, TreeType(); }
    template <typename Tp, typename ModifyType, bool InitClearLazy, bool Lock = false, PerFHQ::size_type MAX_NODE = 1 << 20, typename Operation, typename Mapping, typename Composition, typename TreeType = PerFHQ::Multiset<PerFHQ::CustomLazyNodeWrapper<Tp, ModifyType, Operation, Mapping, Composition, InitClearLazy>::template type, Lock, MAX_NODE>>
    auto make_lazy_PerFHQTreap(Operation op, Mapping map, Composition com, const ModifyType &default_modify = ModifyType()) -> TreeType { return TreeType::node::s_default_modify = default_modify, TreeType(); }
    template <typename Tp, typename Compare = std::less<Tp>, bool Lock = false, PerFHQ::size_type MAX_NODE = 1 << 20>
    using PerFHQTreap = PerFHQ::Multiset<PerFHQ::BaseNodeWrapper<Tp, Compare>::template type, Lock, MAX_NODE>;
}

#endif