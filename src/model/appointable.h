#pragma once
#include <string>
namespace clinic {
class Appointable {
public:
    virtual ~Appointable() = default;
    virtual void scheduleAppointment(const std::string& appointmentId, const std::string& dateTime) = 0;
    virtual bool cancelAppointment(const std::string& appointmentId) = 0;
};
}
