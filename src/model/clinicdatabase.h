#pragma once
#include "model/patientrecord.h"
#include "model/appointment.h"
#include <memory>
#include <vector>
#include <string>
namespace clinic {
class ClinicDatabase {
public:
    bool addPatient(std::unique_ptr<PatientRecord> patient);
    PatientRecord* findPatientById(const std::string& id) const;
    bool deletePatient(const std::string& id);
    const std::vector<std::unique_ptr<PatientRecord>>& patients() const { return patients_; }
    std::vector<std::unique_ptr<PatientRecord>>& patients() { return patients_; }
    bool addAppointment(const Appointment& appointment);
    bool deleteAppointment(const std::string& id);
    Appointment* findAppointment(const std::string& id);
    const std::vector<Appointment>& appointments() const { return appointments_; }
    std::vector<Appointment>& appointments() { return appointments_; }
    void clear();
private:
    std::vector<std::unique_ptr<PatientRecord>> patients_;
    std::vector<Appointment> appointments_;
};
}
