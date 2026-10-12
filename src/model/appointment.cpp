#include "model/appointment.h"
#include <stdexcept>
#include <utility>
namespace clinic {
Appointment::Appointment(std::string id, std::string patientId, std::string dateTime, std::string status)
    : id_(std::move(id)), patientId_(std::move(patientId)), dateTime_(std::move(dateTime)), status_(std::move(status)) {
    if (id_.empty() || patientId_.empty() || dateTime_.empty())
        throw std::invalid_argument("Appointment ID, patient ID and date/time are required.");
    setStatus(status_);
}
void Appointment::setStatus(const std::string& status) {
    if (status != "Pending" && status != "Completed" && status != "Cancelled")
        throw std::invalid_argument("Appointment status must be Pending, Completed or Cancelled.");
    status_ = status;
}
}
