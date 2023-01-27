
typedef long long ll;
// a*b mod p

ll quick_mul(ll a, ll b, ll p) { // ll 是long long类型
    ll ans = 0;
    for(; b; b >>= 1) {
        if(b & 1) ans = (ans + a) % p;
        a = a * 2 % p;
    }
    return ans;
}