#include "diff.hpp"

#include <algorithm>
#include <stdexcept>

int mod(int n, int M) {
    return ((n % M) + M) % M;
}

std::vector<DiffRange> makeDiffRange(const std::vector<int>& olddoc, const std::vector<int>& newdoc) {
    std::vector<DiffEntry> diffres = diff(olddoc, newdoc);

    if(diffres.empty()) {
        return std::vector<DiffRange>();
    }
    
    std::vector<DiffRange> rangediff;
    // Elements of the diffrange struct
    unsigned int len = 1;
    unsigned int start = 0;
    std::vector<int> terms;

    // Keeps track of whether we're looking at insert/delete
    bool isIns = diffres[0].isIns;

    for(size_t i = 1; i < diffres.size(); ++i) {
        if(isIns == diffres[i].isIns && // Insertion/Deletion type matches
            ((!isIns && diffres[i].oldpos == diffres[i-1].oldpos + 1) //Old position lines up in deletion case
             || (isIns && diffres[i].newpos == diffres[i-1].newpos + 1))) //New position lines up in insertion case
        {
            if(isIns)
                terms.push_back(newdoc[diffres[i].newpos+1]);
            ++len;
        }
        else {
            rangediff.emplace_back(isIns, start, len, terms);
            start = i;
            len = 1;
            terms.clear();
            isIns = diffres[i].isIns;
            if(isIns)
                terms.push_back(newdoc[diffres[i].newpos+1]);
        }
    }

    rangediff.emplace_back(isIns, start, len, terms);

    return rangediff;
}

std::vector<DiffEntry> diff(std::vector<int> e, std::vector<int> f, int i, int j) {
    //  Documented at http://blog.robertelder.org/diff-algorithm/
    int N = e.size();
    int M = f.size();
    int L = N + M;
    int Z = 2 * std::min(N, M) + 2;

    if(N > 0 && M > 0) {
        int w = N-M;
        std::vector<int> g(Z);
        std::vector<int> p(Z);
        for(int h = 0; h < (L/2+(mod(L,2)!=0))+1; h++) {
            for(int r = 0; r < 2; r++) {
                std::vector<int>& c = (r == 0) ? g : p;
                std::vector<int>& d = (r == 0) ? p : g;
                int o, m;
                if(r == 0) {
                    o = 1;
                    m = 1;
                }
                else {
                    o = 0;
                    m = -1;
                }
                for(int k = -(h-2*std::max(0,h-M)); k < h-2*std::max(0,h-N)+1; k += 2) {
                    int a = (k==-h || k!=h && c[mod((k-1),Z)]<c[mod((k+1),Z)]) ? c[mod((k+1),Z)] : c[mod((k-1),Z)]+1;
                    int b = a-k;
                    int s = a;
                    int t = b;
                    while(a<N && b<M && e[(1-o)*N+m*a+(o-1)]==f[(1-o)*M+m*b+(o-1)]) {
                        ++a; ++b;
                    }
                    c[mod(k,Z)] = a;
                    int z = -(k-w);
                    if(mod(L,2)==o && z>=-(h-o) && z<=h-o && c[mod(k,Z)]+d[mod(z,Z)] >= N) {
                        int D, x, y, u, v;
                        if(o == 1) {
                            D = 2*h-1;
                            x = s;
                            y = t;
                            u = a;
                            v = b;
                        }
                        else {
                            D = 2*h;
                            x = N-a;
                            y = M-b;
                            u = N-s;
                            v = M-t;
                        }
                        if(D > 1 || (x != u && y != v)) {
                            auto first = diff(std::vector<int>(e.begin(), e.begin() + x),std::vector<int>(f.begin(), f.begin() + y),i,j);
                            auto second = diff(std::vector<int>(e.begin()+u, e.begin()+N), std::vector<int>(f.begin()+v,f.begin()+M),i+u,j+v);
                            first.insert(first.end(), second.begin(), second.end());
                            return first;
                        }
                        else if(M > N)
                            return diff({}, std::vector<int>(f.begin()+N,f.begin()+M),i+N,j+N);
                        else if(M < N)
                            return diff(std::vector<int>(e.begin()+M, e.begin()+N), {}, i+M, j+M);
                        else
                            return {};
                    }
                }
            }
        }
        throw std::runtime_error("failed to find diff");
    }
    else if(N > 0) {
        std::vector<DiffEntry> res;
        res.reserve(N);
        for(int off = 0; off < N; off++)
            res.push_back(DiffEntry(false, i+off, 0));
        return res;
    }
    else {
        std::vector<DiffEntry> res;
        res.reserve(M);
        for(int off = 0; off < M; off++)
            res.push_back(DiffEntry(true, i, j+off));
        return res;
    }
}