#include "utils/filehandler.h"
#include "model/generalpatient.h"
#include "model/emergencypatient.h"
#include "model/followuppatient.h"
#include <QFile>
#include <QDateTime>
#include <QTextStream>
#include <QStringList>
#include <stdexcept>
namespace clinic {
static QString enc(const std::string& s) { QString v=QString::fromStdString(s); v.replace("%","%25"); v.replace("|","%7C"); v.replace("\n","%0A"); return v; }
static std::string dec(QString s) { s.replace("%0A","\n"); s.replace("%7C","|"); s.replace("%25","%"); return s.toStdString(); }
void FileHandler::save(const ClinicDatabase& db, const QString& path) {
    QFile f(path);
    if (!f.open(QIODevice::WriteOnly|QIODevice::Text)) throw std::runtime_error("Could not open the file for writing.");
    QTextStream out(&f);
    out << "SCAPMS1\n";
    for (const auto& p : db.patients())
        out << "P|" << enc(p->id()) << "|" << enc(p->name()) << "|" << p->age() << "|" << enc(p->contact()) << "|" << enc(p->category()) << "\n";
    for (const auto& a : db.appointments())
        out << "A|" << enc(a.id()) << "|" << enc(a.patientId()) << "|" << enc(a.dateTime()) << "|" << enc(a.status()) << "\n";
    if (out.status()!=QTextStream::Ok) throw std::runtime_error("An error occurred while saving the file.");
}
void FileHandler::load(ClinicDatabase& db, const QString& path) {
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly|QIODevice::Text)) throw std::runtime_error("Could not open the selected file.");
    QTextStream in(&f);
    if (in.readLine() != "SCAPMS1") throw std::runtime_error("The selected file is not a valid SCAPMS data file.");
    ClinicDatabase temp;
    while (!in.atEnd()) {
        QString line=in.readLine();
        if (line.trimmed().isEmpty()) continue;
        QStringList x=line.split('|');
        if (x[0]=="P" && x.size()==6) {
            std::string id=dec(x[1]), name=dec(x[2]), contact=dec(x[4]), cat=dec(x[5]);
            bool ageOk = false;
            int age=x[3].toInt(&ageOk);
            if (!ageOk) throw std::runtime_error("Invalid patient age in data file.");
            std::unique_ptr<PatientRecord> p;
            if(cat=="General") p=std::make_unique<GeneralPatient>(id,name,age,contact);
            else if(cat=="Emergency") p=std::make_unique<EmergencyPatient>(id,name,age,contact);
            else if(cat=="Follow-Up") p=std::make_unique<FollowUpPatient>(id,name,age,contact);
            else throw std::runtime_error("Unknown patient category in file.");
            if(!temp.addPatient(std::move(p))) throw std::runtime_error("Duplicate patient ID in file.");
        } else if (x[0]=="A" && x.size()==5) {
            const std::string appointmentId = dec(x[1]);
            const std::string patientId = dec(x[2]);
            const std::string dateTime = dec(x[3]);
            const QString parsedDateTime = QString::fromStdString(dateTime);
            if (!QDateTime::fromString(parsedDateTime, "yyyy-MM-dd HH:mm").isValid())
                throw std::runtime_error("Invalid appointment date/time in data file.");
            Appointment a(appointmentId,patientId,dateTime,dec(x[4]));
            if(!temp.addAppointment(a)) throw std::runtime_error("Duplicate appointment ID in file.");
        } else throw std::runtime_error("Malformed line in data file.");
    }
    db = std::move(temp);
}
}
