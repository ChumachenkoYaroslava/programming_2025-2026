#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <stdexcept>
#include <limits>
#include <windows.h>

using namespace std;

static int utf8SeqLen(unsigned char b) {
    if ((b & 0x80) == 0x00) return 1;
    if ((b & 0xE0) == 0xC0) return 2;
    if ((b & 0xF0) == 0xE0) return 3;
    if ((b & 0xF8) == 0xF0) return 4;
    return 1;
}

static string firstGlyph(const string& s) {
    if (s.empty()) return "";
    int n = utf8SeqLen((unsigned char)s[0]);
    if (n > (int)s.size()) n = (int)s.size();
    return s.substr(0, n);
}

static const vector<string> ALPHA_UP = {
    "А","Б","В","Г","Д","Е","Ё","Ж","З","И","Й","К","Л","М","Н",
    "О","П","Р","С","Т","У","Ф","Х","Ц","Ч","Ш","Щ","Ъ","Ы","Ь",
    "Э","Ю","Я"
};
static const vector<string> ALPHA_LO = {
    "а","б","в","г","д","е","ё","ж","з","и","й","к","л","м","н",
    "о","п","р","с","т","у","ф","х","ц","ч","ш","щ","ъ","ы","ь",
    "э","ю","я"
};

static bool isLatinLetter(const string& g) {
    if (g.size() != 1) return false;
    char c = g[0];
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

static string nextGlyph(const string& g) {
    if (g.empty()) return g;
    if (isLatinLetter(g)) {
        char c = g[0];
        if (c == 'z') return "a";
        if (c == 'Z') return "A";
        return string(1, c + 1);
    }
    for (size_t i = 0; i < ALPHA_UP.size(); ++i)
        if (g == ALPHA_UP[i]) return ALPHA_UP[(i + 1) % ALPHA_UP.size()];
    for (size_t i = 0; i < ALPHA_LO.size(); ++i)
        if (g == ALPHA_LO[i]) return ALPHA_LO[(i + 1) % ALPHA_LO.size()];
    return g;
}

static string shiftFirstGlyph(const string& s) {
    if (s.empty()) return s;
    string f = firstGlyph(s);
    return nextGlyph(f) + s.substr(f.size());
}

class LineView {
    vector<string>* ref;
public:
    explicit LineView(vector<string>& r) : ref(&r) {}

    string& operator[](int j) {
        if (j < 0 || j >= (int)ref->size())
            throw out_of_range("j вне диапазона");
        return (*ref)[j];
    }
    const string& operator[](int j) const {
        if (j < 0 || j >= (int)ref->size())
            throw out_of_range("j вне диапазона");
        return (*ref)[j];
    }
    int length() const { return (int)ref->size(); }
};

class ConstLineView {
    const vector<string>* ref;
public:
    explicit ConstLineView(const vector<string>& r) : ref(&r) {}

    const string& operator[](int j) const {
        if (j < 0 || j >= (int)ref->size())
            throw out_of_range("j вне диапазона");
        return (*ref)[j];
    }
    int length() const { return (int)ref->size(); }
};

class JaggedArray {
    vector<vector<string>> grid_;

public:
    JaggedArray() = default;

    LineView operator[](int i) {
        if (i < 0 || i >= (int)grid_.size())
            throw out_of_range("i вне диапазона");
        return LineView(grid_[i]);
    }
    ConstLineView operator[](int i) const {
        if (i < 0 || i >= (int)grid_.size())
            throw out_of_range("i вне диапазона");
        return ConstLineView(grid_[i]);
    }

    int rowCount() const { return (int)grid_.size(); }
    int colCount(int i) const { return (int)grid_[i].size(); }
    bool empty() const { return grid_.empty(); }

    bool eraseAt(int i, int j) {
        if (i < 0 || i >= (int)grid_.size()) return false;
        if (j < 0 || j >= (int)grid_[i].size()) return false;
        grid_[i].erase(grid_[i].begin() + j);
        return true;
    }

    bool eraseValue(const string& value) {
        for (auto& line : grid_) {
            auto it = find(line.begin(), line.end(), value);
            if (it != line.end()) { line.erase(it); return true; }
        }
        return false;
    }

    int eraseAll(const string& value) {
        int count = 0;
        for (auto& line : grid_) {
            auto newEnd = remove(line.begin(), line.end(), value);
            count += (int)(line.end() - newEnd);
            line.erase(newEnd, line.end());
        }
        return count;
    }

    bool append(int k, const string& value) {
        if (k < 0 || k >= (int)grid_.size()) return false;
        grid_[k].push_back(value);
        return true;
    }

    void pushLine(const vector<string>& line) {
        grid_.push_back(line);
    }

    JaggedArray operator+(const JaggedArray& rhs) const {
        JaggedArray out;
        int total = max(rowCount(), rhs.rowCount());
        out.grid_.resize(total);
        for (int i = 0; i < total; ++i) {
            int a = (i < rowCount()) ? colCount(i) : 0;
            int b = (i < rhs.rowCount()) ? rhs.colCount(i) : 0;
            int len = min(a, b);
            for (int j = 0; j < len; ++j) {
                const string& L = grid_[i][j];
                const string& R = rhs.grid_[i][j];
                if (L.empty())          out.grid_[i].push_back(R);
                else if (R.empty())     out.grid_[i].push_back(L);
                else                    out.grid_[i].push_back(L + R);
            }
        }
        return out;
    }

    JaggedArray& operator++() {
        for (auto& line : grid_)
            for (auto& item : line)
                if (!item.empty())
                    item = shiftFirstGlyph(item);
        return *this;
    }

    JaggedArray operator++(int) {
        JaggedArray snapshot = *this;
        ++(*this);
        return snapshot;
    }

    void sortEachLine() {
        for (auto& line : grid_)
            sort(line.begin(), line.end());
    }

    void render() const {
        if (grid_.empty()) { cout << "(массив пуст)\n"; return; }

        HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
        CONSOLE_SCREEN_BUFFER_INFO csbi;
        GetConsoleScreenBufferInfo(h, &csbi);
        WORD saved = csbi.wAttributes;

        static const WORD palette[] = {
            FOREGROUND_RED | FOREGROUND_INTENSITY,
            FOREGROUND_GREEN | FOREGROUND_INTENSITY,
            FOREGROUND_BLUE | FOREGROUND_INTENSITY,
            FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY,
            FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_INTENSITY,
            FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY,
            FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY
        };
        const int paletteSize = sizeof(palette) / sizeof(palette[0]);

        for (int i = 0; i < rowCount(); ++i) {
            SetConsoleTextAttribute(h, palette[i % paletteSize]);
            cout << "[" << i << "] ";
            for (int j = 0; j < colCount(i); ++j) {
                cout << "<" << grid_[i][j] << ">";
                if (j + 1 < colCount(i)) cout << " ";
            }
            cout << "\n";
        }
        SetConsoleTextAttribute(h, saved);
    }
};

static void showMenu() {
    cout << "\n--------------------------------\n";
    cout << " 1 - Показать массив\n";
    cout << " 2 - Добавить строку\n";
    cout << " 3 - Добавить элемент в конец строки\n";
    cout << " 4 - Изменить A[i][j]\n";
    cout << " 5 - Удалить по индексу (i, j)\n";
    cout << " 6 - Удалить по значению\n";
    cout << " 7 - Оператор ++ (сдвиг первой буквы)\n";
    cout << " 8 - Сортировка строк\n";
    cout << " 9 - Оператор + (сложить с копией)\n";
    cout << " 0 - Выход\n";
    cout << "--------------------------------\n";
    cout << "Выбор: ";
}

static int readInt() {
    int x;
    while (!(cin >> x)) {
        cin.clear();
        cin.ignore((numeric_limits<streamsize>::max)(), '\n');
        cout << "Введите число: ";
    }
    cin.ignore((numeric_limits<streamsize>::max)(), '\n');
    return x;
}

static string readLine(const string& prompt) {
    cout << prompt;
    string s;
    getline(cin, s);
    return s;
}

static vector<string> splitBySpaces(const string& line) {
    vector<string> out;
    istringstream iss(line);
    string tok;
    while (iss >> tok) out.push_back(tok);
    return out;
}

int main() {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    system("chcp 65001 > nul");

    JaggedArray A;

    cout << "=== ЛАБОРАТОРНАЯ РАБОТА №2 (вариант с меню) ===\n";
    cout << "Введите начальные строки массива (элементы через пробел).\n";
    cout << "Пустая строка — конец ввода.\n\n";

    int idx = 0;
    while (true) {
        string line = readLine("Строка " + to_string(idx) + ": ");
        if (line.empty()) break;
        A.pushLine(splitBySpaces(line));
        ++idx;
    }

    if (A.empty()) {
        cout << "Массив пуст. Завершение.\n";
        return 0;
    }

    int choice = -1;
    while (choice != 0) {
        showMenu();
        choice = readInt();

        try {
            switch (choice) {
            case 1:
                cout << "\nТекущий массив:\n";
                A.render();
                break;

            case 2: {
                string line = readLine("Введите элементы строки через пробел: ");
                A.pushLine(splitBySpaces(line));
                cout << "Строка добавлена.\n";
                A.render();
                break;
            }

            case 3: {
                cout << "Индекс строки k: ";
                int k = readInt();
                string val = readLine("Значение: ");
                if (A.append(k, val))
                    cout << "Добавлено.\n";
                else
                    cout << "Нет такой строки.\n";
                A.render();
                break;
            }

            case 4: {
                cout << "i и j: ";
                int i = readInt(), j = readInt();
                string val = readLine("Новое значение: ");
                A[i][j] = val;
                cout << "Изменено.\n";
                A.render();
                break;
            }

            case 5: {
                cout << "i и j: ";
                int i = readInt(), j = readInt();
                if (A.eraseAt(i, j))
                    cout << "Удалено.\n";
                else
                    cout << "Индекс вне диапазона.\n";
                A.render();
                break;
            }

            case 6: {
                string val = readLine("Значение для удаления: ");
                if (A.eraseValue(val))
                    cout << "Удалено первое вхождение.\n";
                else
                    cout << "Не найдено.\n";
                A.render();
                break;
            }

            case 7:
                ++A;
                cout << "После ++A:\n";
                A.render();
                break;

            case 8:
                A.sortEachLine();
                cout << "После сортировки:\n";
                A.render();
                break;

            case 9: {
                JaggedArray B = A;
                JaggedArray C = A + B;
                cout << "A + A:\n";
                C.render();
                break;
            }

            case 0:
                cout << "Выход.\n";
                break;

            default:
                cout << "Нет такого пункта.\n";
            }
        }
        catch (const exception& e) {
            cout << "Ошибка: " << e.what() << "\n";
        }
    }

    return 0;
}