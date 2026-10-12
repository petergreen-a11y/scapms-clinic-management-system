#pragma once
#include <QMainWindow>
#include "controller/clinicmanager.h"
class QTableWidget;
class QLineEdit;
class QComboBox;
class QLabel;
class QTabWidget;
namespace clinic {
class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent=nullptr);
private:
    void buildUi();
    QWidget* makePatientsTab();
    QWidget* makeAppointmentsTab();
    QWidget* makeQueueTab();
    QWidget* makeSearchSortTab();
    QWidget* makeReportsTab();
    void refreshPatients();
    void refreshAppointments();
    void refreshQueue();
    void refreshDashboard();
    void updateStatus(const QString& text);
    void addPatient();
    void editPatient();
    void deletePatient();
    void scheduleAppointment();
    void cancelAppointment();
    void changeAppointmentStatus();
    void addToQueue();
    void callNext();
    void searchPatients();
    void sortPatients();
    void showReports();
    void saveData();
    void loadData();
    void undoLast();
    ClinicManager manager_;
    QTableWidget *patientsTable_=nullptr, *appointmentsTable_=nullptr, *queueTable_=nullptr, *searchTable_=nullptr;
    QLineEdit *searchEdit_=nullptr;
    QComboBox *searchAlgorithm_=nullptr, *sortAlgorithm_=nullptr, *sortField_=nullptr, *searchField_=nullptr;
    QLabel *statusLabel_=nullptr, *reportLabel_=nullptr;
    QLabel *dashboardPatients_=nullptr, *dashboardToday_=nullptr, *dashboardWaiting_=nullptr, *dashboardPending_=nullptr;
};
}
