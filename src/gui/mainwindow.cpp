#include "gui/mainwindow.h"
#include "controller/searchengine.h"
#include "utils/filehandler.h"
#include "model/generalpatient.h"
#include "model/emergencypatient.h"
#include "model/followuppatient.h"
#include <QApplication>
#include <QComboBox>
#include <QColor>
#include <QDateTimeEdit>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QFormLayout>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QInputDialog>
#include <QLabel>
#include <QLineEdit>
#include <QMenuBar>
#include <QMessageBox>
#include <QPushButton>
#include <QShortcut>
#include <QStatusBar>
#include <QTabWidget>
#include <QTableWidget>
#include <QVBoxLayout>
#include <QWidget>
#include <QDateTime>
#include <QTimer>
#include <QIntValidator>
#include <exception>
#include <algorithm>
#include <QDate>
#include <cctype>

namespace clinic {
static QTableWidget* table(int cols, const QStringList& heads, QWidget* parent) {
    auto* t=new QTableWidget(parent); t->setColumnCount(cols); t->setHorizontalHeaderLabels(heads);
    t->setSelectionBehavior(QAbstractItemView::SelectRows); t->setSelectionMode(QAbstractItemView::SingleSelection);
    t->setEditTriggers(QAbstractItemView::NoEditTriggers); t->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    t->verticalHeader()->setVisible(false); return t;
}
static QString cell(QTableWidget* t,int row,int col) { auto* i=t->item(row,col); return i?i->text():QString(); }
static int selected(QTableWidget* t) { auto r=t->selectedRanges(); return r.isEmpty()?-1:r.first().topRow(); }

MainWindow::MainWindow(QWidget* parent):QMainWindow(parent) {
    setWindowTitle("Smart Clinic Appointment & Patient Management System"); resize(1120,720); buildUi();
}
void MainWindow::buildUi() {
    auto* file=menuBar()->addMenu("&File");
    auto* actNew=file->addAction("&New"); actNew->setShortcut(QKeySequence::New);
    auto* actSave=file->addAction("&Save"); actSave->setShortcut(QKeySequence::Save);
    auto* actLoad=file->addAction("&Load");
    file->addSeparator(); auto* actUndo=file->addAction("&Undo Last Administrative Operation"); actUndo->setShortcut(QKeySequence("Ctrl+Z"));
    file->addSeparator(); auto* actExit=file->addAction("E&xit"); actExit->setShortcut(QKeySequence("Ctrl+Q"));
    connect(actNew,&QAction::triggered,this,[this]{ if(QMessageBox::question(this,"New","Clear current clinic data?")==QMessageBox::Yes){ manager_=ClinicManager(); refreshPatients();refreshAppointments();refreshQueue();updateStatus("New clinic data started."); }});
    connect(actSave,&QAction::triggered,this,&MainWindow::saveData); connect(actLoad,&QAction::triggered,this,&MainWindow::loadData);
    connect(actUndo,&QAction::triggered,this,&MainWindow::undoLast); connect(actExit,&QAction::triggered,qApp,&QApplication::quit);
    auto* tabs=new QTabWidget(this);
    auto* dash=new QWidget(tabs);
    auto* dl=new QVBoxLayout(dash);
    auto* title=new QLabel("Smart Clinic Appointment & Patient Management System");
    QFont f=title->font(); f.setPointSize(18); f.setBold(true); title->setFont(f);
    dl->addWidget(title);
    dl->addWidget(new QLabel("Clinic overview"));
    auto* stats = new QGridLayout();
    auto makeCard = [dash, stats](const QString& caption, QLabel*& value, int row, int col) {
        auto* panel = new QWidget(dash);
        panel->setObjectName("dashboardCard");
        panel->setStyleSheet("QWidget#dashboardCard { border: 1px solid #cbd5e1; border-radius: 8px; background: #f8fafc; }");
        auto* layout = new QVBoxLayout(panel);
        auto* heading = new QLabel(caption, panel);
        value = new QLabel("0", panel);
        QFont numberFont = value->font(); numberFont.setPointSize(20); numberFont.setBold(true); value->setFont(numberFont);
        layout->addWidget(heading); layout->addWidget(value);
        stats->addWidget(panel, row, col);
    };
    makeCard("Registered Patients", dashboardPatients_, 0, 0);
    makeCard("Today's Appointments", dashboardToday_, 0, 1);
    makeCard("Patients Waiting", dashboardWaiting_, 1, 0);
    makeCard("Pending Appointments", dashboardPending_, 1, 1);
    dl->addLayout(stats);
    auto* guide = new QLabel("Use the tabs to manage patient records, appointments, the waiting queue, searching, sorting and reports.", dash);
    guide->setWordWrap(true);
    dl->addWidget(guide);
    dl->addStretch();
    tabs->addTab(dash,"Dashboard");
    tabs->addTab(makePatientsTab(),"Patient Records"); tabs->addTab(makeAppointmentsTab(),"Appointments");
    tabs->addTab(makeQueueTab(),"Waiting Queue"); tabs->addTab(makeSearchSortTab(),"Search & Sort"); tabs->addTab(makeReportsTab(),"Reports");
    setCentralWidget(tabs);
    statusLabel_=new QLabel("Ready"); statusBar()->addWidget(statusLabel_,1);
    new QShortcut(QKeySequence("Ctrl+F"),this,[this,tabs]{tabs->setCurrentIndex(4);searchEdit_->setFocus();});
    new QShortcut(QKeySequence("Ctrl+S"),this,[this]{saveData();});
    auto* timer=new QTimer(this); connect(timer,&QTimer::timeout,this,[this]{ statusBar()->showMessage(QString("Patients: %1 | Appointments: %2 | Waiting: %3").arg(manager_.database().patients().size()).arg(manager_.database().appointments().size()).arg(manager_.waitingQueue().size()),3000); }); timer->start(30000);
}
QWidget* MainWindow::makePatientsTab() {
    auto* w=new QWidget; auto* l=new QVBoxLayout(w); auto* bar=new QHBoxLayout;
    auto* add=new QPushButton("Add Patient"); auto* edit=new QPushButton("Edit Selected"); auto* del=new QPushButton("Delete Selected"); auto* undo=new QPushButton("Undo Last Operation");
    add->setToolTip("Register a new patient"); edit->setToolTip("Edit the selected patient record"); del->setToolTip("Delete the selected patient record");
    bar->addWidget(add);bar->addWidget(edit);bar->addWidget(del);bar->addWidget(undo);bar->addStretch();l->addLayout(bar);
    patientsTable_=table(5,{"Patient ID","Name","Age","Contact","Category"},w); l->addWidget(patientsTable_);
    connect(add,&QPushButton::clicked,this,&MainWindow::addPatient);connect(edit,&QPushButton::clicked,this,&MainWindow::editPatient);
    connect(del,&QPushButton::clicked,this,&MainWindow::deletePatient);connect(undo,&QPushButton::clicked,this,&MainWindow::undoLast);
    return w;
}
QWidget* MainWindow::makeAppointmentsTab() {
    auto* w=new QWidget;auto* l=new QVBoxLayout(w);auto* bar=new QHBoxLayout;
    auto* add=new QPushButton("Schedule Appointment");auto* cancel=new QPushButton("Cancel Selected");auto* status=new QPushButton("Set Status");
    bar->addWidget(add);bar->addWidget(cancel);bar->addWidget(status);bar->addStretch();l->addLayout(bar);
    appointmentsTable_=table(4,{"Appointment ID","Patient ID","Date & Time","Status"},w);l->addWidget(appointmentsTable_);
    connect(add,&QPushButton::clicked,this,&MainWindow::scheduleAppointment);connect(cancel,&QPushButton::clicked,this,&MainWindow::cancelAppointment);
    connect(status,&QPushButton::clicked,this,&MainWindow::changeAppointmentStatus);return w;
}
QWidget* MainWindow::makeQueueTab() {
    auto* w=new QWidget;auto* l=new QVBoxLayout(w);auto* bar=new QHBoxLayout;auto* add=new QPushButton("Add Patient to Queue");auto* next=new QPushButton("Call Next Patient");
    bar->addWidget(add);bar->addWidget(next);bar->addStretch();l->addLayout(bar);queueTable_=table(3,{"Queue Position","Patient ID","Patient Name"},w);l->addWidget(queueTable_);
    connect(add,&QPushButton::clicked,this,&MainWindow::addToQueue);connect(next,&QPushButton::clicked,this,&MainWindow::callNext);return w;
}
QWidget* MainWindow::makeSearchSortTab() {
    auto* w=new QWidget;auto* l=new QVBoxLayout(w);auto* search=new QHBoxLayout;
    searchEdit_=new QLineEdit;searchEdit_->setPlaceholderText("Enter patient ID, name or category");searchField_=new QComboBox;searchField_->addItems({"Patient ID","Patient Name","Category"});
    searchAlgorithm_=new QComboBox;searchAlgorithm_->addItems({"Linear Search","Binary Search (sorted ID)","Recursive Search (name)","Category Search"});
    auto* find=new QPushButton("Search");search->addWidget(searchEdit_,2);search->addWidget(searchField_);search->addWidget(searchAlgorithm_);search->addWidget(find);l->addLayout(search);
    auto* sort=new QHBoxLayout;sortAlgorithm_=new QComboBox;sortAlgorithm_->addItems({"Insertion Sort","Merge Sort"});sortField_=new QComboBox;sortField_->addItems({"Patient Name","Patient ID","Age"});
    auto* sortBtn=new QPushButton("Sort Records");sort->addWidget(new QLabel("Algorithm:"));sort->addWidget(sortAlgorithm_);sort->addWidget(new QLabel("Sort by:"));sort->addWidget(sortField_);sort->addWidget(sortBtn);sort->addStretch();l->addLayout(sort);
    searchTable_=table(5,{"Patient ID","Name","Age","Contact","Category"},w);l->addWidget(searchTable_);
    connect(find,&QPushButton::clicked,this,&MainWindow::searchPatients);
    connect(searchEdit_,&QLineEdit::returnPressed,this,&MainWindow::searchPatients);
    connect(sortBtn,&QPushButton::clicked,this,&MainWindow::sortPatients);
    connect(searchAlgorithm_, &QComboBox::currentTextChanged, this, [this](const QString& mode) {
        if (mode.startsWith("Binary")) searchField_->setCurrentText("Patient ID");
        else if (mode.startsWith("Recursive")) searchField_->setCurrentText("Patient Name");
        else if (mode.startsWith("Category")) searchField_->setCurrentText("Category");
    });
    connect(searchField_, &QComboBox::currentTextChanged, this, [this](const QString& field) {
        if (searchAlgorithm_->currentText().startsWith("Binary") && field != "Patient ID")
            searchAlgorithm_->setCurrentText("Linear Search");
        if (searchAlgorithm_->currentText().startsWith("Recursive") && field != "Patient Name")
            searchAlgorithm_->setCurrentText("Linear Search");
        if (searchAlgorithm_->currentText().startsWith("Category") && field != "Category")
            searchAlgorithm_->setCurrentText("Linear Search");
    });
    return w;
}
QWidget* MainWindow::makeReportsTab() {
    auto* w=new QWidget;auto* l=new QVBoxLayout(w);auto* button=new QPushButton("Generate Reports");reportLabel_=new QLabel("Generate at least three reports: frequent attendees, pending appointments, category distribution and daily appointment summary.");reportLabel_->setWordWrap(true);
    l->addWidget(button);l->addWidget(reportLabel_);l->addStretch();connect(button,&QPushButton::clicked,this,&MainWindow::showReports);return w;
}
void MainWindow::updateStatus(const QString& s){statusLabel_->setText(s);statusBar()->showMessage(s,5000);}
void MainWindow::refreshDashboard() {
    if (!dashboardPatients_) return;
    const auto& patients = manager_.database().patients();
    const auto& appointments = manager_.database().appointments();
    int today = 0, pending = 0;
    const QString currentDate = QDate::currentDate().toString("yyyy-MM-dd");
    for (const auto& appointment : appointments) {
        if (appointment.status() == "Pending") ++pending;
        if (QString::fromStdString(appointment.dateTime()).startsWith(currentDate)) ++today;
    }
    dashboardPatients_->setText(QString::number(patients.size()));
    dashboardToday_->setText(QString::number(today));
    dashboardWaiting_->setText(QString::number(manager_.waitingQueue().size()));
    dashboardPending_->setText(QString::number(pending));
}
void MainWindow::refreshPatients() {
    if(!patientsTable_) return;const auto& ps=manager_.database().patients();patientsTable_->setRowCount((int)ps.size());
    for(int r=0;r<(int)ps.size();++r){auto* p=ps[(size_t)r].get();QStringList vals={QString::fromStdString(p->id()),QString::fromStdString(p->name()),QString::number(p->age()),QString::fromStdString(p->contact()),QString::fromStdString(p->category())};
        for(int c=0;c<5;++c){auto* item=new QTableWidgetItem(vals[c]);if(c==4){if(vals[c]=="Emergency")item->setBackground(QColor("#ffd9d9"));else if(vals[c]=="Follow-Up")item->setBackground(QColor("#fff0c2"));else item->setBackground(QColor("#dff3e4"));}patientsTable_->setItem(r,c,item);}}
    refreshQueue();
    refreshDashboard();
}
void MainWindow::refreshAppointments(){if(!appointmentsTable_)return;auto& as=manager_.database().appointments();appointmentsTable_->setRowCount((int)as.size());for(int r=0;r<(int)as.size();++r){const auto&a=as[(size_t)r];QStringList v={QString::fromStdString(a.id()),QString::fromStdString(a.patientId()),QString::fromStdString(a.dateTime()),QString::fromStdString(a.status())};for(int c=0;c<4;++c){auto*i=new QTableWidgetItem(v[c]);if(c==3){if(v[c]=="Completed")i->setBackground(QColor("#dff3e4"));if(v[c]=="Cancelled")i->setBackground(QColor("#ffd9d9"));}appointmentsTable_->setItem(r,c,i);}} refreshDashboard();}
void MainWindow::refreshQueue(){if(!queueTable_)return;auto q=manager_.waitingQueue();queueTable_->setRowCount(0);int r=0;while(!q.empty()){auto id=q.front();q.pop();auto*p=manager_.findPatientById(id);if(!p)continue;queueTable_->insertRow(r);queueTable_->setItem(r,0,new QTableWidgetItem(QString::number(r+1)));queueTable_->setItem(r,1,new QTableWidgetItem(QString::fromStdString(id)));queueTable_->setItem(r,2,new QTableWidgetItem(QString::fromStdString(p->name())));++r;} refreshDashboard();}
void MainWindow::addPatient(){
    QDialog d(this);d.setWindowTitle("Register Patient");auto*f=new QFormLayout(&d);auto*id=new QLineEdit(&d);auto*name=new QLineEdit(&d);auto*age=new QLineEdit(&d);age->setValidator(new QIntValidator(0,120,age));auto*contact=new QLineEdit(&d);auto*cat=new QComboBox(&d);cat->addItems({"General","Emergency","Follow-Up"});
    f->addRow("Patient ID:",id);f->addRow("Name:",name);f->addRow("Age (0–120):",age);f->addRow("Contact:",contact);f->addRow("Category:",cat);auto*b=new QDialogButtonBox(QDialogButtonBox::Save|QDialogButtonBox::Cancel,&d);f->addRow(b);connect(b,&QDialogButtonBox::accepted,&d,&QDialog::accept);connect(b,&QDialogButtonBox::rejected,&d,&QDialog::reject);
    if(d.exec()!=QDialog::Accepted)return;if(id->text().trimmed().isEmpty()||name->text().trimmed().isEmpty()||age->text().isEmpty()){QMessageBox::warning(this,"Invalid input","Patient ID, name and age are required.");return;}
    try{std::unique_ptr<PatientRecord> p;auto i=id->text().trimmed().toStdString(),n=name->text().trimmed().toStdString(),c=contact->text().trimmed().toStdString();int a=age->text().toInt();
        if(cat->currentText()=="General")p=std::make_unique<GeneralPatient>(i,n,a,c);else if(cat->currentText()=="Emergency")p=std::make_unique<EmergencyPatient>(i,n,a,c);else p=std::make_unique<FollowUpPatient>(i,n,a,c);
        if(!manager_.addPatient(std::move(p)))throw std::invalid_argument("Patient ID already exists.");refreshPatients();updateStatus("Patient registered.");}
    catch(const std::exception&e){QMessageBox::warning(this,"Patient error",e.what());}
}
void MainWindow::editPatient(){
    int r=selected(patientsTable_);if(r<0){QMessageBox::warning(this,"Select patient","Select a patient record first.");return;}auto id=cell(patientsTable_,r,0).toStdString();auto*p=manager_.findPatientById(id);if(!p)return;
    QDialog d(this);d.setWindowTitle("Edit Patient");auto*f=new QFormLayout(&d);auto*n=new QLineEdit(QString::fromStdString(p->name()));auto*a=new QLineEdit(QString::number(p->age()));a->setValidator(new QIntValidator(0,120,a));auto*c=new QLineEdit(QString::fromStdString(p->contact()));
    f->addRow("Patient ID:",new QLabel(QString::fromStdString(id)));f->addRow("Name:",n);f->addRow("Age:",a);f->addRow("Contact:",c);auto*b=new QDialogButtonBox(QDialogButtonBox::Save|QDialogButtonBox::Cancel);f->addRow(b);connect(b,&QDialogButtonBox::accepted,&d,&QDialog::accept);connect(b,&QDialogButtonBox::rejected,&d,&QDialog::reject);if(d.exec()!=QDialog::Accepted)return;
    if(n->text().trimmed().isEmpty() || a->text().trimmed().isEmpty()) {
        QMessageBox::warning(this,"Invalid input","Patient name and age are required.");
        return;
    }
    try{manager_.editPatient(id,n->text().trimmed().toStdString(),a->text().toInt(),c->text().trimmed().toStdString());refreshPatients();updateStatus("Patient updated.");}catch(const std::exception&e){QMessageBox::warning(this,"Patient error",e.what());}
}
void MainWindow::deletePatient(){int r=selected(patientsTable_);if(r<0){QMessageBox::warning(this,"Select patient","Select a patient record first.");return;}auto id=cell(patientsTable_,r,0);if(QMessageBox::question(this,"Delete patient","Delete "+id+"?")==QMessageBox::Yes){if(manager_.deletePatient(id.toStdString())){refreshPatients();refreshAppointments();updateStatus("Patient deleted. Use Undo to reverse.");}}}
void MainWindow::scheduleAppointment(){
    if(manager_.database().patients().empty()){QMessageBox::warning(this,"No patients","Register a patient before scheduling an appointment.");return;}
    QDialog d(this);d.setWindowTitle("Schedule Appointment");auto*f=new QFormLayout(&d);auto*id=new QLineEdit;auto*pid=new QComboBox;for(auto&p:manager_.database().patients())pid->addItem(QString::fromStdString(p->id())+" — "+QString::fromStdString(p->name()),QString::fromStdString(p->id()));
    auto*dt=new QDateTimeEdit(QDateTime::currentDateTime());dt->setCalendarPopup(true);dt->setDisplayFormat("yyyy-MM-dd HH:mm");f->addRow("Appointment ID:",id);f->addRow("Patient:",pid);f->addRow("Date & time:",dt);auto*b=new QDialogButtonBox(QDialogButtonBox::Save|QDialogButtonBox::Cancel);f->addRow(b);connect(b,&QDialogButtonBox::accepted,&d,&QDialog::accept);connect(b,&QDialogButtonBox::rejected,&d,&QDialog::reject);if(d.exec()!=QDialog::Accepted)return;
    try{Appointment a(id->text().trimmed().toStdString(),pid->currentData().toString().toStdString(),dt->dateTime().toString("yyyy-MM-dd HH:mm").toStdString());if(!manager_.addAppointment(a))throw std::invalid_argument("Appointment ID already exists.");refreshAppointments();updateStatus("Appointment scheduled.");}catch(const std::exception&e){QMessageBox::warning(this,"Appointment error",e.what());}
}
void MainWindow::cancelAppointment(){int r=selected(appointmentsTable_);if(r<0){QMessageBox::warning(this,"Select appointment","Select an appointment first.");return;}auto id=cell(appointmentsTable_,r,0);if(QMessageBox::question(this,"Cancel appointment","Cancel appointment "+id+"?")==QMessageBox::Yes){auto*a=manager_.database().findAppointment(id.toStdString());if(a)manager_.setAppointmentStatus(id.toStdString(),"Cancelled");refreshAppointments();updateStatus("Appointment cancelled. Use Undo to reverse.");}}
void MainWindow::changeAppointmentStatus(){int r=selected(appointmentsTable_);if(r<0){QMessageBox::warning(this,"Select appointment","Select an appointment first.");return;}QStringList opts={"Pending","Completed","Cancelled"};bool ok=false;QString s=QInputDialog::getItem(this,"Appointment status","Status:",opts,0,false,&ok);if(ok)try{manager_.setAppointmentStatus(cell(appointmentsTable_,r,0).toStdString(),s.toStdString());refreshAppointments();}catch(const std::exception&e){QMessageBox::warning(this,"Status error",e.what());}}
void MainWindow::addToQueue(){QStringList ids;for(auto&p:manager_.database().patients())ids<<QString::fromStdString(p->id())+" — "+QString::fromStdString(p->name());if(ids.isEmpty()){QMessageBox::information(this,"Queue","Register a patient first.");return;}bool ok=false;auto choice=QInputDialog::getItem(this,"Add to queue","Patient:",ids,0,false,&ok);if(!ok)return;auto id=choice.section(" — ",0,0);try{manager_.enqueue(id.toStdString());refreshQueue();updateStatus("Patient added to waiting queue.");}catch(const std::exception&e){QMessageBox::warning(this,"Queue error",e.what());}}
void MainWindow::callNext(){std::string id;if(!manager_.callNext(id)){QMessageBox::information(this,"Waiting queue","No patients are waiting.");return;}auto*p=manager_.findPatientById(id);QMessageBox::information(this,"Next patient",QString("Call %1 (%2) for consultation.").arg(QString::fromStdString(p?p->name():"Unknown")).arg(QString::fromStdString(id)));refreshQueue();}
void MainWindow::searchPatients(){
    const std::string query = searchEdit_->text().trimmed().toStdString();
    if(query.empty()){
        QMessageBox::warning(this,"Search","Enter a search value.");
        return;
    }

    auto& records = manager_.database().patients();
    std::vector<PatientRecord*> found;
    const QString mode = searchAlgorithm_->currentText();
    const QString field = searchField_->currentText();
    const auto normalize = [](std::string value) {
        for (char& c : value) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        return value;
    };

    if (mode.startsWith("Binary")) {
        // Binary search is valid only for Patient ID and sorted records.
        SearchEngine::mergeSort(records, "Patient ID");
        if (auto* patient = SearchEngine::binaryById(records, query))
            found.push_back(patient);
    } else if (mode.startsWith("Recursive")) {
        if (auto* patient = SearchEngine::recursiveByName(records, query))
            found.push_back(patient);
    } else if (mode.startsWith("Category") || field == "Category") {
        const std::string wanted = normalize(query);
        for (const auto& patient : records)
            if (normalize(patient->category()) == wanted) found.push_back(patient.get());
    } else if (field == "Patient Name") {
        const std::string needle = normalize(query);
        for (const auto& patient : records)
            if (normalize(patient->name()).find(needle) != std::string::npos)
                found.push_back(patient.get());
    } else {
        if (auto* patient = SearchEngine::linearById(records, query))
            found.push_back(patient);
    }

    for (auto* patient : found) manager_.refreshCache(patient);
    searchTable_->setRowCount(static_cast<int>(found.size()));
    for(int i=0;i<static_cast<int>(found.size());++i){
        auto* p=found[static_cast<std::size_t>(i)];
        QStringList v={QString::fromStdString(p->id()),QString::fromStdString(p->name()),
                       QString::number(p->age()),QString::fromStdString(p->contact()),
                       QString::fromStdString(p->category())};
        for(int c=0;c<5;++c) searchTable_->setItem(i,c,new QTableWidgetItem(v[c]));
    }
    updateStatus(QString("Search complete: %1 result(s).").arg(found.size()));
}

void MainWindow::sortPatients(){auto& r=manager_.database().patients();if(sortAlgorithm_->currentText()=="Insertion Sort")SearchEngine::insertionSort(r,sortField_->currentText().toStdString());else SearchEngine::mergeSort(r,sortField_->currentText().toStdString());refreshPatients();updateStatus("Records sorted using "+sortAlgorithm_->currentText()+".");}
void MainWindow::showReports(){
    const auto& patients = manager_.database().patients();
    const auto& appointments = manager_.database().appointments();
    int general=0, emergency=0, followUp=0, pending=0;
    int todayTotal=0, todayPending=0, todayCompleted=0, todayCancelled=0;
    const QString today = QDate::currentDate().toString("yyyy-MM-dd");
    struct Attendance { std::string id; std::string name; int count=0; };
    std::vector<Attendance> attendance;
    for (const auto& patient : patients) {
        if (patient->category()=="General") ++general;
        else if (patient->category()=="Emergency") ++emergency;
        else ++followUp;
        attendance.push_back({patient->id(), patient->name(), 0});
    }
    QStringList pendingPatients;
    for (const auto& appointment : appointments) {
        if (appointment.status()=="Pending") {
            ++pending;
            auto* patient = manager_.findPatientById(appointment.patientId());
            if (patient) pendingPatients << QString("%1 (%2) — %3")
                .arg(QString::fromStdString(patient->name()))
                .arg(QString::fromStdString(patient->id()))
                .arg(QString::fromStdString(appointment.dateTime()));
        } else if (appointment.status()=="Completed") {
            for (auto& item : attendance)
                if (item.id == appointment.patientId()) { ++item.count; break; }
        }
        if (QString::fromStdString(appointment.dateTime()).startsWith(today)) {
            ++todayTotal;
            if (appointment.status()=="Pending") ++todayPending;
            else if (appointment.status()=="Completed") ++todayCompleted;
            else if (appointment.status()=="Cancelled") ++todayCancelled;
        }
    }

    int highest = 0;
    for (const auto& item : attendance) highest = std::max(highest, item.count);
    QStringList mostFrequent;
    if (highest > 0) {
        for (const auto& item : attendance)
            if (item.count == highest)
                mostFrequent << QString("%1 (%2): %3 completed appointment(s)")
                    .arg(QString::fromStdString(item.name))
                    .arg(QString::fromStdString(item.id)).arg(item.count);
    } else {
        mostFrequent << "No completed appointments recorded.";
    }
    if (pendingPatients.isEmpty()) pendingPatients << "No pending appointments.";

    reportLabel_->setText(
        QString("REPORT 1 — MOST FREQUENTLY ATTENDED PATIENTS\n%1\n\n"
                "REPORT 2 — PATIENTS WITH PENDING APPOINTMENTS\n%2\n\n"
                "REPORT 3 — PATIENT CATEGORY DISTRIBUTION\n"
                "General: %3\nEmergency: %4\nFollow-Up: %5\n\n"
                "REPORT 4 — DAILY APPOINTMENT SUMMARY (%6)\n"
                "Total: %7 | Completed: %8 | Pending: %9 | Cancelled: %10")
        .arg(mostFrequent.join("\n"))
        .arg(pendingPatients.join("\n"))
        .arg(general).arg(emergency).arg(followUp).arg(today)
        .arg(todayTotal).arg(todayCompleted).arg(todayPending).arg(todayCancelled));
}

void MainWindow::saveData(){QString path=QFileDialog::getSaveFileName(this,"Save clinic data","SCAPMS_data.txt","SCAPMS data (*.txt)");if(path.isEmpty())return;try{FileHandler::save(manager_.database(),path);updateStatus("Clinic data saved.");}catch(const std::exception&e){QMessageBox::critical(this,"Save failed",e.what());}}
void MainWindow::loadData(){QString path=QFileDialog::getOpenFileName(this,"Load clinic data",QString(),"SCAPMS data (*.txt)");if(path.isEmpty())return;try{FileHandler::load(manager_.database(),path);manager_.resetTransientState();refreshPatients();refreshAppointments();refreshQueue();updateStatus("Clinic data loaded.");}catch(const std::exception&e){QMessageBox::critical(this,"Load failed",e.what());}}
void MainWindow::undoLast(){
    try {
        if(!manager_.undo()){
            QMessageBox::information(this,"Undo","There are no administrative operations to undo.");
            return;
        }
        refreshPatients();
        refreshAppointments();
        refreshQueue();
        updateStatus("Last administrative operation undone.");
    } catch (const std::exception& error) {
        QMessageBox::warning(this,"Undo failed",error.what());
    }
}
}
