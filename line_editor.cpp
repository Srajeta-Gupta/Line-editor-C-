#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <stack>
#include <direct.h>   

using namespace std;

class Cursor {
public:
    int lineNum;
    int position;
    
    Cursor(int line = -1, int pos = -1) : lineNum(line), position(pos) {}
};

class BufferState {
public:
    vector<string> lines;
    
    BufferState() = default;
    BufferState(const vector<string>& buffer) : lines(buffer) {}
};

class LineEditor {
private:
    static const int MAX_LINES = 25;
    static const int MAX_UNDO = 3;
    
    vector<string> buffer;
    stack<BufferState> undoStack;
    stack<BufferState> redoStack;
    string filepath;
    
    void saveBufferState() {
        if (undoStack.size() >= MAX_UNDO) {
            while (undoStack.size() >= MAX_UNDO) {
                undoStack.pop();
            }
        }
        undoStack.push(BufferState(buffer));
        while (!redoStack.empty()) {
            redoStack.pop();
        }
    }

public:
    LineEditor(const string& path) : filepath(path) {
        buffer.reserve(MAX_LINES);
    }
    
    bool openFile() {
        ifstream file(filepath.c_str());
        if (!file) {
            // Try to create the file if it doesn't exist
            ofstream newFile(filepath.c_str());
            if (!newFile) {
                return false;
            }
            newFile.close();
            return true;
        }
        
        string line;
        while (getline(file, line) && buffer.size() < MAX_LINES) {
            buffer.push_back(line);
        }
        
        file.close();
        return true;
    }
    
    bool saveFile() {
        ofstream file(filepath.c_str());
        if (!file) {
            return false;
        }
        
        for (const auto& line : buffer) {
            file << line << '\n';
        }
        
        file.close();
        return true;
    }
    
    Cursor searchWord(const string& word) {
        for (size_t i = 0; i < buffer.size(); ++i) {
            size_t found = buffer[i].find(word);
            if (found != string::npos) {
                return Cursor(i, found);
            }
        }
        return Cursor();
    }
    
    bool insertLine(int position, const string& text) {
        if (buffer.size() >= MAX_LINES || position > static_cast<int>(buffer.size())) {
            return false;
        }
        
        saveBufferState();
        buffer.insert(buffer.begin() + position, text);
        return true;
    }
    
    bool insertWord(const Cursor& cursor, const string& word) {
        if (cursor.lineNum < 0 || cursor.lineNum >= static_cast<int>(buffer.size())) {
            return false;
        }
        
        saveBufferState();
        buffer[cursor.lineNum].insert(cursor.position, word);
        return true;
    }
    
    void displayLine(int lineNum) const {
        if (lineNum >= 0 && lineNum < static_cast<int>(buffer.size())) {
            cout << "[Line No" << lineNum + 1 << ":] \"" << buffer[lineNum] << "\"\n";
        }
    }
    
    void displayBuffer() const {
        for (size_t i = 0; i < buffer.size(); ++i) {
            displayLine(i);
        }
    }
    
    bool updateWord(const Cursor& cursor, const string& oldWord, const string& newWord) {
        if (cursor.lineNum < 0 || cursor.lineNum >= static_cast<int>(buffer.size())) {
            return false;
        }
        
        size_t found = buffer[cursor.lineNum].find(oldWord, cursor.position);
        if (found != string::npos) {
            saveBufferState();
            buffer[cursor.lineNum].replace(found, oldWord.length(), newWord);
            return true;
        }
        return false;
    }
    
    bool deleteLine(int lineNum) {
        if (lineNum < 0 || lineNum >= static_cast<int>(buffer.size())) {
            return false;
        }
        
        saveBufferState();
        buffer.erase(buffer.begin() + lineNum);
        return true;
    }
    
    bool deleteWord(const Cursor& cursor, const string& word) {
        if (cursor.lineNum < 0 || cursor.lineNum >= static_cast<int>(buffer.size())) {
            return false;
        }
        
        size_t found = buffer[cursor.lineNum].find(word, cursor.position);
        if (found != string::npos) {
            saveBufferState();
            buffer[cursor.lineNum].erase(found, word.length());
            return true;
        }
        return false;
    }
    
    bool undo() {
        if (undoStack.empty()) {
            cout << "Nothing to undo\n";
            return false;
        }
        
        redoStack.push(BufferState(buffer));
        buffer = undoStack.top().lines;
        undoStack.pop();
        return true;
    }
    
    bool redo() {
        if (redoStack.empty()) {
            cout << "Nothing to redo\n";
            return false;
        }
        
        undoStack.push(BufferState(buffer));
        buffer = redoStack.top().lines;
        redoStack.pop();
        return true;
    }
};

int main(int argc, char* argv[]) {
    string filename = "file.txt";
    string directory = ".";
    
    
    switch (argc) {
        case 1:
            
            break;
            
        case 2:
            
            filename = argv[1];
            break;
            
        case 3:
            
            filename = argv[1];
            directory = argv[2];
            
            _mkdir(directory.c_str());
            break;
            
        default:
            cout << "Error: Too many arguments\n";
            cout << "Usage: " << argv[0] << " [filename] [directory]\n";
            return 1;
    }

    string filepath = directory + "/" + filename;
    
    
    LineEditor editor(filepath);
    
    
    if (!editor.openFile()) {
        cout << "Error: Cannot create/open file " << filepath << endl;
        return 1;
    }
    
    // Print commands help
    cout << "Line Editor Commands:\n"
         << "display - Show buffer contents\n"
         << "insert <line_number> <text> - Insert line\n"
         << "search <word> - Search for word\n"
         << "update <old_word> <new_word> - Replace word\n"
         << "delete <line_number> - Delete line\n"
         << "undo - Undo last operation\n"
         << "redo - Redo last operation\n"
         << "save - Save changes\n"
         << "exit - Exit editor\n\n";
    
    // Main editor loop
    string command;
    while (true) {
        cout << "Enter command: ";
        cin >> command;
        
        if (command == "exit") {
            break;
        }
        else if (command == "display") {
            editor.displayBuffer();
        }
        else if (command == "insert") {
            int lineNum;
            string text;
            cin >> lineNum;
            cin.ignore(); // Consume newline
            getline(cin, text);
            if (!editor.insertLine(lineNum - 1, text)) {
                cout << "Failed to insert line\n";
            }
        }
        else if (command == "search") {
            string word;
            cin >> word;
            Cursor cursor = editor.searchWord(word);
            if (cursor.lineNum != -1) {
                cout << "Found at line " << cursor.lineNum + 1 
                     << ", position " << cursor.position << endl;
            } else {
                cout << "Word not found\n";
            }
        }
        else if (command == "update") {
            string oldWord, newWord;
            cin >> oldWord >> newWord;
            Cursor cursor = editor.searchWord(oldWord);
            if (cursor.lineNum != -1) {
                if (!editor.updateWord(cursor, oldWord, newWord)) {
                    cout << "Failed to update word\n";
                }
            } else {
                cout << "Word not found\n";
            }
        }
        else if (command == "delete") {
            int lineNum;
            cin >> lineNum;
            if (!editor.deleteLine(lineNum - 1)) {
                cout << "Failed to delete line\n";
            }
        }
        else if (command == "undo") {
            editor.undo();
        }
        else if (command == "redo") {
            editor.redo();
        }
        else if (command == "save") {
            if (editor.saveFile()) {
                cout << "Changes saved\n";
            } else {
                cout << "Failed to save changes\n";
            }
        }
        else {
            cout << "Unknown command\n";
        }
    }
    
    return 0;
}