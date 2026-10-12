#include "controller/clinicmanager.h"
#include "model/generalpatient.h"
#include "model/emergencypatient.h"
#include "model/followuppatient.h"
#include <stdexcept>
namespace clinic {
bool ClinicManager::addPatient(std::unique_ptr<PatientRecord> p) {
    if (!p) throw std::invalid_argument("Patient record is missing.");
    std::string id=p->id();
    if(!db_.addPatient(std::move(p))) return false;
    UndoAction action{}; action.type=UndoType::AddPatient; action.patientId=id; undo_.push(std::move(action));
    refreshCache(db_.findPatientById(id)); return true;
}
bool ClinicManager::deletePatient(const std::string& id) {
    auto* p=db_.findPatientById(id);
    if(!p) return false;

    UndoAction u;
    u.type=UndoType::DeletePatient;
    u.patientId=id;
    u.oldName=p->name();
    u.oldAge=p->age();
    u.oldContact=p->contact();
    u.oldCategory=p->category();

    for (const auto& appointment : db_.appointments()) {
        if (appointment.patientId() == id)
            u.relatedAppointments.push_back(appointment);
    }

    undo_.push(std::move(u));
    db_.deletePatient(id);

    // Remove this patient from the FIFO queue without changing the order of others.
    std::queue<std::string> remaining;
    while (!queue_.empty()) {
        const std::string queuedId = queue_.front();
        queue_.pop();
        if (queuedId != id) remaining.push(queuedId);
    }
    queue_ = std::move(remaining);

    for(auto& ptr:cache_) if(ptr && ptr->id()==id) ptr=nullptr;
    return true;
}

void ClinicManager::editPatient(const std::string& id,const std::string& name,int age,const std::string& contact) {
    auto* p=db_.findPatientById(id);
    if(!p) throw std::invalid_argument("Patient not found.");
    if(name.empty()) throw std::invalid_argument("Patient name is required.");
    if(age < 0 || age > 120) throw std::invalid_argument("Patient age must be between 0 and 120.");

    UndoAction u;
    u.type=UndoType::EditPatient;
    u.patientId=id;
    u.oldName=p->name();
    u.oldAge=p->age();
    u.oldContact=p->contact();
    u.oldCategory=p->category();
    undo_.push(std::move(u));

    p->setName(name);
    p->setAge(age);
    p->setContact(contact);
    refreshCache(p);
}

bool ClinicManager::addAppointment(const Appointment& a) {
    if(!db_.addAppointment(a)) return false;
    UndoAction action{}; action.type=UndoType::AddAppointment; action.appointmentId=a.id(); undo_.push(std::move(action)); return true;
}
bool ClinicManager::cancelAppointment(const std::string& id) {
    auto* a=db_.findAppointment(id); if(!a) return false;
    UndoAction u; u.type=UndoType::DeleteAppointment; u.appointmentId=a->id(); u.patientId=a->patientId(); u.oldDateTime=a->dateTime(); u.oldStatus=a->status(); undo_.push(u);
    return db_.deleteAppointment(id);
}
void ClinicManager::setAppointmentStatus(const std::string& id,const std::string& status) {
    auto* a=db_.findAppointment(id);
    if(!a) throw std::invalid_argument("Appointment not found.");
    if(status != "Pending" && status != "Completed" && status != "Cancelled")
        throw std::invalid_argument("Invalid appointment status.");

    UndoAction u;
    u.type=UndoType::EditAppointment;
    u.appointmentId=id;
    u.patientId=a->patientId();
    u.oldDateTime=a->dateTime();
    u.oldStatus=a->status();
    undo_.push(std::move(u));
    a->setStatus(status);
}

void ClinicManager::enqueue(const std::string& id) {
    if(!db_.findPatientById(id)) throw std::invalid_argument("Patient ID does not exist.");
    queue_.push(id);
}
bool ClinicManager::callNext(std::string& id) {
    while(!queue_.empty()) { id=queue_.front(); queue_.pop(); if(db_.findPatientById(id)) return true; }
    return false;
}
bool ClinicManager::undo() {
    if(undo_.empty()) return false;
    UndoAction u=std::move(undo_.top());
    undo_.pop();

    switch(u.type) {
    case UndoType::AddPatient:
        db_.deletePatient(u.patientId);
        for(auto& ptr:cache_) if(ptr && ptr->id()==u.patientId) ptr=nullptr;
        break;

    case UndoType::DeletePatient: {
        std::unique_ptr<PatientRecord> p;
        if(u.oldCategory=="General")
            p=std::make_unique<GeneralPatient>(u.patientId,u.oldName,u.oldAge,u.oldContact);
        else if(u.oldCategory=="Emergency")
            p=std::make_unique<EmergencyPatient>(u.patientId,u.oldName,u.oldAge,u.oldContact);
        else
            p=std::make_unique<FollowUpPatient>(u.patientId,u.oldName,u.oldAge,u.oldContact);

        if (!db_.addPatient(std::move(p)))
            throw std::runtime_error("Undo failed because the patient ID is already in use.");
        for (const auto& appointment : u.relatedAppointments)
            db_.addAppointment(appointment);
        refreshCache(db_.findPatientById(u.patientId));
        break;
    }

    case UndoType::EditPatient: {
        auto* p=db_.findPatientById(u.patientId);
        if(p) {
            p->setName(u.oldName);
            p->setAge(u.oldAge);
            p->setContact(u.oldContact);
            refreshCache(p);
        }
        break;
    }

    case UndoType::AddAppointment:
        db_.deleteAppointment(u.appointmentId);
        break;

    case UndoType::DeleteAppointment:
        db_.addAppointment(Appointment(u.appointmentId,u.patientId,u.oldDateTime,u.oldStatus));
        break;

    case UndoType::EditAppointment: {
        auto* a=db_.findAppointment(u.appointmentId);
        if(a) a->setStatus(u.oldStatus);
        break;
    }}
    return true;
}

void ClinicManager::refreshCache(PatientRecord* p) {
    if(!p) return;
    for(auto& x:cache_) if(x && x->id()==p->id()) { x=p; return; }
    cache_[cacheNext_]=p; cacheNext_=(cacheNext_+1)%cache_.size();
}
PatientRecord* ClinicManager::cachedPatient(std::size_t i) const { return i<cache_.size()?cache_[i]:nullptr; }

PatientRecord* ClinicManager::findPatientById(const std::string& id) {
    auto* patient = db_.findPatientById(id);
    if (patient) refreshCache(patient);
    return patient;
}

void ClinicManager::resetTransientState() {
    queue_ = std::queue<std::string>();
    undo_ = std::stack<UndoAction>();
    cache_.fill(nullptr);
    cacheNext_ = 0;
}

} // namespace clinic
