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


