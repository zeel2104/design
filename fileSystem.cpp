#include <bits/stdc++.h>

using namespace std;

/*
    Requirements:
        - Search files by name
        - Search files by extension
        - Search files by size
        - Combine multiple filters using AND / OR
        - Search through directories and subdirectories
*/

class File {
    string name;
    string extension;
    int size;

public:
    File(string fileName, string fileExtension, int fileSize) {
        name = fileName;
        extension = fileExtension;
        size = fileSize;
    }

    string getName() {
        return name;
    }

    string getExtension() {
        return extension;
    }

    int getSize() {
        return size;
    }
};


/*
    Filter interface

    Every filter must implement match().
*/
class Filter {
public:
    virtual bool match(File &file) = 0;

    virtual ~Filter() = default;
};


/*
    Matches a file based on its name.
*/
class NameFilter : public Filter {
    string name;

public:
    NameFilter(string fileName) {
        name = fileName;
    }

    bool match(File &file) {
        return file.getName() == name;
    }
};


/*
    Matches a file based on its extension.
*/
class ExtensionFilter : public Filter {
    string extension;

public:
    ExtensionFilter(string fileExtension) {
        extension = fileExtension;
    }

    bool match(File &file) {
        return file.getExtension() == extension;
    }
};


/*
    Matches a file based on its size.

    Supported operators:
        >
        >=
        <
        <=
        ==
        !=
*/
class SizeFilter : public Filter {
    int size;
    string operation;

public:
    SizeFilter(int fileSize, string op) {
        size = fileSize;
        operation = op;
    }

    bool match(File &file) {
        if (operation == ">")
            return file.getSize() > size;

        if (operation == ">=")
            return file.getSize() >= size;

        if (operation == "<")
            return file.getSize() < size;

        if (operation == "<=")
            return file.getSize() <= size;

        if (operation == "==")
            return file.getSize() == size;

        if (operation == "!=")
            return file.getSize() != size;

        return false;
    }
};


/*
    Represents a directory.

    Every directory can contain:
        - Files
        - Subdirectories
*/
class FileSystem {
    string name;
    bool isDirectory;

    vector<File*> files;
    vector<FileSystem*> subDirectories;

public:
    FileSystem(string directoryName, bool directory = true) {
        name = directoryName;
        isDirectory = directory;
    }

    string getName() {
        return name;
    }

    vector<File*>& getFiles() {
        return files;
    }

    vector<FileSystem*>& getSubDirectories() {
        return subDirectories;
    }

    void addFile(File *file) {
        files.push_back(file);
    }

    void deleteFile(string fileName) {
        for (int i = 0; i < files.size(); i++) {
            if (files[i]->getName() == fileName) {
                files.erase(files.begin() + i);
                return;
            }
        }
    }

    void addSubDirectory(FileSystem *directory) {
        subDirectories.push_back(directory);
    }
};


/*
    Search class:
        - Traverses directories using BFS
        - Applies filters
        - Supports AND / OR conditions
*/
class Search {
    FileSystem *root;
    vector<Filter*> filters;
    string condition;

public:
    Search(
        FileSystem *fileSystem,
        vector<Filter*> searchFilters,
        string searchCondition = ""
    ) {
        root = fileSystem;
        filters = searchFilters;
        condition = searchCondition;
    }

    bool checkConditions(File &file) {
        if (filters.size() == 0)
            return true;

        if (filters.size() == 1)
            return filters[0]->match(file);

        if (condition == "AND") {
            for (Filter *filter : filters) {
                if (!filter->match(file))
                    return false;
            }

            return true;
        }

        // Default condition is OR.
        for (Filter *filter : filters) {
            if (filter->match(file))
                return true;
        }

        return false;
    }

    vector<string> findFiles() {
        vector<string> result;

        queue<FileSystem*> directories;
        directories.push(root);

        // BFS traversal
        while (!directories.empty()) {
            FileSystem *currentDirectory = directories.front();
            directories.pop();

            // Search files in the current directory
            for (File *file : currentDirectory->getFiles()) {
                if (checkConditions(*file)) {
                    result.push_back(file->getName());
                }
            }

            // Add subdirectories to the queue
            for (FileSystem *subDirectory :
                 currentDirectory->getSubDirectories()) {
                directories.push(subDirectory);
            }
        }

        return result;
    }
};


int main() {
    File f1("abc", "txt", 10);
    File f2("cde", "txt", 20);
    File f3("def", "pdf", 30);
    File f4("ghi", "py", 5);
    File f5("uvw", "java", 10);

    FileSystem root("/");

    root.addFile(&f1);
    root.addFile(&f2);
    root.addFile(&f3);
    root.addFile(&f4);
    root.addFile(&f5);

    /*
        Example 1:
        Search file by name
    */

    // NameFilter nameFilter("abc");
    // Search search(&root, {&nameFilter});


    /*
        Example 2:
        Search files having size >= 10
    */

    // SizeFilter sizeFilter(10, ">=");
    // Search search(&root, {&sizeFilter});


    /*
        Example 3:
        Extension is "java" OR size >= 10
    */

    ExtensionFilter extensionFilter("java");
    SizeFilter sizeFilter(10, ">=");

    vector<Filter*> filters = {
        &extensionFilter,
        &sizeFilter
    };

    Search search(&root, filters, "OR");

    vector<string> result = search.findFiles();

    for (string fileName : result) {
        cout << fileName << endl;
    }

    return 0;
}