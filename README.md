# SCAPMS  COS202 Programming II Lab Assessment

This C++17 / Qt Widgets project implements the required Smart Clinic Appointment & Patient Management System.

## Build
1. Install Qt 6 with a compatible desktop C++ kit (for example MinGW 64-bit) and CMake.
2. Open `CMakeLists.txt` in Qt Creator.
3. Select the installed Qt desktop kit, configure, build, and run.

## Implemented assessment components
- Abstract `PatientRecord`; `GeneralPatient`, `EmergencyPatient`, `FollowUpPatient`; abstract `Appointable`; runtime polymorphism.
- `Appointment` and `ClinicDatabase`; namespaces and separate model/controller/GUI/utilities.
- `std::vector` patient storage; `std::queue` waiting list; `std::stack` undo; fixed `std::array` cache of 10 patient pointers.
- Manual linear search by ID, binary search by ID (requires sorted records), recursive name search.
- Manual insertion sort and merge sort; sort by name, ID, or age.
- Event-driven Qt Widgets GUI, menus, buttons, text inputs, tables, combo boxes, dialogs and status messages.
- `QVBoxLayout`, `QHBoxLayout`, `QFormLayout`; custom table colouring and input validation; file dialogs; keyboard shortcuts; timer status refresh.
- Exception handling and file persistence for patient and appointment records.
- Reports: most frequently attended patients, pending appointments, category distribution, appointment status summary.

## Important
The source project is provided for you to build and verify in your installed Qt environment. The final PDF needs genuine screenshots captured from the running application and your personal submission details.
