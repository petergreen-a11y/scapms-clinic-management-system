#include "model/clinicdatabase.h"
#include "model/appointable.h"
#include <algorithm>
#include <stdexcept>
namespace clinic {
bool ClinicDatabase::addPatient(std::unique_ptr<PatientRecord> p) {
    if (!p) throw std::invalid_argument("Patient record is missing.");
    if (findPatientById(p->id())) return false;
    patients_.push_back(std::move(p)); return true;
}
PatientRecord* ClinicDatabase::findPatientById(const std::string& id) const {
    for (const auto& p : patients_) if (p->id() == id) return p.get();
    return nullptr;
}
bool ClinicDatabase::deletePatient(const std::string& id) {
    auto it = std::find_if(patients_.begin(), patients_.end(), [&](const auto& p){ return p->id() == id; });
    if (it == patients_.end()) return false;
    appointments_.erase(std::remove_if(appointments_.begin(), appointments_.end(),
        [&](const Appointment& a){ return a.patientId() == id; }), appointments_.end());
    patients_.erase(it);
    return true;
}
bool ClinicDatabase::addAppointment(const Appointment& a) {
    for (const auto& old : appointments_) if (old.id() == a.id()) return false;
    auto* patient = findPatientById(a.patientId());
    if (!patient) throw std::invalid_argument("Patient ID does not exist.");
    auto* appointable = dynamic_cast<Appointable*>(patient);
    if (!appointable) throw std::invalid_argument("This patient type cannot be scheduled.");
    appointments_.push_back(a);
    try {
        appointable->scheduleAppointment(a.id(), a.dateTime());
    } catch (...) {
        appointments_.pop_back();
        throw;
    }
    return true;
}
bool ClinicDatabase::deleteAppointment(const std::string& id) {
    auto it = std::find_if(appointments_.begin(), appointments_.end(), [&](const Appointment& a){ return a.id() == id; });
    if (it == appointments_.end()) return false;
    auto* patient = findPatientById(it->patientId());
    if (patient) {
        if (auto* appointable = dynamic_cast<Appointable*>(patient))
            appointable->cancelAppointment(id);
    }
    appointments_.erase(it);
    return true;
}
Appointment* ClinicDatabase::findAppointment(const std::string& id) {
    for (auto& a : appointments_) if (a.id() == id) return &a;
    return nullptr;
}
void ClinicDatabase::clear() { patients_.clear(); appointments_.clear(); }
}
