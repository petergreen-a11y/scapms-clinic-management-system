#include "model/followuppatient.h"
#include <algorithm>
#include <stdexcept>
namespace clinic {
std::string FollowUpPatient::category() const { return "Follow-Up"; }
std::string FollowUpPatient::careDescription() const { return "Follow-up visit"; }
std::unique_ptr<PatientRecord> FollowUpPatient::clone() const {
    auto p = std::make_unique<FollowUpPatient>(id(), name(), age(), contact());
    p->appointmentIds_ = appointmentIds_;
    return p;
}
void FollowUpPatient::scheduleAppointment(const std::string& appointmentId, const std::string& dateTime) {
    if (appointmentId.empty() || dateTime.empty()) throw std::invalid_argument("Appointment ID and date/time are required.");
    appointmentIds_.push_back(appointmentId);
}
bool FollowUpPatient::cancelAppointment(const std::string& appointmentId) {
    auto it = std::find(appointmentIds_.begin(), appointmentIds_.end(), appointmentId);
    if (it == appointmentIds_.end()) return false;
    appointmentIds_.erase(it);
    return true;
}
}
