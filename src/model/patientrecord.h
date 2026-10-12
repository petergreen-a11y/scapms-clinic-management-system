#pragma once
#include <string>
#include <memory>
namespace clinic {
class PatientRecord {
public:
    PatientRecord(std::string id, std::string name, int age, std::string contact);
    virtual ~PatientRecord() = default;
    const std::string& id() const { return id_; }
    const std::string& name() const { return name_; }
    int age() const { return age_; }
    const std::string& contact() const { return contact_; }
    void setName(const std::string& value);
    void setAge(int value);
    void setContact(const std::string& value) { contact_ = value; }
    virtual std::string category() const = 0;
    virtual std::string careDescription() const = 0;
    virtual std::unique_ptr<PatientRecord> clone() const = 0;
private:
    std::string id_, name_;
    int age_;
    std::string contact_;
};
}
