//make sure wkhtmltopdf is installed on your laptop before running 
#define _CRT_SECURE_NO_WARNINGS
#include <iostream> //input and output
#include <fstream> //file read and write
#include <sstream> // string stream operations
#include <string> //string handling
#include <cctype> //character checks 
#include <algorithm> //algorithms like remove or transform
#include <conio.h> //hidden input for passwords
#include <iomanip> //output formatting 
#include <ctime> //get current date and time
#include <windows.h>
#include <unordered_map>
#include <vector>
#ifdef _WIN32
#endif
#define COLOR_RESET   "\033[0m"
#define COLOR_RED     "\033[31m"
#define COLOR_GREEN   "\033[32m"
#define COLOR_YELLOW  "\033[33m"
#define COLOR_BLUE    "\033[34m"
#define COLOR_MAGENTA "\033[35m"
#define COLOR_CYAN    "\033[36m"
#define COLOR_WHITE   "\033[37m"


void setConsoleColor(int color) {
#ifdef _WIN32
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, color);
#endif
}

void resetConsoleColor() {
#ifdef _WIN32
    setConsoleColor(7); // Default color
#else
    cout << COLOR_RESET;
#endif
}
using namespace std;
int lastSavedDonationIndex = 0;
struct User {
    int userID;
    string firstName;
    string lastName;
    string password;
    string email;
    string phone;
};
struct Charity {
    int charityID;
    string name;
    string description;
    double targetAmount;
    double currentAmount;
    string deadline;
    string status;
};
struct Donation {
    int donationID;
    int userID;
    int charityID;
    double amount;
    string message;
    string dateTime; 
};
unordered_map<string, int> emailToUserIndex;
const int MAX_USERS = 200;//This program uses a fixed number of users 200 You can modify this number as needed
const int MAX_CHARITIES = 200;//fixed number of charities 200 
const int MAX_DONATIONS = 200;//fixed number of donation 200
vector<User> users;
vector<Charity> charities;
vector<Donation> donations;
void browseCharities();
bool loginUser();
void registerUser();
void adminMenu();
void generateStyledHTMLReport();
void searchCharities();
void logUserDonation(const Donation& d, const User& user, const Charity& charity);
string getCurrentDateTimeString() {
    time_t now = time(0);
    tm* ltm = localtime(&now);
    char buffer[20];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", ltm);
    return string(buffer);
}
bool isValidPassword(const string& pwd) {
    if (pwd.length() < 8) return false;
    bool hasDigit = false, hasLetter = false, hasSpecial = false;
    for (char c : pwd) {
        if (isdigit(c)) hasDigit = true;
        else if (isalpha(c)) hasLetter = true;
        else if (ispunct(c)) hasSpecial = true;
    }
    return hasDigit && hasLetter && hasSpecial;
}
string getHiddenPassword() {
    string password;
    char ch;
    while ((ch = _getch()) != '\r') { 
        if (ch == '\b') { 
            if (!password.empty()) {
                cout << "\b \b"; 
                password.pop_back(); 
            }
        }
        else {
            password += ch;
            cout << '*';
        }
    }
    cout << endl;
    return password;
}
bool isValidEmail(const string& email) {
    size_t atPos = email.find('@');
    size_t dotPos = email.find('.', atPos);
    return atPos != string::npos && dotPos != string::npos && atPos < dotPos;
}
bool isValidPhone(const string& phone) {
    if (phone.length() != 9) return false;
    if (phone[2] != '-') return false;
    string prefix = phone.substr(0, 2);
    string allowedPrefixes[] = { "03", "70", "71", "76", "78", "79", "81" ,"08" ,"06" ,"07" ,"04" ,"01", "28" };
    bool validPrefix = false;
    for (const string& p : allowedPrefixes) {
        if (prefix == p) {
            validPrefix = true;
            break;
        }
    }
    if (!validPrefix) return false;
    for (int i = 0; i < phone.length(); i++) {
        if (i == 2) continue;
        if (!isdigit(phone[i])) return false;
    }

    return true;
}
void saveUserToFile(User user) {
    ofstream file("users.txt", ios::app);
    if (!file) {
        cout << "Error saving user.\n";
        return;
    }
    file << user.userID << " "
        << user.firstName << " "
        << user.lastName << " "
        << user.email << " "
        << user.password << " "
        << user.phone << endl;
    file.close();
}
int generateUserID() {
    int lastID = 0;
    ifstream file("users.txt");
    string line;
    while (getline(file, line)) {
        if (line.empty()) continue;
        stringstream ss(line);
        int id;
        ss >> id;
        if (id > lastID)
            lastID = id;
    }
    file.close();
    return lastID + 1;
}
void loadUsersFromFile() {
    ifstream file("users.txt");
    while (file) {
        User user;
        file >> user.userID >>
            user.firstName >>
            user.lastName >>
            user.email >>
            user.password >>
            user.phone;
        if (file.fail()) break;
        users.push_back(user);
        emailToUserIndex[user.email] = users.size() - 1;
    }
    file.close();
}
void loadCharitiesFromFile() {
    ifstream file("charities.txt");
    if (!file) return;
    charities.clear(); 

    while (file) {
        Charity c;
        string word1, word2;
        file >> c.charityID >> word1 >> word2;
        c.name = word1 + " " + word2;

        string temp, desc = "";
        while (file >> temp) {
            if (isdigit(temp[0])) break;
            desc += temp + " ";
        }
        if (!desc.empty()) desc.pop_back();
        c.description = desc;

        temp.erase(remove(temp.begin(), temp.end(), ','), temp.end());
        if (!temp.empty() && temp.back() == '$') {
            temp.pop_back();
        }
        if (!temp.empty()) {
            c.targetAmount = stod(temp);
        }
        else {
            c.targetAmount = 0;
        }
        file >> temp;
        temp.erase(remove(temp.begin(), temp.end(), ','), temp.end());
        if (!temp.empty() && temp.back() == '$') {
            temp.pop_back();
        }
        if (!temp.empty()) {
            c.targetAmount = stod(temp);
        }
        else {
            c.targetAmount = 0; 
        }

        file >> c.deadline;
        getline(file, c.status);
        if (!c.status.empty() && c.status[0] == ' ')
            c.status.erase(0, 1);

        charities.push_back(c);
    }
    file.close();
}
void saveCharitiesToFile() {
    ofstream file("charities.txt");
    if (!file) {
        cout << "Error opening charities.txt\n";
        return;
    }

    for (const auto& c : charities) {
        file << c.charityID << "|"
            << c.name << "|"
            << c.description << "|"
            << c.targetAmount << "|"
            << c.currentAmount << "|"
            << c.deadline << "|"
            << c.status << "\n";
    }

    file.close();
}
bool isCharityNameExists(const string& name) {
    for (int i = 0; i < charities.size(); i++) {
        if (charities[i].name == name)
            return true;
    }
    return false;
}
void addCharity() {
    Charity c;//create a new charity object
    c.charityID = charities.empty() ? 1 : charities.back().charityID + 1;
    cin.ignore(); 
    cout << "Enter charity name: ";
    getline(cin, c.name);
    cout << "Enter description: ";
    getline(cin, c.description);
    cout << "Enter target amount: ";
    cin >> c.targetAmount;
    cout << "Enter current amount: ";
    cin >> c.currentAmount;
    cin.ignore();
    string date, time;//store date and time
    cout << "Enter deadline date: ";
    getline(cin, date);
    cout << "Enter deadline time: ";
    getline(cin, time);
    c.deadline = date + "," + time; 
    cout << "Enter status: ";
    getline(cin, c.status);//add the charity status
    charities.push_back(c);
    saveCharitiesToFile();//save charities to file
    cout << "Charity added successfully!\n"; 
}
void removeCharity() {
    cin.ignore();
    string name;
    cout << "Enter the name of charity to remove: ";
    getline(cin, name);
    for (int i = 0; i < charities.size(); i++) {
        if (charities[i].name == name) {
            charities.erase(charities.begin() + i);
            saveCharitiesToFile();
            cout << "Charity removed successfully!\n";
            return;
        }
    }
    cout << "Charity not found.\n";
}
void modifyCharity() {
    cin.ignore();
    string name;
    cout << "Enter the name of charity to modify: ";
    getline(cin, name);
    for (int i = 0; i < charities.size(); i++) {
        if (charities[i].name == name) {
            cout << "Enter new description: ";
            getline(cin, charities[i].description);
            cout << "Enter new target amount: ";
            cin >> charities[i].targetAmount;
            cout << "Enter new current amount: ";
            cin >> charities[i].currentAmount;
            cin.ignore();
            cout << "Enter new deadline: ";
            getline(cin, charities[i].deadline);
            cout << "Enter new status: ";
            getline(cin, charities[i].status);
            saveCharitiesToFile();
            cout << "Charity updated successfully!\n";
            return;
        }
    }
    cout << "Charity not found.\n";
}
void adminMenu() {
    loadCharitiesFromFile();
    int choice;
    do {
        cout << "\nAdmin Menu:\n";
        cout << "1. Add Charity\n";
        cout << "2. Remove Charity\n";
        cout << "3. Modify Charity\n";
        cout << "4. View Charities\n";
        cout << "5. Exit Admin Menu\n";
        cout << "Enter choice: ";
        cin >> choice;
        switch (choice) {
        case 1: addCharity(); break;
        case 2: removeCharity(); break;
        case 3: modifyCharity(); break;
        case 4: browseCharities(); break;
        case 5: cout << "Returning to main menu...\n"; break;
        default: cout << "Invalid choice!\n";
        }
    } while (choice != 5);
}
string capitalizeFirstLetter(string str) {
    if (!str.empty()) {
        str[0] = toupper(str[0]);
        for (int i = 1; i < str.length(); i++) {
            str[i] = tolower(str[i]);
        }
    }
    return str;
}
bool isFirstLetterCapital(string str) {
    if (str.empty()) return false;
    return isupper(str[0]);
}
bool isEmailExists(const string& email) {
    for (int i = 0; i < users.size(); i++) {
        if (users[i].email == email)
            return true;
    }
    return false;
}
bool isPhoneExists(const string& phone) {
    for (const auto& user : users) {
        if (user.phone == phone) return true;
    }
    return false;
}

void registerUser() {
    User newUser;
    newUser.userID = generateUserID();

    while (true) {
        cout << "Enter first name: ";
        cin >> newUser.firstName;
        if (!isFirstLetterCapital(newUser.firstName)) {
            cout << "Error: First letter must be CAPITAL. Please try again.\n";
        }
        else {
            break;
        }
    }

    while (true) {
        cout << "Enter last name: ";
        cin >> newUser.lastName;
        if (!isFirstLetterCapital(newUser.lastName)) {
            cout << "Error: First letter must be CAPITAL. Please try again.\n";
        }
        else {
            break;
        }
    }

    do {
        cout << "Enter email: ";
        cin >> newUser.email;
        if (!isValidEmail(newUser.email)) {
            cout << "Invalid email! (Format should be like name@example.com)\n";
        }
        else if (isEmailExists(newUser.email)) {
            cout << "This email already exists! Please try another.\n";
        }
    } while (!isValidEmail(newUser.email) || isEmailExists(newUser.email));

    do {
        cout << "Enter password : ";
        newUser.password = getHiddenPassword();
        if (!isValidPassword(newUser.password)) {
            cout << "Invalid password. (At least 8 characters, with letters, digits, and special characters)\n";
        }
    } while (!isValidPassword(newUser.password));

    do {
        cout << "Enter phone: ";
        cin >> newUser.phone;
        if (!isValidPhone(newUser.phone)) {
            cout << "Invalid phone number! Format should be like 03-123456\n";
        }
        else if (isPhoneExists(newUser.phone)) {
            cout << "This phone number already exists! Please try another.\n";
        }
    } while (!isValidPhone(newUser.phone) || isPhoneExists(newUser.phone));

    users.push_back(newUser);
    emailToUserIndex[newUser.email] = users.size() - 1;
    saveUserToFile(newUser);

    cout << "Registration successful!\n";
}
void donorMenu(int currentUserIndex);
bool loginUser() {
    string email, password;
    while (true) {
        cout << "Enter email: ";
        cin >> email;
        cout << "Enter password: ";
        password = getHiddenPassword();

        if (emailToUserIndex.find(email) != emailToUserIndex.end()) {
            int index = emailToUserIndex[email];
            if (users[index].password == password) {
                cout << "Welcome, " << users[index].firstName << "!\n";
                if (email == "admin@admin.com") {
                    adminMenu();
                }
                else {
                    donorMenu(index);
                }
                return true;
            }
        }
        cout << "Invalid email or password. Please try again.\n\n";
    }
}
void saveDonationsToFile() {
    ofstream file("donations.txt", ios::app); // Append mode
    if (!file) {
        cout << "Error opening donations.txt\n";
        return;
    }
    for (int i = lastSavedDonationIndex; i < donations.size(); i++) {
        file << donations[i].donationID << " "
            << donations[i].userID << " "
            << donations[i].charityID << " "
            << fixed << setprecision(0) << donations[i].amount << "$ "
            << donations[i].dateTime << " "
            << donations[i].message << endl;
    }
    lastSavedDonationIndex = donations.size();
    file.close();
}
void loadDonationsFromFile() {
    ifstream file("donations.txt");
    donations.clear();
    while (file) {
        Donation d;
        string amountStr;
        file >> d.donationID >> d.userID >> d.charityID >> amountStr;
        if (file.fail()) break;

        amountStr.erase(remove(amountStr.begin(), amountStr.end(), '$'), amountStr.end());
        d.amount = stod(amountStr);

        file >> d.dateTime;
        getline(file, d.message);
        if (!d.message.empty() && d.message[0] == ' ')
            d.message.erase(0, 1);

        donations.push_back(d);
    }
    file.close();
}
void browseCharities() {
    cout << "\nAvailable Charities:\n\n";
    cout << left << setw(5) << "ID"
        << setw(25) << "Charity Name"
        << setw(30) << "Description"
        << setw(12) << "Target($)"
        << setw(12) << "Current($)"
        << setw(22) << "Deadline"
        << setw(25) << "Status"
        << "Progress\n";

    cout << string(150, '-') << endl;

    for (const auto& charity : charities) {
        double progress = (charity.targetAmount > 0) ? (charity.currentAmount / charity.targetAmount) : 0.0;
        if (progress > 1.0) progress = 1.0;

        int barWidth = 20;
        int filled = static_cast<int>(progress * barWidth);

        string bar = "[";
        bar += string(filled, '#');
        bar += string(barWidth - filled, '-');
        bar += "]";

        cout << left
            << setw(5) << charity.charityID
            << setw(25) << charity.name.substr(0, 24)
            << setw(30) << charity.description.substr(0, 29)
            << setw(12) << fixed << setprecision(0) << charity.targetAmount
            << setw(12) << fixed << setprecision(0) << charity.currentAmount
            << setw(22) << charity.deadline
            << setw(25) << charity.status.substr(0, 24)
            << bar << endl;
    }

    cout << string(150, '-') << endl;
}
double getTotalDonatedByUser(int userID) {
    double total = 0.0;
    for (const auto& d : donations) {
        if (d.userID == userID) {
            total += d.amount;
        }
    }
    return total;
}
void makeDonation(int currentUserIndex) {
    browseCharities();
    cout << "Enter the number of the charity you want to donate to: ";
    int choice;
    cin >> choice;
    if (choice < 1 || choice > charities.size()) {
        cout << "Invalid choice!\n";
        return;
    }
    if (charities[choice - 1].status == "Closed for donations") {
        cout << "This charity is closed for donations!\n";
        return;
    }
    double amount;
    cout << "Enter donation amount: ";
    cin >> amount;
    cin.ignore();
    string message;
    cout << "Enter a message (optional): ";
    getline(cin, message);
    Donation d;
    d.donationID = donations.empty() ? 1 : donations.back().donationID + 1;
    d.userID = users[currentUserIndex].userID;
    d.charityID = charities[choice - 1].charityID;
    d.amount = amount;
    d.message = message;
    d.dateTime = getCurrentDateTimeString();
    charities[choice - 1].currentAmount += amount;
    if (charities[choice - 1].currentAmount >= charities[choice - 1].targetAmount) {
        charities[choice - 1].status = "Closed for donations";
        cout << "This charity has reached its goal and is now closed for donations!\n";
    }
    donations.push_back(d);
    logUserDonation(d, users[currentUserIndex], charities[choice - 1]);
    saveDonationsToFile();
    saveCharitiesToFile();
    double userTotal = getTotalDonatedByUser(users[currentUserIndex].userID);
    cout << "Thank you, " << users[currentUserIndex].firstName << "!\n";
    cout << "Your total donations so far: $" << fixed << setprecision(2) << userTotal << "\n";
    generateStyledHTMLReport();
}
void logUserDonation(const Donation& d, const User& user, const Charity& charity) {
    string filename = "donations_" + user.firstName + ".txt";
    ofstream file(filename, ios::app);
    if (!file) return;

    file << "Donation ID: " << d.donationID << "\n"
        << "Charity: " << charity.name << "\n"
        << "Amount: $" << fixed << setprecision(2) << d.amount << "\n"
        << "Message: " << d.message << "\n"
        << "Date: " << d.dateTime << "\n"
        << "-----------------------------\n";

    file.close();
}
void generateStyledHTMLReport() {
    ofstream file("donation_report.html");
    if (!file) {
        cout << "Error creating donation_report.html\n";
        return;
    }

    // Sort donations by datetime
    sort(donations.begin(), donations.end(), [](const Donation& a, const Donation& b) {
        return a.dateTime < b.dateTime;
        });

    file << R"(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <title>Donation Report</title>
    <style>
        body {
            font-family: Arial, sans-serif;
            margin: 40px;
            color: #333;
        }
        h1 { 
            text-align: center;
            margin-bottom: 40px;
        }
        table {
            width: 100%;
            border-collapse: collapse;
            font-size: 14px;
        }
        th, td {
            border: 1px solid #999;
            padding: 8px 12px;
            text-align: left;
        }
        th {
            background-color: #f0f0f0;
        }
        tfoot td {
            font-weight: bold;
            background-color: #f9f9f9;
        }
    </style>
</head>
<body>
    <h1>Donation Report</h1>
    <table>
    <thead>
      <tr>
        <th>Donor Name</th>
        <th>Email</th>
        <th>Charity</th>
        <th>Status</th>
        <th>Amount ($)</th>
        <th>Date & Time</th>
      </tr>
    </thead>
    <tbody>
)";

    double total = 0.0;
    for (const auto& d : donations) {
        string donorName = "Unknown", donorEmail = "Unknown";
        string charityName = "Unknown", charityStatus = "Unknown";

        for (const auto& u : users) {
            if (u.userID == d.userID) {
                donorName = u.firstName + " " + u.lastName;
                donorEmail = u.email;
                break;
            }
        }

        for (const auto& c : charities) {
            if (c.charityID == d.charityID) {
                charityName = c.name;
                charityStatus = c.status;
                break;
            }
        }

        total += d.amount;

        file << "<tr>"
            << "<td>" << donorName << "</td>"
            << "<td>" << donorEmail << "</td>"
            << "<td>" << charityName << "</td>"
            << "<td>" << charityStatus << "</td>"
            << "<td>" << fixed << setprecision(2) << d.amount << "</td>"
            << "<td>" << d.dateTime << "</td>"
            << "</tr>\n";
    }

    file << R"(
        </tbody>
        <tfoot>
            <tr>
                <td colspan="4">Total Donations</td>
                <td colspan="2">$)" << fixed << setprecision(2) << total << R"(</td>
            </tr>
        </tfoot>
    </table>
</body>
</html>
)";
    file.close();
    system("wkhtmltopdf donation_report.html donation_report.pdf >nul 2>&1");
}
void viewAndCancelDonations(int currentUserIndex) {
    cout << "\nYour Donations:\n";
    int found = 0;
    for (int i = 0; i < donations.size(); i++) {
        if (donations[i].userID == users[currentUserIndex].userID) {
            cout << found + 1 << ". Donation ID: " << donations[i].donationID
                << ", Charity ID: " << donations[i].charityID
                << ", Amount: $" << donations[i].amount
                << ", Message: " << donations[i].message << endl;
            found++;
        }
    }
    if (found == 0) {
        cout << "No donations found.\n";
        return;
    }

    cout << "Enter the number to cancel (0 to exit): ";
    int choice;
    cin >> choice;
    if (choice == 0) return;

    int realIndex = -1;
    int count = 0;
    for (int i = 0; i < donations.size(); i++) {
        if (donations[i].userID == users[currentUserIndex].userID) {
            count++;
            if (count == choice) {
                realIndex = i;
                break;
            }
        }
    }

    if (realIndex != -1) {
        donations.erase(donations.begin() + realIndex); // ✅ fixed line
        saveDonationsToFile();
        cout << "Donation canceled.\n";
    }
    else {
        cout << "Invalid selection.\n";
    }
}
void modifyDonation(int currentUserIndex) {
    cout << "\nYour Donations:\n";
    int found = 0;
    for (int i = 0; i < donations.size(); i++) {
        if (donations[i].userID == users[currentUserIndex].userID) {
            cout << found + 1 << ". Donation ID: " << donations[i].donationID
                << ", Charity ID: " << donations[i].charityID
                << ", Amount: $" << donations[i].amount << endl;
            found++;
        }
    }
    if (found == 0) {
        cout << "No donations found.\n";
        return;
    }
    cout << "Enter the number to modify (0 to exit): ";
    int choice;
    cin >> choice;
    if (choice == 0) return;
    int realIndex = -1;
    int count = 0;
    for (int i = 0; i < donations.size(); i++) {
        if (donations[i].userID == users[currentUserIndex].userID) {
            count++;
            if (count == choice) {
                realIndex = i;
                break;
            }
        }
    }
    if (realIndex != -1) {
        double oldAmount = donations[realIndex].amount;
        int oldCharityID = donations[realIndex].charityID;
        cout << "Enter new amount: ";
        double newAmount;
        cin >> newAmount;
        cin.ignore();
        browseCharities();
        cout << "Enter new charity number: ";
        int newCharityNum;
        cin >> newCharityNum;
        if (newCharityNum < 1 || newCharityNum > charities.size()) {
            cout << "Invalid charity.\n";
            return;
        }
        int newCharityID = charities[newCharityNum - 1].charityID;
        for (int i = 0; i < charities.size(); i++) {
            if (charities[i].charityID == oldCharityID) {
                charities[i].currentAmount -= oldAmount;
                if (charities[i].currentAmount < charities[i].targetAmount) {
                    charities[i].status = "Open for donations";
                }
                break;
            }
        }
        for (int i = 0; i < charities.size(); i++) {
            if (charities[i].charityID == newCharityID) {
                charities[i].currentAmount += newAmount;
                if (charities[i].currentAmount >= charities[i].targetAmount) {
                    charities[i].status = "Closed for donations";
                    cout << "This charity has reached its target and is now closed.\n";
                }
                break;
            }
        }
        donations[realIndex].amount = newAmount;
        donations[realIndex].charityID = newCharityID;
        saveCharitiesToFile();
        saveDonationsToFile();
        cout << "Donation updated successfully.\n";
    }
    else {
        cout << "Invalid selection.\n";
    }
}
void donorMenu(int currentUserIndex) {
    int choice;
    do {
        cout << "\nDonor Menu:\n";
        cout << "1. Browse Available Charities\n";
        cout << "2. Make a Donation\n";
        cout << "3. View and Cancel Donation\n";
        cout << "4. Modify Donation\n";
        cout << "5. Generate Donation Report (PDF)\n";
        cout << "6. Search charities\n";
        cout << "Enter choice: ";
        cin >> choice;
        switch (choice) {
        case 1: browseCharities(); break;
        case 2: makeDonation(currentUserIndex); break;
        case 3: viewAndCancelDonations(currentUserIndex); break;
        case 4: modifyDonation(currentUserIndex); break;
        case 5: generateStyledHTMLReport(); break;
        case 7: searchCharities(); break;
        default: cout << "Invalid choice. Please enter 1-5.\n"; break;
        }
    } while (choice != 5);
}
// Add these constants near the top with your other constants
const string USER_FILE = "users.csv";
const string CHARITY_FILE = "charities.csv";
const string DONATION_FILE = "donations.csv";
const string BACKUP_FOLDER = "backups/";

// Add these helper functions
void createBackupFolder() {
    // Create backups directory if it doesn't exist
#ifdef _WIN32
    system("mkdir backups 2> nul");
#else
    system("mkdir -p backups 2> /dev/null");
#endif
}

string getCurrentDateForFilename() {
    time_t now = time(0);
    tm* ltm = localtime(&now);
    char buffer[20];
    strftime(buffer, sizeof(buffer), "%Y%m%d_%H%M%S", ltm);
    return string(buffer);
}

void createBackup(const string& sourceFile) {
    createBackupFolder();
    string backupFile = BACKUP_FOLDER + sourceFile + "." + getCurrentDateForFilename() + ".bak";
    ifstream src(sourceFile, ios::binary);
    ofstream dst(backupFile, ios::binary);
    dst << src.rdbuf();
}

void passwordRecovery() {
    string email, phone;
    cout << "Enter your registered email: ";
    cin >> email;
    cout << "Enter your registered phone: ";
    cin >> phone;

    for (int i = 0; i < users.size(); i++) {
        if (users[i].email == email && users[i].phone == phone) {
            cout << "Your password is: " << users[i].password << endl;
            return;
        }
    }
    cout << "No account found with those credentials.\n";
}
void searchCharities() {
    cout << "\nSearch Charities\n";
    cout << "1. Search by keyword\n";
    cout << "2. Show only open charities\n";
    cout << "Enter choice: ";
    int choice;
    cin >> choice;
    cin.ignore();

    vector<Charity> results;

    if (choice == 1) {
        cout << "Enter keyword to search in name or description: ";
        string keyword;
        getline(cin, keyword);

        transform(keyword.begin(), keyword.end(), keyword.begin(), ::tolower);

        for (const auto& c : charities) {
            string name = c.name;
            string desc = c.description;
            transform(name.begin(), name.end(), name.begin(), ::tolower);
            transform(desc.begin(), desc.end(), desc.begin(), ::tolower);

            if (name.find(keyword) != string::npos || desc.find(keyword) != string::npos) {
                results.push_back(c);
            }
        }

    }
    else if (choice == 2) {
        for (const auto& c : charities) {
            if (c.status == "Open for donations") {
                results.push_back(c);
            }
        }
    }
    else {
        cout << "Invalid choice.\n";
        return;
    }

    if (results.empty()) {
        cout << "No charities found.\n";
    }
    else {
        cout << "\nMatching Charities:\n\n";
        for (const auto& c : results) {
            cout << "ID: " << c.charityID << " | " << c.name << " (" << c.status << ")\n";
            cout << "   " << c.description.substr(0, 80) << "...\n";
        }
    }
}
void logError(const string& message) {
    ofstream logfile("error.log", ios::app);
    if (logfile) {
        logfile << "[" << getCurrentDateTimeString() << "] ERROR: " << message << "\n";
        logfile.close();
    }
}

void logActivity(const string& message) {
    ofstream logfile("activity.log", ios::app);
    if (logfile) {
        logfile << "[" << getCurrentDateTimeString() << "] ACTIVITY: " << message << "\n";
        logfile.close();
    }
}

int main() {
    loadUsersFromFile(); 
    loadCharitiesFromFile(); 
    loadDonationsFromFile(); 

    void searchCharities();
    void logUserDonation(const Donation & d, const User & user, const Charity & charity);
    int choice = 0;
    cout << "Welcome to the Charity Donation System\n";
    while (true) {
        cout << "Do you have an account?\n1. Yes\n2. No\nChoice: ";
        cin >> choice;
        if (choice == 1) {
            if (loginUser()) {
                generateStyledHTMLReport();//generate PDF report after successful login
                break;
            }
        }
        else if (choice == 2) {
            registerUser();
            cout << "Now please log in with your new account:\n";
        }
        else {
            cout << "Invalid option. Please enter 1 or 2.\n";
            cin.clear();
            cin.ignore(10000, '\n');
        }
    }
    return 0;
}