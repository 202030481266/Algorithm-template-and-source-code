class SuffixArray {
public:
    using size_type = unsigned;
    using pointer = size_type*;
    using const_pointer = const size_type*;

private:
    template<typename S>
    inline static bool substring_equal(const S& s, size_type p1, size_type p2, size_type len) {
        for (size_type i = 0;i < len;++i)
            if (s[p1 + i] != s[p2 + i])
                return false;
        return true;
    }

    template<typename S>
    inline static void induced_sort(
        const S& s,
        pointer sa,
        bool* type,
        pointer pos,
        pointer lbuk,
        pointer sbuk,
        size_type n,
        size_type m,
        size_type n0) {
        std::fill_n(sa, n, 0);
        std::copy_n(lbuk + 1, m - 1, sbuk);
        sbuk[m - 1] = n;
        for (size_type i = n0;i-- > 0;)
            sa[--sbuk[s[pos[i]]]] = pos[i];
        std::copy_n(lbuk + 1, m - 1, sbuk);
        sbuk[m - 1] = n;
        sa[lbuk[s[n - 1]]++] = n - 1;
        for (size_type i = 0;i < n;++i)
            if (sa[i] > 0 && !type[sa[i] - 1])
                sa[lbuk[s[sa[i] - 1]]++] = sa[i] - 1;
        std::copy_n(sbuk, m - 1, lbuk + 1);
        lbuk[0] = 0;
        for (size_type i = n;i-- > 0;)
            if (sa[i] > 0 && type[sa[i] - 1])
                sa[--sbuk[s[sa[i] - 1]]] = sa[i] - 1;
    }

public:
    template<typename S>
    static void sais(
        const S& s,
        pointer sa,
        bool* type,
        pointer len,
        pointer pos,
        pointer lbuk,
        pointer sbuk,
        size_type n,
        size_type m) {
        size_type n0 = 0;
        type[n - 1] = false;
        std::fill_n(len, n, 0);
        for (size_type p = n - 1, i = n - 1;i-- > 0;) {
            type[i] = s[i] != s[i + 1] ? s[i] < s[i + 1] : type[i + 1];
            if (!type[i] && type[i + 1]) {
                len[i + 1] = p - i;
                pos[n0++] = i + 1;
                p = i + 1;
            }
        }
        std::reverse(pos, pos + n0);
        std::fill_n(sbuk, m, 0);
        for (size_type i = 0;i < n;++i)
            ++sbuk[s[i]];
        for (size_type sum = 0, i = 0;i < m;sum += sbuk[i++])
            lbuk[i] = sum;
        induced_sort(s, sa, type, pos, lbuk, sbuk, n, m, n0);
        size_type m0 = -1;
        size_type ppos = -1, plen = 0;
        for (size_type i = 0;i < n;++i) {
            if (len[sa[i]] == 0) continue;
            if (len[sa[i]] != plen || !substring_equal(s, sa[i], ppos, plen)) ++m0;
            plen = len[sa[i]];
            len[sa[i]] = m0;
            ppos = sa[i];
        }
        const pointer s0 = sa;
        const pointer sa0 = sa + n0;
        for (size_type i = 0;i < n0;++i)
            s0[i] = len[pos[i]];
        if (++m0 < n0)
            sais(s0, sa0, type + n, len, pos + n0, sbuk, sbuk + n0, n0, m0);
        else for (size_type i = 0;i < n0;++i)
            sa0[s0[i]] = i;
        for (size_type i = 0;i < n0;++i)
            pos[i + n0] = pos[sa0[i]];
        induced_sort(s, sa, type, pos + n0, lbuk, sbuk, n, m, n0);
    }

private:
    std::unique_ptr<size_type[]> data;

public:
    const const_pointer sa, rk, ht;

public:
    template<typename S>
    SuffixArray(const S& s, size_type n, size_type m)
        : data(std::make_unique<size_type[]>(3 * n))
        , sa(data.get()), rk(sa + n), ht(rk + n) {
        const auto type = std::make_unique<bool[]>(2 * n);
        const auto lbuk = std::make_unique<size_type[]>(m);
        const auto sbuk = std::make_unique<size_type[]>(std::max(n, m));
        const pointer sa = data.get(), rk = sa + n, ht = rk + n;
        sais(s, sa, type.get(), rk, ht, lbuk.get(), sbuk.get(), n, m);
        for (size_type i = 0;i < n;++i)
            rk[sa[i]] = i;
        for (size_type k = 0, i = 0;i < n;++i) {
            if (rk[i] == 0) continue;
            if (k > 0) --k;
            const size_type j = sa[rk[i] - 1];
            const size_type l = n - std::max(i, j);
            for (;k < l;++k) if (s[i + k] != s[j + k]) break;
            ht[rk[i]] = k;
        }
    }

    inline size_type suffix(size_type p) const {
        return sa[p];
    }

    inline size_type rank(size_type p) const {
        return rk[p];
    }

    inline size_type height(size_type p) const {
        return ht[p];
    }
};

