#pragma once
#include "model/patientrecord.h"
#include "model/appointable.h"
#include <string>
#include <vector>
namespace clinic {
class GeneralPatient final : public PatientRecord, public Appointable {
public:
    using PatientRecord::PatientRecord;
    std::string category() const override;
    std::string careDescription() const override;
    std::unique_ptr<PatientRecord> clone() const override;
    void scheduleAppointment(const std::string& appointmentId, const std::string& dateTime) override;
    bool cancelAppointment(const std::string& appointmentId) override;
private:
    std::vector<std::string> appointmentIds_;
};
}
