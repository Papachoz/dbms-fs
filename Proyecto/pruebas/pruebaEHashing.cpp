//
// Created by Usuario on 5/5/2026.
//
#include <bits/stdc++.h>
#include "../Index/ExtendibleHashing.h"

using namespace std;

int main() {
    // limpiar archivo anterior para prueba fresca
    remove("ehash_test.bin");

    Diske disk("ehash_test.bin");
    ExtendibleHashing<int> eh(disk);

    // ── 1. INSERT ────────────────────────────────────────────────
    cout << "===== INSERT =====\n";

    vector<int> keys;
    for (int i = 1; i <= 200; i++)
        keys.push_back(i);

    // shuffle para romper implementaciones que solo funcionan en orden
    mt19937 rng(42);
    shuffle(keys.begin(), keys.end(), rng);

    for (int i = 0; i < (int)keys.size(); i++)
        eh.insert_hash(keys[i], RID_h{1, i});

    cout << "Insertadas " << keys.size() << " keys\n";

    // ── 2. SEARCH (oracle check) ──────────────────────────────────
    cout << "\n===== SEARCH ORACLE =====\n";

    int bad = 0;
    for (int k : keys) {
        vector<RID_h> res = eh.search_hash(k);
        if (res.empty()) {
            cout << "FAIL: key " << k << " no encontrada\n";
            bad++;
        }
    }
    cout << "Total keys: " << keys.size() << "\n";
    cout << "Missing:    " << bad << "\n";

    // ── 3. SEARCH key inexistente ─────────────────────────────────
    cout << "\n===== SEARCH INEXISTENTE =====\n";
    vector<RID_h> r = eh.search_hash(9999);
    cout << "Buscar 9999 (no existe): "
         << (r.empty() ? "OK — vacío" : "FAIL — encontró algo") << "\n";

    // ── 4. DELETE ────────────────────────────────────────────────
    cout << "\n===== DELETE =====\n";

    // borrar la mitad de las keys (usamos el RID exacto con el que se insertó)
    int deleted = 0;
    for (int i = 0; i < (int)keys.size(); i += 2) {
        eh.delete_hash(keys[i], RID_h{1, i});
        deleted++;
    }
    cout << "Borradas " << deleted << " keys\n";

    // verificar que las borradas ya no están
    int falsepositives = 0;
    for (int i = 0; i < (int)keys.size(); i += 2) {
        vector<RID_h> res = eh.search_hash(keys[i]);
        if (!res.empty()) {
            cout << "FAIL: key " << keys[i] << " sigue apareciendo tras delete\n";
            falsepositives++;
        }
    }
    cout << "False positives tras delete: " << falsepositives << "\n";

    // verificar que las NO borradas siguen estando
    int missing = 0;
    for (int i = 1; i < (int)keys.size(); i += 2) {
        vector<RID_h> res = eh.search_hash(keys[i]);
        if (res.empty()) {
            cout << "FAIL: key " << keys[i] << " desapareció sin ser borrada\n";
            missing++;
        }
    }
    cout << "Missing tras delete: " << missing << "\n";

    // ── 5. PERSISTENCIA ──────────────────────────────────────────
    cout << "\n===== PERSISTENCIA =====\n";

    {
        // abrir el mismo archivo con una instancia nueva
        Diske disk2("ehash_test.bin");
        ExtendibleHashing<int> eh2(disk2);

        int missing2 = 0;
        for (int i = 1; i < (int)keys.size(); i += 2) {
            vector<RID_h> res = eh2.search_hash(keys[i]);
            if (res.empty()) {
                missing2++;
            }
        }
        cout << "Keys sobrevivientes tras reabrir archivo: "
             << (keys.size()/2) - missing2 << "/" << keys.size()/2 << "\n";
        cout << (missing2 == 0 ? "PERSISTENCIA OK\n" : "FAIL PERSISTENCIA\n");
    }

    // ── 6. STATS ─────────────────────────────────────────────────
    cout << "\n===== STATS DISCO =====\n";
    cout << "Páginas totales: " << disk.pageCount()     << "\n";
    cout << "Reads:           " << disk.totalReads()    << "\n";
    cout << "Writes:          " << disk.totalWrites()   << "\n";
    cout << "Accesos totales: " << disk.totalAccesses() << "\n";

    return 0;
}
