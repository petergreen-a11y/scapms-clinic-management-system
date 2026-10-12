#include "model/emergencypatient.h"
#include <algorithm>
#include <stdexcept>
namespace clinic {
std::string EmergencyPatient::category() const { return "Emergency"; }
std::string EmergencyPatient::careDescription() const { return "High-priority care"; }
std::unique_ptr<PatientRecord> EmergencyPatient::clone() const {
    auto p = std::make_unique<EmergencyPatient>(id(), name(), age(), contact());
    p->appointmentIds_ = appointmentIds_;
    return p;
}
void EmergencyPatient::scheduleAppointment(const std::string& appointmentId, const std::string& dateTime) {
    if (appointmentId.empty() || dateTime.empty()) throw std::invalid_argument("Appointment ID and date/time are required.");
    appointmentIds_.push_back(appointmentId);
}
bool EmergencyPatient::cancelAppointment(const std::string& appointmentId) {
    auto it = std::find(appointmentIds_.begin(), appointmentIds_.end(), appointmentId);
    if (it == appointmentIds_.end()) return false;
    appointmentIds_.erase(it);
    return true;
}
}
