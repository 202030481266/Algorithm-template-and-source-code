class Solution {
public:
    int xorBeauty(vector<int>& nums) {
        int ans = 0;
        for (auto c : nums) ans ^= c;
        return ans;
    }
};