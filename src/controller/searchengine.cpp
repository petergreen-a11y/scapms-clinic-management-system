#include "controller/searchengine.h"
#include <algorithm>
#include <cctype>
namespace clinic {
static std::string lower(std::string s) {
    for (char& c : s) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    return s;
}
PatientRecord* SearchEngine::linearById(const std::vector<std::unique_ptr<PatientRecord>>& r, const std::string& id) {
    for (const auto& p : r) if (p->id() == id) return p.get();
    return nullptr;
}
PatientRecord* SearchEngine::binaryById(const std::vector<std::unique_ptr<PatientRecord>>& r, const std::string& id) {
    std::size_t lo=0, hi=r.size();
    while (lo < hi) {
        std::size_t mid = lo + (hi-lo)/2;
        if (r[mid]->id() == id) return r[mid].get();
        if (r[mid]->id() < id) lo=mid+1; else hi=mid;
    }
    return nullptr;
}
PatientRecord* SearchEngine::recursiveByName(const std::vector<std::unique_ptr<PatientRecord>>& r, const std::string& name, std::size_t i) {
    if (i >= r.size()) return nullptr;
    if (lower(r[i]->name()).find(lower(name)) != std::string::npos) return r[i].get();
    return recursiveByName(r, name, i+1);
}
bool SearchEngine::less(const PatientRecord& a, const PatientRecord& b, const std::string& field) {
    if (field == "Age") return a.age() < b.age();
    if (field == "Patient ID") return a.id() < b.id();
    return lower(a.name()) < lower(b.name());
}
void SearchEngine::insertionSort(std::vector<std::unique_ptr<PatientRecord>>& r, const std::string& field) {
    for (std::size_t i=1; i<r.size(); ++i) {
        auto key = std::move(r[i]);
        std::size_t j=i;
        while (j>0 && less(*key, *r[j-1], field)) { r[j] = std::move(r[j-1]); --j; }
        r[j] = std::move(key);
    }
}
void SearchEngine::mergeSort(std::vector<std::unique_ptr<PatientRecord>>& r, const std::string& field) {
    if (r.size()<2) return;
    std::vector<std::unique_ptr<PatientRecord>> temp(r.size());
    mergeSortRange(r,temp,0,r.size(),field);
}
void SearchEngine::mergeSortRange(std::vector<std::unique_ptr<PatientRecord>>& r, std::vector<std::unique_ptr<PatientRecord>>& t,
                                  std::size_t l, std::size_t h, const std::string& field) {
    if (h-l<2) return;
    std::size_t m=l+(h-l)/2;
    mergeSortRange(r,t,l,m,field); mergeSortRange(r,t,m,h,field);
    std::size_t i=l,j=m,k=l;
    while(i<m && j<h) {
        if (!less(*r[j],*r[i],field)) t[k++]=std::move(r[i++]);
        else t[k++]=std::move(r[j++]);
    }
    while(i<m) t[k++]=std::move(r[i++]);
    while(j<h) t[k++]=std::move(r[j++]);
    for(k=l;k<h;++k) r[k]=std::move(t[k]);
}
}
