class Solution {
public:
    struct Seg {
        int left, right;
        bool operator < (const Seg& rhs) const {
            if (right == rhs.right) {
                return left > rhs.left;
            }
            return right < rhs.right;
        }
    };

    vector<string> maxNumOfSubstrings(string s) {
        vector<Seg> seg(26, (Seg){-1, -1});
        // 预处理左右端点
        for (int i = 0; i < s.length(); ++i) {
            int char_idx = s[i] - 'a';
            if (seg[char_idx].left == -1) {
                seg[char_idx].left = seg[char_idx].right = i;
            } else {
                seg[char_idx].right = i;
            }
        }
        // 这里的迭代递归才是重点
        for (int i = 0; i < 26; ++i) {
            if (seg[i].left != -1) {
                for (int j = seg[i].left; j <= seg[i].right; ++j) {
                    int char_idx = s[j] - 'a';
                    if (seg[i].left <= seg[char_idx].left && seg[char_idx].right <= seg[i].right) {
                        continue;
                    }
                    seg[i].left = min(seg[i].left, seg[char_idx].left);
                    seg[i].right = max(seg[i].right, seg[char_idx].right);
                    // 重新定义左端点
                    j = seg[i].left;
                }
            }
        }
        // 贪心选取
        sort(seg.begin(), seg.end());
        vector<string> ans;
        int end = -1;
        for (auto& segment: seg) {
            int left = segment.left, right = segment.right;
            if (left == -1) {
                continue;
            }
            if (end == -1 || left > end) {
                end = right;
                ans.emplace_back(s.substr(left, right - left + 1));
            }
        }
        return ans;
    }
};

