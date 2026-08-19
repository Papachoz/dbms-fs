#include <bits/stdc++.h>
#include "../Index/BPTree.h"
using namespace std;

int main() {
    Disk disk("test.db");
    BPlusTree<FixedString<4>> tree(disk);

    vector<string> keys;

    // Generamos claves que fuerzan splits en TODAS las capas
    for (char a = 'A'; a <= 'D'; a++) {
        for (char b = 'A'; b <= 'Z'; b++) {
            string s;
            s += a;
            s += b;
            s += 'A';
            s += 'A';
            keys.push_back(s);
        }
    }

    // shuffle fuerte (esto rompe árboles mal diseñados)
    mt19937 rng(12345);
    shuffle(keys.begin(), keys.end(), rng);

    // INSERT
    for (int i = 0; i < keys.size(); i++) {
        tree.insert(keys[i], {1, i});
    }

    // ORACLE CHECK
    cout << "\n===== ORACLE CHECK =====\n";

    int bad = 0;

    for (int i = 0; i < keys.size(); i++) {
        RID r = tree.search(keys[i]);

        if (isNullRID(r)) {
            cout << "FAIL: " << keys[i] << "\n";
            bad++;
        }
    }

    cout << "\nTotal keys: " << keys.size() << "\n";
    cout << "Missing: " << bad << "\n";

    return 0;
}