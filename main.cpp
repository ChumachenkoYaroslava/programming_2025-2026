#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <stdexcept>
#include <windows.h>

using namespace std;

int utf8Len(unsigned char c) {
    if ((c & 0x80) == 0x00) return 1;
    if ((c & 0xE0) == 0xC0) return 2;
    if ((c & 0xF0) == 0xE0) return 3;
    if ((c & 0xF8) == 0xF0) return 4;
    return 1;
}

string firstChar(const string& s) {
    if (s.empty()) return "";
    int len = utf8Len((unsigned char)s[0]);
    if (len > (int)s.size()) len = (int)s.size();
    return s.substr(0, len);
}

const vector<string>& rusUpper() {
    static const vector<string> a = {
        "А","Б","В","Г","Д","Е","Ё","Ж","З","И","Й","К","Л","М","Н",
        "О","П","Р","С","Т","У","Ф","Х","Ц","Ч","Ш","Щ","Ъ","Ы","Ь",
        "Э","Ю","Я"
    };
    return a;
}

const vector<string>& rusLower() {
    static const vector<string> a = {
        "а","б","в","г","д","е","ё","ж","з","и","й","к","л","м","н",
        "о","п","р","с","т","у","ф","х","ц","ч","ш","щ","ъ","ы","ь",
        "э","ю","я"
    };
    return a;
}

bool isAsciiLetter(const string& c) {
    return c.size() == 1 &&
        ((c[0] >= 'a' && c[0] <= 'z') || (c[0] >= 'A' && c[0] <= 'Z'));
}

string shiftLetter(const string& c) {
    if (isAsciiLetter(c)) {
        char ch = c[0];
        if (ch >= 'a' && ch <= 'z') return string(1, (ch == 'z') ? 'a' : ch + 1);
        return string(1, (ch == 'Z') ? 'A' : ch + 1);
    }
    const auto& up = rusUpper();
    const auto& lo = rusLower();
    for (int i = 0; i < (int)up.size(); i++)
        if (c == up[i]) return up[(i + 1) % up.size()];
    for (int i = 0; i < (int)lo.size(); i++)
        if (c == lo[i]) return lo[(i + 1) % lo.size()];
    return c;
}

string shiftFirstLetter(const string& s) {
    if (s.empty()) return s;
    string first = firstChar(s);
    string rest = s.substr(first.size());
    return shiftLetter(first) + rest;
}

class RowProxy {
    vector<string>* row;
public:
    explicit RowProxy(vector<string>& r) : row(&r) {}
    string& operator[](int j) {
        if (j < 0 || j >= (int)row->size())
            throw out_of_range("column index out of range");
        return (*row)[j];
    }
    int size() const { return (int)row->size(); }
};

class ConstRowProxy {
    const vector<string>* row;
public:
    explicit ConstRowProxy(const vector<string>& r) : row(&r) {}
    const string& operator[](int j) const {
        if (j < 0 || j >= (int)row->size())
            throw out_of_range("column index out of range");
        return (*row)[j];
    }
    int size() const { return (int)row->size(); }
};

class JaggedArray {
    vector<vector<string>> data;

public:
    JaggedArray() = default;
    explicit JaggedArray(int rows) : data(rows) {}

    RowProxy operator[](int i) {
        if (i < 0 || i >= (int)data.size())
            throw out_of_range("row index out of range");
        return RowProxy(data[i]);
    }
    ConstRowProxy operator[](int i) const {
        if (i < 0 || i >= (int)data.size())
            throw out_of_range("row index out of range");
        return ConstRowProxy(data[i]);
    }

    int rows() const { return (int)data.size(); }
    int cols(int i) const { return (int)data[i].size(); }

    void deleteAt(int i, int j) {
        if (i < 0 || i >= (int)data.size()) return;
        if (j < 0 || j >= (int)data[i].size()) return;
        data[i].erase(data[i].begin() + j);
    }

    void deleteItem(const string& item) {
        for (auto& row : data)
            row.erase(remove(row.begin(), row.end(), item), row.end());
    }

    void add_endline(int k, const string& item) {
        if (k < 0 || k >= (int)data.size()) return;
        data[k].push_back(item);
    }

    JaggedArray operator+(const JaggedArray& other) const {
        JaggedArray result;
        int maxRows = max((int)data.size(), (int)other.data.size());
        result.data.resize(maxRows);
        for (int i = 0; i < maxRows; i++) {
            int sa = (i < (int)data.size()) ? (int)data[i].size() : 0;
            int sb = (i < (int)other.data.size()) ? (int)other.data[i].size() : 0;
            int m = min(sa, sb);
            for (int j = 0; j < m; j++) {
                const string& a = data[i][j];
                const string& b = other.data[i][j];
                if (a.empty())        result.data[i].push_back(b);
                else if (b.empty())   result.data[i].push_back(a);
                else                  result.data[i].push_back(a + b);
            }
        }
        return result;
    }

    JaggedArray& operator++() {
        for (auto& row : data)
            for (auto& item : row)
                if (!item.empty())
                    item = shiftFirstLetter(item);
        return *this;
    }

    JaggedArray operator++(int) {
        JaggedArray copy = *this;
        ++(*this);
        return copy;
    }

    void sortRows() {
        for (auto& row : data)
            sort(row.begin(), row.end());
    }

    void print() const {
        HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
        CONSOLE_SCREEN_BUFFER_INFO info;
        GetConsoleScreenBufferInfo(h, &info);
        WORD def = info.wAttributes;

        static const WORD colors[] = {
            FOREGROUND_RED | FOREGROUND_INTENSITY,
            FOREGROUND_GREEN | FOREGROUND_INTENSITY,
            FOREGROUND_BLUE | FOREGROUND_INTENSITY,
            FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY,
            FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_INTENSITY,
            FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY,
            FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY
        };
        const int NC = 7;

        for (int i = 0; i < (int)data.size(); i++) {
            SetConsoleTextAttribute(h, colors[i % NC]);
            cout << "Строка " << i << ": ";
            for (const auto& item : data[i])
                cout << "[" << item << "] ";
            cout << "\n";
        }
        SetConsoleTextAttribute(h, def);
    }

    void loadFromFile(const string& filename) {
        data.clear();
        string ext;
        size_t dot = filename.rfind('.');
        if (dot != string::npos) ext = filename.substr(dot);

        if (ext == ".json")      loadJSON(filename);
        else if (ext == ".csv")  loadCSV(filename);
        else                     loadTXT(filename);
    }

private:
    void loadTXT(const string& filename) {
        ifstream f(filename);
        if (!f) { cerr << "Не удалось открыть " << filename << "\n"; return; }
        string line;
        while (getline(f, line)) {
            if (!line.empty() && line.back() == '\r') line.pop_back();
            vector<string> row;
            istringstream iss(line);
            string tok;
            while (iss >> tok) row.push_back(tok);
            data.push_back(row);
        }
    }

    void loadCSV(const string& filename) {
        ifstream f(filename);
        if (!f) { cerr << "Не удалось открыть " << filename << "\n"; return; }
        string line;
        while (getline(f, line)) {
            if (!line.empty() && line.back() == '\r') line.pop_back();
            vector<string> row;
            istringstream iss(line);
            string tok;
            while (getline(iss, tok, ',')) {
                size_t b = tok.find_first_not_of(" \t\r\n");
                size_t e = tok.find_last_not_of(" \t\r\n");
                if (b == string::npos) tok = "";
                else tok = tok.substr(b, e - b + 1);
                row.push_back(tok);
            }
            data.push_back(row);
        }
    }

    void loadJSON(const string& filename) {
        ifstream f(filename);
        if (!f) { cerr << "Не удалось открыть " << filename << "\n"; return; }
        string s((istreambuf_iterator<char>(f)), istreambuf_iterator<char>());

        vector<string> row;
        string cur;
        bool inStr = false;
        int depth = 0;
        for (size_t i = 0; i < s.size(); i++) {
            char c = s[i];
            if (inStr) {
                if (c == '\\' && i + 1 < s.size()) {
                    cur += s[++i];
                }
                else if (c == '"') {
                    inStr = false;
                    row.push_back(cur);
                    cur.clear();
                }
                else {
                    cur += c;
                }
            }
            else {
                if (c == '"') inStr = true;
                else if (c == '[') { depth++; if (depth == 2) row.clear(); }
                else if (c == ']') {
                    if (depth == 2) data.push_back(row);
                    depth--;
                }
            }
        }
    }
};

void createInitialFile(const string& filename) {
    ofstream out(filename, ios::binary);
    if (!out) {
        cerr << "Не удалось создать " << filename << "\n";
        return;
    }
    out << "абв где ёж\n";
    out << "яблоко груша\n";
    out << "тест 123\n";
    out << "АБВ ГДЕ\n";
    out.close();
    cout << "Создан файл с начальными данными: " << filename << "\n";
}
int main() {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    system("chcp 65001 > nul");

    const string filename = "input.txt";

    {
        ifstream check(filename);
        if (!check.good()) {
            createInitialFile(filename);
        }
    }

    JaggedArray A;
    A.loadFromFile(filename);

    cout << "\n=== Исходный массив A ===\n";
    A.print();

    cout << "\n=== A после сортировки строк ===\n";
    A.sortRows();
    A.print();

    cout << "\n=== A после ++A (первая буква каждого элемента сдвинута) ===\n";
    ++A;
    A.print();

    JaggedArray B;
    B.loadFromFile(filename);
    B.sortRows();

    cout << "\n=== C = A + B (поэлементная конкатенация) ===\n";
    JaggedArray C = A + B;
    C.print();

    cout << "\n=== A.deleteAt(0, 0) ===\n";
    A.deleteAt(0, 0);
    A.print();

    cout << "\n=== A.add_endline(0, \"новый\") ===\n";
    A.add_endline(0, "новый");
    A.print();

    cout << "\n=== A.deleteItem(\"тест\") ===\n";
    A.deleteItem("тест");
    A.print();

    cout << "\n=== A[0][0] = \"изменено\" (operator[]) ===\n";
    A[0][0] = "изменено";
    A.print();

    return 0;
}