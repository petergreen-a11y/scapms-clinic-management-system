#include "model/patientrecord.h"
#include <stdexcept>
#include <utility>
namespace clinic {
PatientRecord::PatientRecord(std::string id, std::string name, int age, std::string contact)
    : id_(std::move(id)), name_(std::move(name)), age_(age), contact_(std::move(contact)) {
    if (id_.empty()) throw std::invalid_argument("Patient ID is required.");
    if (name_.empty()) throw std::invalid_argument("Patient name is required.");
    if (age_ < 0 || age_ > 120) throw std::invalid_argument("Patient age must be between 0 and 120.");
}
void PatientRecord::setName(const std::string& value) {
    if (value.empty()) throw std::invalid_argument("Patient name is required.");
    name_ = value;
}
void PatientRecord::setAge(int value) {
    if (value < 0 || value > 120) throw std::invalid_argument("Patient age must be between 0 and 120.");
    age_ = value;
}
}
