#pragma once
#include "model/clinicdatabase.h"
#include <QString>
namespace clinic {
class FileHandler {
public:
    static void save(const ClinicDatabase& db, const QString& path);
    static void load(ClinicDatabase& db, const QString& path);
};
}
