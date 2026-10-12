#include "model/generalpatient.h"
#include <algorithm>
#include <stdexcept>
namespace clinic {
std::string GeneralPatient::category() const { return "General"; }
std::string GeneralPatient::careDescription() const { return "Routine care"; }
std::unique_ptr<PatientRecord> GeneralPatient::clone() const {
    auto p = std::make_unique<GeneralPatient>(id(), name(), age(), contact());
    p->appointmentIds_ = appointmentIds_;
    return p;
}
void GeneralPatient::scheduleAppointment(const std::string& appointmentId, const std::string& dateTime) {
    if (appointmentId.empty() || dateTime.empty()) throw std::invalid_argument("Appointment ID and date/time are required.");
    appointmentIds_.push_back(appointmentId);
}
bool GeneralPatient::cancelAppointment(const std::string& appointmentId) {
    auto it = std::find(appointmentIds_.begin(), appointmentIds_.end(), appointmentId);
    if (it == appointmentIds_.end()) return false;
    appointmentIds_.erase(it);
    return true;
}
}
