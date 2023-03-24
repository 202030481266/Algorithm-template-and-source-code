// https://leetcode.cn/problems/number-of-pairs-satisfying-inequality/



template <class T>
struct fenwick {
    vector<T> t;

    fenwick(int n) : t(n) {}

    void add(int i, T value) {
        for (; i < t.size(); i |= i + 1)
            t[i] += value;
    }

    // sum[0..i]
    T sum(int i) {
        T res{};
        for (; i >= 0; i = (i & (i + 1)) - 1)
            res += t[i];
        return res;
    }

    // returns min(p | sum[0..p] >= sum)
    // requires non-negative tree values
    int lower_bound(T sum) {
        int highest_one_bit = 1;
        while (highest_one_bit << 1 <= t.size())
            highest_one_bit <<= 1;
        int pos = 0;
        // 使用倍增的方法来进行查找
        for (size_t blockSize = highest_one_bit; blockSize != 0; blockSize >>= 1) {
            int p = pos + blockSize - 1;
            if (p < t.size() && t[p] < sum) {
                sum -= t[p];
                pos += blockSize;
            }
        }
        return pos;
    }
};
class Solution {
public:
    static constexpr int N = 40000;
    long long numberOfPairs(vector<int>& nums1, vector<int>& nums2, int diff) {
        int n = nums1.size();

        fenwick<int> tree(N * 2);
        tree.add(nums1[0] - nums2[0] + N, 1);

        long long ans = 0;
        for (int i = 1; i < n; ++i) {
            int tmp = nums1[i] - nums2[i] + diff;
            ans += tree.sum(tmp + N);
            tree.add(tmp - diff + N, 1);
        }

        return ans;
    }
};