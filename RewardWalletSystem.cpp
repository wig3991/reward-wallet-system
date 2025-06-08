#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <ctime>
#include <cstdlib>
#include "picosha2.h"


struct PendingUpdate {
    std::string username;
    std::string oldFullName, newFullName;
    std::string oldEmail, newEmail;
    std::string oldDob, newDob;
    std::string requestedBy;
    std::string otpCode;
};



using namespace std;

// ==== FUNCTION DECLARATIONS ====
string loginUser();
void viewWallet(const string& walletID);
void showUserMenu(const string& username);
void transferPoints(const string& senderID);
void saveTransactionLog(const string& sender, const string& receiver, int amount);
void viewTransactionHistory(const string& username);
void showAdminMenu();
string generateOTP(int length = 6);
void changePassword(const string& username);
string generateRandomPassword(int length = 8);
void createAccountForUser();
void addPoints(const string& username);
string hashPassword(const string& password);
void logUserChange(const string& editor, const string& targetUser, const string& field, const string& oldVal, const string& newVal);
void editUserInfo(const string& editorUsername, const string& editorRole);
PendingUpdate requestUserInfoUpdateByAdmin();
void confirmPendingUpdate(const PendingUpdate& pending);
bool masterWalletExists();
bool transfer(const string& from, const string& to, int amount);





// ==== USER CLASS ====
class User {
public:
    string username;
    string password;
    string fullName;
    string role;
    bool isFirstLogin;  // Thêm thuộc tính để lưu trạng thái lần đầu đăng nhập
    string email;       // Thêm thuộc tính email
    string dob;         // Thêm thuộc tính ngày sinh (dob - date of birth)

    // Cập nhật constructor để khởi tạo email và ngày sinh
    User(string user, string pass, string name, string r = "user", bool firstLogin = true, string mail = "", string dateOfBirth = "") {
        username = user;
        password = pass;
        fullName = name;
        role = r;
        isFirstLogin = firstLogin;
        email = mail;          // Khởi tạo email
        dob = dateOfBirth;     // Khởi tạo ngày sinh
    }

    void display() {
        cout << "Username : " << username << endl;
        cout << "Full name: " << fullName << endl;
        cout << "Role     : " << role << endl;
        cout << "First login: " << (isFirstLogin ? "Yes" : "No") << endl;
        cout << "Email    : " << email << endl;   // Hiển thị email
        cout << "Date of Birth: " << dob << endl; // Hiển thị ngày sinh
    }
};



// ==== WALLET CLASS ====
class Wallet {
public:
    string walletID;
    int balance;

    Wallet(string id, int b = 0) {
        walletID = id;
        balance = b;
    }

    void showBalance() {
        cout << "Wallet ID: " << walletID << endl;
        cout << "Balance  : " << balance << " points\n";
    }
};

// ==== SAVE USER TO FILE ====
// Cập nhật hàm lưu thông tin người dùng vào file khi dùng isFirstLogin
void saveUserToFile(const User& user) {
    ofstream file("users.txt", ios::app);
    if (file.is_open()) {
        file << user.username << "," << user.password << "," << user.fullName << ","
            << user.role << "," << (user.isFirstLogin ? "true" : "false") << ","
            << user.email << "," << user.dob << "\n";  // Lưu email và ngày sinh
        file.close();
    }
    else {
        cout << "Failed to open user file.\n";
    }
}


// ==== REGISTER USER ====
void registerUser() {
    string username, password, name, email, dob;

    cout << "=== Register Account ===\n";
    cout << "Enter username: ";
    cin >> username;
    cout << "Enter password: ";
    cin >> password;
    cin.ignore();  // Xử lý newline sau password
    cout << "Enter full name: ";
    getline(cin, name);
    cout << "Enter email: ";
    getline(cin, email);
    cout << "Enter date of birth (DD/MM/YYYY): ";
    getline(cin, dob);

    string hashed = hashPassword(password);
    User newUser(username, hashed, name, "user", false, email, dob);
    saveUserToFile(newUser);
    cout << "Registration successful!\n";

    // Kiểm tra và tạo ví người dùng với 0 điểm ban đầu
    ofstream wf("wallets.txt", ios::app);
    if (wf.is_open()) {
        wf << username << ",0\n";  // Ban đầu là 0 điểm, sẽ cộng qua hàm transfer
        wf.close();
    }
    else {
        cout << "[ERROR] Failed to create wallet.\n";
        return;
    }

    // Chuyển 100 điểm từ master sang người dùng
    if (!transfer("__master__", username, 100)) {
        cout << "[ERROR] Failed to fund new user wallet. Please check master balance.\n";
        return;
    }

    cout << "[OK] A wallet with 100 points was funded from master wallet.\n";
}



// ==== VIEW WALLET ====
void viewWallet(const string& walletID) {
    ifstream file("wallets.txt");
    if (!file.is_open()) {
        cout << "Cannot open wallet file.\n";
        return;
    }

    string line;
    while (getline(file, line)) {
        size_t pos = line.find(',');
        string id = line.substr(0, pos);
        int bal = stoi(line.substr(pos + 1));

        if (id == walletID) {
            Wallet w(id, bal);
            w.showBalance();
            return;
        }
    }

    cout << "Wallet not found.\n";
}

// ==== USER MENU ====
void showUserMenu(const string& username) {
    int choice;
    do {
        cout << "\n=== USER MENU ===\n";
        cout << "1. View Wallet\n";
        cout << "2. Transfer Points\n";
        cout << "3. View Transaction History\n";
        cout << "4. Add Points\n";
        cout << "5. Change Password\n";
        cout << "6. Edit Personal Information\n"; //  Mới
        cout << "7. Logout\n";
        cout << "Choose: ";
        cin >> choice;

        switch (choice) {
        case 1:
            viewWallet(username);
            break;
        case 2:
            transferPoints(username);
            break;
        case 3:
            viewTransactionHistory(username);
            break;
        case 4:
            addPoints(username);
            break;
        case 5:
            changePassword(username);
            break;
        case 6:
            editUserInfo(username, "user"); //  Cho phép chỉnh thông tin cá nhân
            break;
        case 7:
            cout << "Logging out...\n";
            break;
        default:
            cout << "Invalid choice.\n";
        }
    } while (choice != 7);
}




// ==== LOGIN USER ====
string loginUser() {
    string username, password;
    cout << "=== Login ===\n";
    cout << "Enter username: ";
    cin >> username;
    cout << "Enter password: ";
    cin >> password;

    ifstream file("users.txt");
    if (!file.is_open()) {
        cout << "Could not open user database.\n";
        return "";
    }

    string line;
    while (getline(file, line)) {
        vector<string> fields;
        string temp = line;
        size_t pos = 0;

        while ((pos = temp.find(',')) != string::npos) {
            fields.push_back(temp.substr(0, pos));
            temp.erase(0, pos + 1);
        }
        fields.push_back(temp); // Add last field (dob)

        if (fields.size() < 7) continue;

        string u = fields[0];
        string p = fields[1];
        string name = fields[2];
        string role = fields[3];
        string firstLogin = fields[4];

        // So sánh thông tin đăng nhập
        if (u == username && p == hashPassword(password)) {
            cout << "\nLogin successful!\n";
            cout << "Welcome, " << name << " (" << role << ")\n";

            // Nếu là lần đầu đăng nhập
            if (firstLogin == "true") {
                cout << "[WARNING] First login detected. You must change your password.\n";
                changePassword(username);
            }

            return role + "|" + u;
        }
    }

    cout << "Incorrect username or password.\n";
    return "";
}



// ==== TRANSFER POINTS ====
void transferPoints(const string& senderID) {
    string receiverID;
    int amount;

    cout << "=== TRANSFER POINTS ===\n";
    cout << "Enter receiver username: ";
    cin >> receiverID;

    if (receiverID == senderID) {
        cout << "[ERROR] You cannot transfer to your own wallet.\n";
        return;
    }

    cout << "Enter amount to transfer: ";
    cin >> amount;

    if (amount <= 0) {
        cout << "[ERROR] Invalid transfer amount.\n";
        return;
    }

    // OTP xác thực
    srand(time(0));
    string otp = generateOTP();
    cout << "\nOTP has been sent to your phone (simulated): " << otp << "\n";

    string userInput;
    cout << "Enter OTP to confirm transaction: ";
    cin >> userInput;

    if (userInput != otp) {
        cout << "[ERROR] Invalid OTP. Transaction cancelled.\n";
        return;
    }

    // Gọi hàm transfer dùng chung
    if (transfer(senderID, receiverID, amount)) {
        cout << "[OK] Transfer successful! " << amount << " points sent to " << receiverID << ".\n";
    }
    else {
        cout << "[ERROR] Transfer failed.\n";
    }
}



// ==== SAVE TRANSACTION LOG ====
void saveTransactionLog(const string& sender, const string& receiver, int amount) {
    ofstream log("transactions.txt", ios::app);
    if (log.is_open()) {
        // Get current time
        time_t now = time(0);
        tm ltm;
        localtime_s(&ltm, &now); // [OK] dùng hàm an toàn

        char timestamp[20];
        strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", &ltm);

        log << "[" << timestamp << "] " << sender << " transferred " << amount << " points to " << receiver << "\n";
        log.close();
    }
    else {
        cout << "Failed to write transaction log.\n";
    }
}

// ==== MAIN FUNCTION ====
int main() {
    int choice;
    do {
        cout << "\n=== MAIN MENU ===\n";
        cout << "1. Register new account\n";
        cout << "2. Login\n";
        cout << "3. Exit\n";
        cout << "Select: ";
        cin >> choice;

        switch (choice) {
        case 1:
            registerUser();
            break;
        case 2: {
            string loginResult = loginUser();
            size_t sep = loginResult.find('|');
            if (sep != string::npos) {
                string role = loginResult.substr(0, sep);
                string username = loginResult.substr(sep + 1);

                if (role == "user") {
                    showUserMenu(username);
                }
                else if (role == "admin") {
                    showAdminMenu();
                }
            }
            else {
                cout << "Login failed.\n";
            }
            break;
        }
        case 3:
            cout << "Goodbye!\n";
            break;
        default:
            cout << "Invalid choice.\n";
        }
    } while (choice != 3);

    return 0;
}

// ==== VIEW TRANSACTION HISTORY ====
void viewTransactionHistory(const string& username) {
    ifstream file("transactions.txt");
    if (!file.is_open()) {
        cout << "No transaction history found.\n";
        return;
    }

    vector<string> matchedLines;
    string line;

    while (getline(file, line)) {
        if (line.find(username) != string::npos) {
            matchedLines.push_back(line);
        }
    }

    file.close();

    cout << "\n=== TRANSACTION HISTORY (newest first) ===\n";
    if (matchedLines.empty()) {
        cout << "No transactions involving this user.\n";
    }
    else {
        for (int i = matchedLines.size() - 1; i >= 0; --i) {
            cout << matchedLines[i] << endl;
        }
    }
}

// ==== ADMIN MENU ====
void showAdminMenu() {
    int choice;
    do {
        cout << "\n=== ADMIN MENU ===\n";
        cout << "1. View All Users\n";
        cout << "2. View All Transactions\n";
        cout << "3. Create Account for User\n";
        cout << "4. View Change History\n";
        cout << "5. Request Info Change with OTP Confirmation\n";
        cout << "6. Logout\n";
        cout << "Choose: ";
        cin >> choice;

        switch (choice) {
        case 1: {
            ifstream file("users.txt");
            if (!file.is_open()) {
                cout << "Cannot open user file.\n";
                break;
            }
            cout << "\n--- USERS LIST ---\n";
            string line;
            while (getline(file, line)) {
                vector<string> fields;
                size_t pos;
                string temp = line;
                while ((pos = temp.find(',')) != string::npos) {
                    fields.push_back(temp.substr(0, pos));
                    temp.erase(0, pos + 1);
                }
                fields.push_back(temp);

                if (fields.size() >= 4) {
                    cout << "Username: " << fields[0]
                        << " | Full name: " << fields[2]
                        << " | Role: " << fields[3] << "\n";
                }
            }
            file.close();
            break;
        }

        case 2: {
            ifstream log("transactions.txt");
            if (!log.is_open()) {
                cout << "No transaction history found.\n";
                break;
            }
            cout << "\n--- ALL TRANSACTIONS ---\n";
            string line;
            while (getline(log, line)) {
                cout << line << "\n";
            }
            log.close();
            break;
        }

        case 3:
            createAccountForUser();
            break;

        case 4: {
            ifstream log("user_changes.log");
            if (!log.is_open()) {
                cout << "No user change history found.\n";
                break;
            }
            cout << "\n--- USER CHANGE HISTORY ---\n";
            string line;
            while (getline(log, line)) {
                cout << line << "\n";
            }
            log.close();
            break;
        }

        case 5: {
            PendingUpdate pending = requestUserInfoUpdateByAdmin();
            if (!pending.username.empty()) {
                confirmPendingUpdate(pending);
            }
            break;
        }

        case 6:
            cout << "Logging out of admin account...\n";
            break;

        default:
            cout << "Invalid option.\n";
        }
    } while (choice != 6);
}





string generateOTP(int length) {
    string otp = "";
    for (int i = 0; i < length; ++i) {
        otp += to_string(rand() % 10);
    }
    return otp;
}
// Hàm thay đổi mật khẩu
void changePassword(const string& username) {
    string oldPassword, newPassword, confirmPassword;

    ifstream file("users.txt");
    if (!file.is_open()) {
        cout << "Error opening user file.\n";
        return;
    }

    vector<User> users;
    string line;
    bool found = false;

    while (getline(file, line)) {
        vector<string> fields;
        size_t pos = 0;
        string temp = line;
        while ((pos = temp.find(',')) != string::npos) {
            fields.push_back(temp.substr(0, pos));
            temp.erase(0, pos + 1);
        }
        fields.push_back(temp); // last field

        if (fields.size() < 7) continue;

        User u(fields[0], fields[1], fields[2], fields[3], fields[4] == "true", fields[5], fields[6]);

        if (u.username == username) {
            cout << "Enter your old password: ";
            cin >> oldPassword;
            if (hashPassword(oldPassword) != u.password) {
                cout << "[ERROR] Incorrect old password.\n";
                file.close();
                return;
            }

            cout << "Enter new password: ";
            cin >> newPassword;
            cout << "Confirm new password: ";
            cin >> confirmPassword;

            if (newPassword != confirmPassword) {
                cout << "[ERROR] Passwords do not match.\n";
                file.close();
                return;
            }

            // [OK] Xác minh OTP
            srand(time(0)); // Khởi tạo ngẫu nhiên
            string otp = generateOTP(6);
            cout << "\nAn OTP has been sent to your phone (simulated): " << otp << "\n";

            string inputOTP;
            cout << "Enter OTP to confirm password change: ";
            cin >> inputOTP;

            if (inputOTP != otp) {
                cout << "[ERROR] Incorrect OTP. Password change cancelled.\n";
                file.close();
                return;
            }

            u.password = hashPassword(newPassword);
            u.isFirstLogin = false;
            found = true;
        }

        users.push_back(u);
    }
    file.close();

    if (!found) {
        cout << "[ERROR] User not found.\n";
        return;
    }

    ofstream out("users.txt");
    if (!out.is_open()) {
        cout << "Error writing user file.\n";
        return;
    }

    for (const User& u : users) {
        out << u.username << "," << u.password << "," << u.fullName << ","
            << u.role << "," << (u.isFirstLogin ? "true" : "false") << ","
            << u.email << "," << u.dob << "\n";
    }
    out.close();

    cout << "[OK] Password changed successfully with OTP verification.\n";
}



// Hàm sinh mật khẩu ngẫu nhiên
string generateRandomPassword(int length) {
    const string characters = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789!@#$%^&*()";
    string password = "";
    srand(time(0));  // Khởi tạo số ngẫu nhiên
    for (int i = 0; i < length; ++i) {
        password += characters[rand() % characters.length()];
    }
    return password;
}
// Thêm mật khẩu ngẫu nhiên vào phần tạo tài khoản của admin
// Thêm email và ngày sinh vào hàm tạo tài khoản của admin
void createAccountForUser() {
    string username, fullname, email, dob;

    cout << "\n=== Create Account for User ===\n";
    cout << "Enter username: ";
    cin >> username;
    cout << "Enter full name: ";
    cin.ignore();
    getline(cin, fullname);

    cout << "Enter email: ";
    getline(cin, email);
    cout << "Enter date of birth (DD/MM/YYYY): ";
    getline(cin, dob);

    // Sinh mật khẩu ngẫu nhiên
    string randomPassword = generateRandomPassword(10);
    cout << "Generated password for the user: " << randomPassword << endl;

    // Hash và lưu người dùng
    string hashed = hashPassword(randomPassword);
    User newUser(username, hashed, fullname, "user", true, email, dob);
    saveUserToFile(newUser);

    // Tạo ví với 0 điểm ban đầu
    ofstream wf("wallets.txt", ios::app);
    if (wf.is_open()) {
        wf << username << ",0\n";
        wf.close();
    }
    else {
        cout << "[ERROR] Failed to create wallet.\n";
        return;
    }

    // Chuyển 100 điểm từ master
    if (!transfer("__master__", username, 100)) {
        cout << "[ERROR] Failed to fund user wallet. Please check master balance.\n";
        return;
    }

    cout << "[OK] Wallet created and funded from master wallet.\n";
    cout << "Please notify the user to log in using the following password: " << randomPassword << endl;
}


// Hàm nạp điểm
void addPoints(const string& username) {
    int amount;
    cout << "=== Add Points ===\n";
    cout << "Enter the amount of points to add: ";
    cin >> amount;

    if (amount <= 0) {
        cout << "[ERROR] Amount must be greater than 0.\n";
        return;
    }

    // Xác nhận OTP
    srand(time(0));
    string otp = generateOTP(6);
    cout << "\nOTP has been sent to your phone (simulated): " << otp << "\n";

    string userInput;
    cout << "Enter OTP to confirm transaction: ";
    cin >> userInput;

    if (userInput != otp) {
        cout << "[ERROR] Invalid OTP. Transaction cancelled.\n";
        return;
    }

    // Bước 1: cộng điểm vào master wallet (mô phỏng nạp từ ngoài)
    vector<Wallet> wallets;
    ifstream file("wallets.txt");
    if (!file.is_open()) {
        cout << "[ERROR] Cannot open wallet file.\n";
        return;
    }

    string line;
    while (getline(file, line)) {
        size_t pos = line.find(',');
        string id = line.substr(0, pos);
        int bal = stoi(line.substr(pos + 1));
        wallets.push_back(Wallet(id, bal));
    }
    file.close();

    // Tìm ví tổng
    int masterIndex = -1;
    for (int i = 0; i < wallets.size(); ++i) {
        if (wallets[i].walletID == "__master__") {
            masterIndex = i;
            break;
        }
    }

    if (masterIndex == -1) {
        cout << "[ERROR] Master wallet not found.\n";
        return;
    }

    wallets[masterIndex].balance += amount;  // Ghi nhận nạp hệ thống

    // Ghi lại wallets.txt
    ofstream outFile("wallets.txt");
    for (const Wallet& w : wallets) {
        outFile << w.walletID << "," << w.balance << "\n";
    }
    outFile.close();

    saveTransactionLog("System", "__master__", amount);  // Ghi log nạp vào hệ thống

    // Bước 2: chuyển từ master sang người dùng
    if (!transfer("__master__", username, amount)) {
        cout << "[ERROR] Failed to transfer from master to user.\n";
        return;
    }

    cout << "[OK] Points successfully added to your wallet.\n";
}



string hashPassword(const string& password) {
    return picosha2::hash256_hex_string(password);
}


void logUserChange(const string& editor, const string& targetUser, const string& field, const string& oldVal, const string& newVal) {
    ofstream log("user_changes.log", ios::app);
    if (!log.is_open()) return;

    time_t now = time(0);
    tm ltm;
    localtime_s(&ltm, &now);
    char timestamp[20];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", &ltm);

    log << "[" << timestamp << "] " << editor << " changed '" << field
        << "' of user '" << targetUser << "' from '" << oldVal << "' to '" << newVal << "'\n";
    log.close();
}


void editUserInfo(const string& editorUsername, const string& editorRole) {
    string targetUsername;

    if (editorRole == "admin") {
        cout << "\n=== Admin: Edit User Information ===\n";
        cout << "Enter username of the user to edit: ";
        cin >> targetUsername;
    }
    else {
        // User chỉ được sửa thông tin chính mình
        targetUsername = editorUsername;
        cout << "\n=== Edit Your Information ===\n";
    }

    // Đọc danh sách người dùng
    ifstream inFile("users.txt");
    if (!inFile.is_open()) {
        cout << "[ERROR] Cannot open user file.\n";
        return;
    }

    vector<User> users;
    string line;
    bool found = false;

    while (getline(inFile, line)) {
        vector<string> fields;
        string temp = line;
        size_t pos;
        while ((pos = temp.find(',')) != string::npos) {
            fields.push_back(temp.substr(0, pos));
            temp.erase(0, pos + 1);
        }
        fields.push_back(temp);

        if (fields.size() < 7) continue;

        User u(fields[0], fields[1], fields[2], fields[3], fields[4] == "true", fields[5], fields[6]);

        if (u.username == targetUsername) {
            found = true;
            string input;

            // fullName
            cout << "Current full name: " << u.fullName << "\nNew full name (leave blank to skip): ";
            cin.ignore();
            getline(cin, input);
            if (!input.empty()) {
                logUserChange(editorUsername, u.username, "fullName", u.fullName, input);
                u.fullName = input;
            }

            // email
            cout << "Current email: " << u.email << "\nNew email (leave blank to skip): ";
            getline(cin, input);
            if (!input.empty()) {
                logUserChange(editorUsername, u.username, "email", u.email, input);
                u.email = input;
            }

            // dob
            cout << "Current date of birth: " << u.dob << "\nNew date of birth (leave blank to skip): ";
            getline(cin, input);
            if (!input.empty()) {
                logUserChange(editorUsername, u.username, "dob", u.dob, input);
                u.dob = input;
            }

            // role (chỉ admin mới chỉnh được)
            if (editorRole == "admin") {
                cout << "Current role: " << u.role << "\nNew role (user/admin) (leave blank to skip): ";
                getline(cin, input);
                if (!input.empty() && (input == "user" || input == "admin")) {
                    logUserChange(editorUsername, u.username, "role", u.role, input);
                    u.role = input;
                }
            }

        }

        users.push_back(u);
    }
    inFile.close();

    if (!found) {
        cout << "[ERROR] User not found.\n";
        return;
    }

    // Ghi lại danh sách đã cập nhật
    ofstream outFile("users.txt");
    if (!outFile.is_open()) {
        cout << "[ERROR] Cannot write to user file.\n";
        return;
    }

    for (const User& u : users) {
        outFile << u.username << "," << u.password << "," << u.fullName << ","
            << u.role << "," << (u.isFirstLogin ? "true" : "false") << ","
            << u.email << "," << u.dob << "\n";
    }
    outFile.close();

    cout << "[OK] User information updated successfully.\n";
}
