# Core tests

These tests exercise the non-GUI model/controller components and require only a C++17 compiler.

From the project root, compile and run with MinGW (adjust the compiler command if needed):

```bash
g++ -std=c++17 -Wall -Wextra -Wpedantic -Isrc ^
  src/model/patientrecord.cpp src/model/generalpatient.cpp src/model/emergencypatient.cpp ^
  src/model/followuppatient.cpp src/model/appointment.cpp src/model/clinicdatabase.cpp ^
  src/controller/searchengine.cpp src/controller/clinicmanager.cpp tests/core_tests.cpp ^
  -o core_tests.exe
core_tests.exe
```

These tests do not replace a full Qt GUI build and manual UI testing.
