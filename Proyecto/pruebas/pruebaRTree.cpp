#include <iostream>
#include <vector>
#include <cstdio>
#include <cmath>
#include <sstream>
#include <iomanip>
#include <spatialindex/SpatialIndex.h>

#include "../rtree/RangeSearchVisitor.h"
#include "../rtree/KNNVisitor.h"
#include "../rtree/MiPagedDiskStorageManager.h"

struct PuntoRegistro {
    SpatialIndex::id_type id;
    double x;
    double y;
};

std::vector<SpatialIndex::id_type> kNN(
    SpatialIndex::ISpatialIndex& arbol,
    const SpatialIndex::Point& puntoConsulta,
    uint32_t k
) {
    KNNVisitor visitor;

    arbol.nearestNeighborQuery(k, puntoConsulta, visitor);

    return visitor.getResultados();
}

std::vector<SpatialIndex::id_type> rangeSearch(
    SpatialIndex::ISpatialIndex& arbol,
    const SpatialIndex::Point& centro,
    double radio
) {
    uint32_t dimension = centro.getDimension();

    std::vector<double> low(dimension);
    std::vector<double> high(dimension);

    for (uint32_t i = 0; i < dimension; ++i) {
        low[i] = centro.getCoordinate(i) - radio;
        high[i] = centro.getCoordinate(i) + radio;
    }

    SpatialIndex::Region regionBusqueda(low.data(), high.data(), dimension);

    RangeSearchVisitor visitor(centro, radio);

    arbol.intersectsWithQuery(regionBusqueda, visitor);

    return visitor.getResultados();
}

double calcularDistancia(
    double x1,
    double y1,
    double x2,
    double y2
) {
    double dx = x1 - x2;
    double dy = y1 - y2;

    return std::sqrt(dx * dx + dy * dy);
}

bool contieneId(
    const std::vector<SpatialIndex::id_type>& ids,
    SpatialIndex::id_type id
) {
    for (SpatialIndex::id_type actual : ids) {
        if (actual == id) {
            return true;
        }
    }

    return false;
}

int obtenerRank(
    const std::vector<SpatialIndex::id_type>& ids,
    SpatialIndex::id_type id
) {
    for (size_t i = 0; i < ids.size(); ++i) {
        if (ids[i] == id) {
            return static_cast<int>(i + 1);
        }
    }

    return -1;
}

std::string construirArrayResultIds(
    const std::vector<SpatialIndex::id_type>& resultIds
) {
    std::ostringstream json;

    json << "[";

    for (size_t i = 0; i < resultIds.size(); ++i) {
        if (i > 0) {
            json << ", ";
        }

        json << resultIds[i];
    }

    json << "]";

    return json.str();
}

std::string construirJsonRangeSearch(
    const std::vector<PuntoRegistro>& puntos,
    const std::vector<SpatialIndex::id_type>& resultIds,
    double queryX,
    double queryY,
    double radio,
    uint64_t reads,
    uint64_t writes
) {
    std::ostringstream json;

    json << std::fixed << std::setprecision(6);

    json << "{\n";
    json << "  \"type\": \"rangeSearch\",\n";

    json << "  \"query\": {\n";
    json << "    \"x\": " << queryX << ",\n";
    json << "    \"y\": " << queryY << ",\n";
    json << "    \"radio\": " << radio << ",\n";
    json << "    \"k\": null\n";
    json << "  },\n";

    json << "  \"resultIds\": " << construirArrayResultIds(resultIds) << ",\n";

    json << "  \"io\": {\n";
    json << "    \"reads\": " << reads << ",\n";
    json << "    \"writes\": " << writes << "\n";
    json << "  },\n";

    json << "  \"points\": [\n";

    for (size_t i = 0; i < puntos.size(); ++i) {
        const PuntoRegistro& p = puntos[i];

        bool selected = contieneId(resultIds, p.id);
        double distance = calcularDistancia(queryX, queryY, p.x, p.y);

        json << "    {\n";
        json << "      \"id\": " << p.id << ",\n";
        json << "      \"x\": " << p.x << ",\n";
        json << "      \"y\": " << p.y << ",\n";
        json << "      \"selected\": " << (selected ? "true" : "false") << ",\n";
        json << "      \"distance\": " << distance << ",\n";
        json << "      \"rank\": null\n";
        json << "    }";

        if (i + 1 < puntos.size()) {
            json << ",";
        }

        json << "\n";
    }

    json << "  ]\n";
    json << "}";

    return json.str();
}

std::string construirJsonKNN(
    const std::vector<PuntoRegistro>& puntos,
    const std::vector<SpatialIndex::id_type>& resultIds,
    double queryX,
    double queryY,
    uint32_t k,
    uint64_t reads,
    uint64_t writes
) {
    std::ostringstream json;

    json << std::fixed << std::setprecision(6);

    json << "{\n";
    json << "  \"type\": \"kNN\",\n";

    json << "  \"query\": {\n";
    json << "    \"x\": " << queryX << ",\n";
    json << "    \"y\": " << queryY << ",\n";
    json << "    \"radio\": null,\n";
    json << "    \"k\": " << k << "\n";
    json << "  },\n";

    json << "  \"resultIds\": " << construirArrayResultIds(resultIds) << ",\n";

    json << "  \"io\": {\n";
    json << "    \"reads\": " << reads << ",\n";
    json << "    \"writes\": " << writes << "\n";
    json << "  },\n";

    json << "  \"points\": [\n";

    for (size_t i = 0; i < puntos.size(); ++i) {
        const PuntoRegistro& p = puntos[i];

        bool selected = contieneId(resultIds, p.id);
        double distance = calcularDistancia(queryX, queryY, p.x, p.y);
        int rank = obtenerRank(resultIds, p.id);

        json << "    {\n";
        json << "      \"id\": " << p.id << ",\n";
        json << "      \"x\": " << p.x << ",\n";
        json << "      \"y\": " << p.y << ",\n";
        json << "      \"selected\": " << (selected ? "true" : "false") << ",\n";
        json << "      \"distance\": " << distance << ",\n";

        if (rank == -1) {
            json << "      \"rank\": null\n";
        } else {
            json << "      \"rank\": " << rank << "\n";
        }

        json << "    }";

        if (i + 1 < puntos.size()) {
            json << ",";
        }

        json << "\n";
    }

    json << "  ]\n";
    json << "}";

    return json.str();
}

void imprimirResultados(
    const std::string& titulo,
    const std::vector<SpatialIndex::id_type>& resultados
) {
    std::cout << "\n" << titulo << std::endl;

    if (resultados.empty()) {
        std::cout << "No se encontraron resultados." << std::endl;
        return;
    }

    for (SpatialIndex::id_type id : resultados) {
        std::cout << "ID encontrado: " << id << std::endl;
    }
}

void imprimirIO(
    const std::string& operacion,
    MiPagedDiskStorageManager* almacenamiento
) {
    std::cout << "\nI/O " << operacion << ":" << std::endl;
    std::cout << "Reads : " << almacenamiento->getReadCount() << std::endl;
    std::cout << "Writes: " << almacenamiento->getWriteCount() << std::endl;
}

void insertarPuntosEnArbol(
    SpatialIndex::ISpatialIndex* arbol,
    const std::vector<PuntoRegistro>& puntos
) {
    for (const PuntoRegistro& p : puntos) {
        double coords[] = {p.x, p.y};

        SpatialIndex::Point punto(coords, 2);
        SpatialIndex::Region region(punto, punto);

        arbol->insertData(0, nullptr, region, p.id);
    }
}

int main() {
    // Para que cada ejecución sea limpia.
    // Si quieres persistencia real entre ejecuciones, elimina esta línea.
    std::remove("rtree_pages.dat");

    auto* almacenamiento =
        new MiPagedDiskStorageManager("rtree_pages.dat");

    SpatialIndex::id_type idIndice;

    SpatialIndex::ISpatialIndex* arbol =
        SpatialIndex::RTree::createNewRTree(
            *almacenamiento,
            0.7,
            10,
            10,
            2,
            SpatialIndex::RTree::RV_RSTAR,
            idIndice
        );

    std::cout << "R-Tree creado correctamente." << std::endl;
    std::cout << "ID del indice: " << idIndice << std::endl;

    std::vector<PuntoRegistro> puntos = {
        {101, 1.0, 1.0},
        {102, 2.0, 2.0},
        {103, 8.0, 8.0},
        {104, 4.0, 4.0},
        {105, 1.0, 3.0}
    };

    // ============================
    // Prueba 0: inserciones
    // ============================
    almacenamiento->resetCounters();

    insertarPuntosEnArbol(arbol, puntos);

    imprimirIO("inserciones", almacenamiento);

    almacenamiento->dumpPages();

    double queryX = 1.0;
    double queryY = 1.0;

    double consultaCoords[] = {queryX, queryY};
    SpatialIndex::Point puntoConsulta(consultaCoords, 2);

    // ============================
    // Prueba 1: rangeSearch(point, radio)
    // ============================
    double radio = 2.0;

    almacenamiento->resetCounters();

    std::vector<SpatialIndex::id_type> resultadosRange =
        rangeSearch(*arbol, puntoConsulta, radio);

    uint64_t rangeReads = almacenamiento->getReadCount();
    uint64_t rangeWrites = almacenamiento->getWriteCount();

    imprimirResultados(
        "Resultados de rangeSearch desde (1,1) con radio 2.0:",
        resultadosRange
    );

    imprimirIO("rangeSearch", almacenamiento);

    std::string jsonRange = construirJsonRangeSearch(
        puntos,
        resultadosRange,
        queryX,
        queryY,
        radio,
        rangeReads,
        rangeWrites
    );

    std::cout << "\nJSON RangeSearch para frontend:\n";
    std::cout << jsonRange << std::endl;

    // ============================
    // Prueba 2: kNN(point, k)
    // ============================
    uint32_t k = 3;

    almacenamiento->resetCounters();

    std::vector<SpatialIndex::id_type> resultadosKNN =
        kNN(*arbol, puntoConsulta, k);

    uint64_t knnReads = almacenamiento->getReadCount();
    uint64_t knnWrites = almacenamiento->getWriteCount();

    imprimirResultados(
        "Resultados de kNN desde (1,1) con k = 3:",
        resultadosKNN
    );

    imprimirIO("kNN", almacenamiento);

    std::string jsonKNN = construirJsonKNN(
        puntos,
        resultadosKNN,
        queryX,
        queryY,
        k,
        knnReads,
        knnWrites
    );

    std::cout << "\nJSON kNN para frontend:\n";
    std::cout << jsonKNN << std::endl;

    arbol->flush();
    almacenamiento->flush();

    delete arbol;
    delete almacenamiento;

    return 0;
}