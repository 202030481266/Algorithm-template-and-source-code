#include "DS/SegBIT.h"
#include "IO/FastIO.h"

int main() {
    // 二维线段树与一维线段树的用法基本类似，所以只展示一下特殊方法
    int matrix[4][5] = {
        {3, 0, 1, 4, 2},
        {5, 6, 3, 2, 1},
        {1, 2, 0, 1, 5},
        {4, 1, 0, 1, 7},
    };
    // 除了行数、列数，还需要传递一个寻址函数
    OY::SegBITSumTree<false, uint32_t, 1000, 10000> T(4, 5, [&](int i, int j) { return matrix[i][j]; });
    cout << T << endl;
    // 输出[0,2]行，[1,4]列的和
    cout << "sum(matrix[0~2][1~4])=" << T.query(0, 2, 1, 4) << endl;
}
/*
#输出如下
[[3, 0, 1, 4, 2], [5, 6, 3, 2, 1], [1, 2, 0, 1, 5], [4, 1, 0, 1, 7]]
sum(matrix[0~2][1~4])=27
*/