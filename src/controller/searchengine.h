#pragma once
#include "model/patientrecord.h"
#include <vector>
#include <string>
namespace clinic {
class SearchEngine {
public:
    static PatientRecord* linearById(const std::vector<std::unique_ptr<PatientRecord>>& records, const std::string& id);
    static PatientRecord* binaryById(const std::vector<std::unique_ptr<PatientRecord>>& records, const std::string& id);
    static PatientRecord* recursiveByName(const std::vector<std::unique_ptr<PatientRecord>>& records, const std::string& name, std::size_t index = 0);
    static void insertionSort(std::vector<std::unique_ptr<PatientRecord>>& records, const std::string& field);
    static void mergeSort(std::vector<std::unique_ptr<PatientRecord>>& records, const std::string& field);
private:
    static bool less(const PatientRecord& a, const PatientRecord& b, const std::string& field);
    static void mergeSortRange(std::vector<std::unique_ptr<PatientRecord>>& records, std::vector<std::unique_ptr<PatientRecord>>& temp,
                               std::size_t left, std::size_t right, const std::string& field);
};
}
