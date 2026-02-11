#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <fstream>
#include <iomanip>
#include <ctime>

using namespace std;

// ---------- Helpers ----------
string toLower(string s) {
    for (char &c : s) c = (char)tolower((unsigned char)c);
    return s;
}

string nowTimeString() {
    time_t t = time(nullptr);
    tm *lt = localtime(&t);
    char buf[64];
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", lt);
    return string(buf);
}

bool contains(const vector<int>& v, int x) {
    return find(v.begin(), v.end(), x) != v.end();
}

// ---------- Domain model ----------
struct Symptom {
    int id;
    string name;
};

class Disease {
private:
    string name;
    vector<int> symptomIds;     // which symptoms belong to this disease
    string advice;              // what to do / treatment info
    string danger;              // red flags / when to see doctor urgently
public:
    Disease() = default;
    Disease(string n, vector<int> sIds, string adv, string dang)
        : name(std::move(n)), symptomIds(std::move(sIds)), advice(std::move(adv)), danger(std::move(dang)) {}

    const string& getName() const { return name; }
    const vector<int>& getSymptomIds() const { return symptomIds; }
    const string& getAdvice() const { return advice; }
    const string& getDanger() const { return danger; }

    // score = how many symptoms match
    int score(const vector<int>& chosen) const {
        int cnt = 0;
        for (int sid : symptomIds) if (contains(chosen, sid)) cnt++;
        return cnt;
    }

    double percentMatch(const vector<int>& chosen) const {
        if (symptomIds.empty()) return 0.0;
        return 100.0 * score(chosen) / (double)symptomIds.size();
    }
};

struct Result {
    Disease disease;
    int matchedCount;
    double percent;
};

class DiagnosticSystem {
private:
    vector<Symptom> symptoms;
    vector<Disease> diseases;
    vector<int> chosenSymptoms;

public:
    DiagnosticSystem() {
        seedSymptoms();
        seedDiseases();
    }

    void run() {
        cout << "Program started\n";
        while (true) {
            cout << "\n=============================\n";
            cout << "  MEDI-SYMPTOM CHECKER (C++)\n";
            cout << "=============================\n";
            cout << "1) Show symptoms list\n";
            cout << "2) Add symptom (by number)\n";
            cout << "3) Remove symptom (by number)\n";
            cout << "4) Show chosen symptoms\n";
            cout << "5) Diagnose (show possible diseases)\n";
            cout << "6) Clear chosen symptoms\n";
            cout << "7) Save chosen symptoms to file\n";
            cout << "0) Exit\n";
            cout << "Choose: ";

            int choice;
            if (!(cin >> choice)) {
                cin.clear();
                cin.ignore(10000, '\n');
                cout << "Invalid input.\n";
                continue;
            }

            switch (choice) {
                case 1: showSymptoms(); break;
                case 2: addSymptom(); break;
                case 3: removeSymptom(); break;
                case 4: showChosen(); break;
                case 5: diagnose(); break;
                case 6: clearChosen(); break;
                case 7: saveToFile(); break;
                case 0:
                    cout << "Bye!\n";
                    return;
                default:
                    cout << "Unknown option.\n";
            }
        }
    }

private:
    void seedSymptoms() {
        symptoms = {
            {1, "Fever (high temperature)"},
            {2, "Cough"},
            {3, "Sore throat"},
            {4, "Runny / stuffy nose"},
            {5, "Headache"},
            {6, "Fatigue / weakness"},
            {7, "Muscle aches"},
            {8, "Nausea"},
            {9, "Vomiting"},
            {10, "Diarrhea"},
            {11, "Shortness of breath"},
            {12, "Chest pain"},
            {13, "Loss of taste/smell"},
            {14, "Skin rash"},
            {15, "Stomach pain"},
            {16, "Burning when urinating"},
            {17, "Frequent urination"},
            {18, "Dizziness"},
            {19, "Allergic itching/sneezing"},
            {20, "Eye redness/watery eyes"}
        };
    }

    void seedDiseases() {
        diseases = {
            Disease(
                "Common cold",
                {2,3,4,5,6},
                "Rest, warm drinks, saline nasal spray, mild pain reliever if needed. Usually improves in 3–7 days.",
                "See a doctor if symptoms last >10 days, fever is high, or condition worsens."
            ),
            Disease(
                "Flu (influenza)",
                {1,2,5,6,7},
                "Rest, hydration, fever control. If severe/high risk, antiviral treatment may be needed (doctor).",
                "Urgent if breathing problems, chest pain, confusion, or very high fever."
            ),
            Disease(
                "COVID-like viral infection",
                {1,2,6,11,13},
                "Rest, hydration. Consider testing and isolation if recommended. Treat fever/symptoms.",
                "Urgent if shortness of breath, chest pain, blue lips, or worsening condition."
            ),
            Disease(
                "Food poisoning / gastroenteritis",
                {8,9,10,15,6},
                "Oral rehydration (electrolytes), light food, rest. Avoid fatty/spicy foods.",
                "Urgent if blood in stool/vomit, severe dehydration, or >2 days severe symptoms."
            ),
            Disease(
                "Allergy (rhinitis/conjunctivitis)",
                {4,19,20},
                "Avoid allergen, antihistamine (if allowed), rinse nose, keep room clean.",
                "Urgent if swelling of face/throat, difficulty breathing (possible anaphylaxis)."
            ),
            Disease(
                "Urinary tract infection (UTI)",
                {16,17,1,15},
                "Drink water. Often needs antibiotics from a doctor. Do not delay if symptoms persist.",
                "Urgent if strong back pain, high fever, chills (possible kidney infection)."
            ),
            Disease(
                "Dermatitis / skin reaction",
                {14,19},
                "Avoid irritant/allergen, moisturize, mild anti-itch cream. If spreading—doctor.",
                "Urgent if rash with fever, blistering, face swelling, or breathing difficulty."
            ),
            Disease(
                "Possible cardiac emergency",
                {12,11,18},
                "Call emergency services. Do not self-treat.",
                "Chest pain + shortness of breath/dizziness can be dangerous. Seek urgent help."
            )
        };
    }

    void showSymptoms() const {
        cout << "\n--- Symptoms list ---\n";
        for (const auto& s : symptoms) {
            cout << setw(2) << s.id << ") " << s.name << "\n";
        }
    }

    void addSymptom() {
        int id;
        cout << "Enter symptom number to add: ";
        cin >> id;

        if (!existsSymptom(id)) {
            cout << "No such symptom.\n";
            return;
        }
        if (contains(chosenSymptoms, id)) {
            cout << "Already added.\n";
            return;
        }
        chosenSymptoms.push_back(id);
        cout << "Added.\n";
    }

    void removeSymptom() {
        int id;
        cout << "Enter symptom number to remove: ";
        cin >> id;

        auto it = find(chosenSymptoms.begin(), chosenSymptoms.end(), id);
        if (it == chosenSymptoms.end()) {
            cout << "That symptom is not in your chosen list.\n";
            return;
        }
        chosenSymptoms.erase(it);
        cout << "Removed.\n";
    }

    void showChosen() const {
        cout << "\n--- Your chosen symptoms ---\n";
        if (chosenSymptoms.empty()) {
            cout << "(none)\n";
            return;
        }
        for (int sid : chosenSymptoms) {
            cout << sid << ") " << symptomName(sid) << "\n";
        }
    }

    void clearChosen() {
        chosenSymptoms.clear();
        cout << "Cleared.\n";
    }

    void diagnose() const {
        cout << "\n=== Diagnosis results ==\n";
        if (chosenSymptoms.empty()) {
            cout << "You have not selected any symptoms.\n";
            return;
        }

        vector<Result> results;
        for (const auto& d : diseases) {
            int sc = d.score(chosenSymptoms);
            if (sc > 0) {
                results.push_back({d, sc, d.percentMatch(chosenSymptoms)});
            }
        }

        if (results.empty()) {
            cout << "No matches found. Try adding more symptoms.\n";
            return;
        }

        sort(results.begin(), results.end(), [](const Result& a, const Result& b) {
            if (a.matchedCount != b.matchedCount) return a.matchedCount > b.matchedCount;
            return a.percent > b.percent;
        });

        cout << "Matched diseases (top first):\n\n";
        int rank = 1;
        for (const auto& r : results) {
            cout << rank++ << ") " << r.disease.getName()
                 << "  | matched: " << r.matchedCount
                 << "  | approx fit: " << fixed << setprecision(1) << r.percent << "%\n";
            cout << "   Advice: " << r.disease.getAdvice() << "\n";
            cout << "   Danger: " << r.disease.getDanger() << "\n\n";
        }

        cout << "NOTE: This program is educational and does NOT replace a doctor.\n";
    }

    void saveToFile() const {
        ofstream out("history.txt", ios::app);
        if (!out) {
            cout << "Failed to open history.txt\n";
            return;
        }

        out << "---- " << nowTimeString() << " ----\n";
        if (chosenSymptoms.empty()) {
            out << "(no symptoms selected)\n\n";
            cout << "Saved empty selection.\n";
            return;
        }

        out << "Symptoms:\n";
        for (int sid : chosenSymptoms) {
            out << " - " << sid << ") " << symptomName(sid) << "\n";
        }
        out << "\n";
        cout << "Saved to history.txt\n";
    }

    bool existsSymptom(int id) const {
        for (const auto& s : symptoms) if (s.id == id) return true;
        return false;
    }

    string symptomName(int id) const {
        for (const auto& s : symptoms) if (s.id == id) return s.name;
        return "Unknown";
    }
};

// ---------- Main ----------
int main() {
   // ios::sync_with_stdio(false);
   // cin.tie(nullptr);

    DiagnosticSystem app;
    app.run();
    return 0;
}