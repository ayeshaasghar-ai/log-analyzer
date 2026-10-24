#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

// Chhota segment tree (sum aur max dono) jo har kadam dikhata hai.
// Node 1 root hai (poori range). Node i ke bachche: 2*i (left) aur 2*i+1 (right).
class SegTreeDemo {
public:
    explicit SegTreeDemo(const std::vector<int>& a)
        : n_(static_cast<int>(a.size())), sum_(4 * a.size(), 0), mx_(4 * a.size(), 0) {
        build(1, 0, n_ - 1, a);
    }

    // Poora tree dikhao (har node ki range, sum aur max)
    void printTree() const { printNode(1, 0, n_ - 1, 0); }

    // Sum query: [ql, qr] (dono shamil). verbose ho to har node ka faisla dikhao.
    int sumQuery(int ql, int qr, bool verbose, int& visited) const {
        visited = 0;
        return querySum(1, 0, n_ - 1, ql, qr, verbose, 0, visited);
    }

    int maxQuery(int ql, int qr, int& visited) const {
        visited = 0;
        return queryMax(1, 0, n_ - 1, ql, qr, visited);
    }

    // Point update: a[idx] = value
    void update(int idx, int value) {
        std::cout << "update index " << idx << " to " << value << ": path ";
        updateNode(1, 0, n_ - 1, idx, value);
        std::cout << "\n";
    }

    int nodeCount() const { return 2 * n_ - 1; }

private:
    int n_;
    std::vector<int> sum_;
    std::vector<int> mx_;

    void build(int node, int l, int r, const std::vector<int>& a) {
        if (l == r) {                       // leaf: ek hi item
            sum_[node] = a[l];
            mx_[node] = a[l];
            return;
        }
        int mid = (l + r) / 2;
        build(2 * node, l, mid, a);
        build(2 * node + 1, mid + 1, r, a);
        sum_[node] = sum_[2 * node] + sum_[2 * node + 1];
        mx_[node] = std::max(mx_[2 * node], mx_[2 * node + 1]);
    }

    void printNode(int node, int l, int r, int depth) const {
        std::cout << std::string(depth * 2, ' ') << "node " << node << " [" << l << "," << r
                  << "] sum=" << sum_[node] << " max=" << mx_[node] << "\n";
        if (l == r) return;
        int mid = (l + r) / 2;
        printNode(2 * node, l, mid, depth + 1);
        printNode(2 * node + 1, mid + 1, r, depth + 1);
    }

    int querySum(int node, int l, int r, int ql, int qr, bool verbose,
                 int depth, int& visited) const {
        visited++;
        std::string pad(depth * 2, ' ');
        if (qr < l || r < ql) {                          // bilkul bahar
            if (verbose) {
                std::cout << pad << "node " << node << " [" << l << "," << r
                          << "]: outside -> skip\n";
            }
            return 0;
        }
        if (ql <= l && r <= qr) {                        // poora andar
            if (verbose) {
                std::cout << pad << "node " << node << " [" << l << "," << r
                          << "]: fully inside -> take sum " << sum_[node] << "\n";
            }
            return sum_[node];
        }
        if (verbose) {                                   // aadha andar: bachchon se poocho
            std::cout << pad << "node " << node << " [" << l << "," << r
                      << "]: partly inside -> ask both children\n";
        }
        int mid = (l + r) / 2;
        return querySum(2 * node, l, mid, ql, qr, verbose, depth + 1, visited)
             + querySum(2 * node + 1, mid + 1, r, ql, qr, verbose, depth + 1, visited);
    }

    int queryMax(int node, int l, int r, int ql, int qr, int& visited) const {
        visited++;
        if (qr < l || r < ql) return -1;                 // bahar: kuch nahi (errors kabhi negative nahi)
        if (ql <= l && r <= qr) return mx_[node];
        int mid = (l + r) / 2;
        return std::max(queryMax(2 * node, l, mid, ql, qr, visited),
                        queryMax(2 * node + 1, mid + 1, r, ql, qr, visited));
    }

    void updateNode(int node, int l, int r, int idx, int value) {
        std::cout << "node " << node << " [" << l << "," << r << "]";
        if (l == r) {                                    // leaf tak pahunch gaye
            sum_[node] = value;
            mx_[node] = value;
            std::cout << " (leaf set)";
            return;
        }
        std::cout << " -> ";
        int mid = (l + r) / 2;
        if (idx <= mid) updateNode(2 * node, l, mid, idx, value);
        else updateNode(2 * node + 1, mid + 1, r, idx, value);
        // wapas aate hue is node ko dobara jodo
        sum_[node] = sum_[2 * node] + sum_[2 * node + 1];
        mx_[node] = std::max(mx_[2 * node], mx_[2 * node + 1]);
    }
};

// Saari (l, r) ranges ke liye tree ka jawab brute force se milao
int countMismatches(const SegTreeDemo& tree, const std::vector<int>& a) {
    int bad = 0;
    int n = static_cast<int>(a.size());
    for (int l = 0; l < n; l++) {
        for (int r = l; r < n; r++) {
            int sum = 0;
            int best = a[l];
            for (int i = l; i <= r; i++) {
                sum += a[i];
                best = std::max(best, a[i]);
            }
            int visited = 0;
            if (tree.sumQuery(l, r, false, visited) != sum) bad++;
            if (tree.maxQuery(l, r, visited) != best) bad++;
        }
    }
    return bad;
}

int main() {
    std::vector<int> errors = {3, 1, 4, 1, 5, 9, 2, 6, 5, 3};
    SegTreeDemo tree(errors);

    std::cout << "=== Part 1: tree (build ke baad) ===\n";
    std::cout << "errors [3 1 4 1 5 9 2 6 5 3]  (n = 10, tree mein " << tree.nodeCount()
              << " nodes)\n";
    tree.printTree();

    std::cout << "\n=== Part 2: sum(2, 5) ka safar ===\n";
    int visited = 0;
    int sum = tree.sumQuery(2, 5, true, visited);
    std::cout << "sum(2, 5) = " << sum << "   (" << visited << " nodes dekhe, brute force mein 4 items)\n";

    std::cout << "\n=== Part 3: max(2, 5) ===\n";
    int best = tree.maxQuery(2, 5, visited);
    std::cout << "max(2, 5) = " << best << "   (" << visited << " nodes dekhe)\n";

    std::cout << "\n=== Part 4: update ===\n";
    tree.update(4, 10);
    int v2 = 0;
    std::cout << "sum(2, 5) ab = " << tree.sumQuery(2, 5, false, v2)
              << ", max(2, 5) ab = " << tree.maxQuery(2, 5, v2) << "\n";
    std::cout << "Sirf 4 nodes badle (kul " << tree.nodeCount()
              << " mein se); prefix sums mein 6 entries badalni padi thi\n";

    std::cout << "\n=== Part 5: saari ranges brute force se milao ===\n";
    errors[4] = 10;     // brute force ke liye data bhi badlo
    int bad = countMismatches(tree, errors);
    std::cout << "55 ranges (sum aur max dono) ka jawab brute force se: "
              << (bad == 0 ? "sab match" : "MISMATCH") << " (galat: " << bad << ")\n";
    return 0;
}