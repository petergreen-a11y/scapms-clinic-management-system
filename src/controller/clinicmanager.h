#pragma once
#include "model/clinicdatabase.h"
#include <queue>
#include <stack>
#include <array>
#include <string>
#include <vector>
namespace clinic {
class ClinicManager {
public:
    enum class UndoType { AddPatient, DeletePatient, EditPatient, AddAppointment, DeleteAppointment, EditAppointment };
    struct UndoAction {
        UndoType type;
        std::string patientId;
        std::string oldName, oldContact, oldCategory;
        int oldAge = 0;
        std::string appointmentId, oldDateTime, oldStatus;
        std::vector<Appointment> relatedAppointments;
    };
    ClinicDatabase& database() { return db_; }
    const ClinicDatabase& database() const { return db_; }
    bool addPatient(std::unique_ptr<PatientRecord> p);
    bool deletePatient(const std::string& id);
    void editPatient(const std::string& id, const std::string& name, int age, const std::string& contact);
    bool addAppointment(const Appointment& a);
    bool cancelAppointment(const std::string& id);
    void setAppointmentStatus(const std::string& id, const std::string& status);
    void enqueue(const std::string& patientId);
    bool callNext(std::string& patientId);
    const std::queue<std::string>& waitingQueue() const { return queue_; }
    bool undo();
    void refreshCache(PatientRecord* p);
    PatientRecord* findPatientById(const std::string& id);
    void resetTransientState();
    PatientRecord* cachedPatient(std::size_t index) const;
private:
    ClinicDatabase db_;
    std::queue<std::string> queue_;
    std::stack<UndoAction> undo_;
    std::array<PatientRecord*,10> cache_{};
    std::size_t cacheNext_=0;
};
}
