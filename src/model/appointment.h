#pragma once
#include <string>
namespace clinic {
class Appointment {
public:
    Appointment(std::string id, std::string patientId, std::string dateTime, std::string status = "Pending");
    const std::string& id() const { return id_; }
    const std::string& patientId() const { return patientId_; }
    const std::string& dateTime() const { return dateTime_; }
    const std::string& status() const { return status_; }
    void setStatus(const std::string& status);
private:
    std::string id_, patientId_, dateTime_, status_;
};
}
