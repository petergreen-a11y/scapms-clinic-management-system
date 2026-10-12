#include "controller/clinicmanager.h"
#include "controller/searchengine.h"
#include "model/generalpatient.h"
#include "model/emergencypatient.h"
#include "model/followuppatient.h"
#include <cassert>
#include <iostream>
#include <stdexcept>
#include <string>

using namespace clinic;

int main() {
    ClinicManager manager;
    manager = ClinicManager(); // The GUI's New command resets the manager this way.

    // Registration, duplicate-ID protection, and runtime polymorphism.
    assert(manager.addPatient(std::make_unique<GeneralPatient>("P2", "Zara", 32, "0802")));
    assert(manager.addPatient(std::make_unique<EmergencyPatient>("P1", "Ada", 25, "0803")));
    assert(manager.addPatient(std::make_unique<FollowUpPatient>("P3", "Mina", 40, "0804")));
    assert(!manager.addPatient(std::make_unique<GeneralPatient>("P1", "Duplicate", 20, "")));
    assert(manager.database().patients().size() == 3);
    PatientRecord* polymorphicPatient = manager.database().findPatientById("P1");
    assert(polymorphicPatient && polymorphicPatient->category() == "Emergency");
    assert(polymorphicPatient->careDescription() == "High-priority care");

    // Invalid edit must not partially mutate a record.
    bool threw = false;
    try {
        manager.editPatient("P2", "Changed", -1, "new");
    } catch (const std::invalid_argument&) {
        threw = true;
    }
    assert(threw);
    auto* p2 = manager.database().findPatientById("P2");
    assert(p2 && p2->name() == "Zara" && p2->age() == 32 && p2->contact() == "0802");

    // Appointment association and appointment-ID uniqueness.
    assert(manager.addAppointment(Appointment("A1", "P2", "2026-10-12 09:00")));
    assert(manager.addAppointment(Appointment("A2", "P2", "2026-10-12 10:00")));
    assert(manager.addAppointment(Appointment("A3", "P1", "2026-10-12 11:00")));
    assert(!manager.addAppointment(Appointment("A1", "P1", "2026-10-12 12:00")));
    assert(manager.database().appointments().size() == 3);

    // Removing and undoing an appointment preserves the appointment record.
    assert(manager.cancelAppointment("A3"));
    assert(manager.database().findAppointment("A3") == nullptr);
    assert(manager.undo());
    assert(manager.database().findAppointment("A3") != nullptr);

    // Deleting and undoing a patient preserves every associated appointment.
    assert(manager.deletePatient("P2"));
    assert(manager.database().findPatientById("P2") == nullptr);
    assert(manager.database().appointments().size() == 1);
    assert(manager.undo());
    assert(manager.database().findPatientById("P2") != nullptr);
    assert(manager.database().appointments().size() == 3);

    // Appointment status change is reversible and invalid status is rejected.
    manager.setAppointmentStatus("A1", "Completed");
    assert(manager.database().findAppointment("A1")->status() == "Completed");
    assert(manager.undo());
    assert(manager.database().findAppointment("A1")->status() == "Pending");
    threw = false;
    try {
        manager.setAppointmentStatus("A1", "NotAStatus");
    } catch (const std::invalid_argument&) {
        threw = true;
    }
    assert(threw);
    assert(manager.database().findAppointment("A1")->status() == "Pending");

    // FIFO queue; deleting a patient removes their queue entries.
    manager.enqueue("P2");
    manager.enqueue("P1");
    manager.enqueue("P2");
    assert(manager.deletePatient("P2"));
    assert(manager.waitingQueue().size() == 1);
    assert(manager.undo());
    std::string next;
    assert(manager.callNext(next) && next == "P1");
    assert(!manager.callNext(next));

    // Recent-patient cache is updated and can be reset safely.
    assert(manager.findPatientById("P1") != nullptr);
    bool cached = false;
    for (std::size_t i = 0; i < 10; ++i) {
        if (manager.cachedPatient(i) && manager.cachedPatient(i)->id() == "P1")
            cached = true;
    }
    assert(cached);
    manager.resetTransientState();
    for (std::size_t i = 0; i < 10; ++i)
        assert(manager.cachedPatient(i) == nullptr);

    // Manual searching and sorting algorithms.
    auto& records = manager.database().patients();
    SearchEngine::insertionSort(records, "Patient ID");
    assert(records.front()->id() == "P1");
    assert(SearchEngine::linearById(records, "P1") != nullptr);
    assert(SearchEngine::binaryById(records, "P3") != nullptr);
    assert(SearchEngine::recursiveByName(records, "ad") != nullptr);
    SearchEngine::mergeSort(records, "Age");
    assert(records.front()->age() <= records.back()->age());

    // Undo of patient registration removes the latest registered patient.
    assert(manager.addPatient(std::make_unique<GeneralPatient>("P4", "Nora", 22, "")));
    assert(manager.undo());
    assert(manager.database().findPatientById("P4") == nullptr);

    std::cout << "PASS: core model, polymorphism, validation, appointments, queue, undo, cache, search and sorting.\n";
    return 0;
}
