## 1. Confirm the requirements

“I’ll design a file-search system that can search files by name, extension, or size. It should also support combining filters using AND or OR, and it should recursively search through subdirectories.”

“For simplicity, I’ll keep the filesystem in memory. I’ll first model files and directories, then add filters, and finally implement the search traversal.”

---

## 2. Start with the `File` entity

“The most basic entity is a file. For the current requirements, a file only needs a name, extension, and size.”

“I’m keeping this as a simple data class with getters because filters should read file properties without directly modifying them.”

```cpp
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
```

---

## 3. Introduce the filter abstraction

“Now I need different ways to match a file. Instead of putting all conditions inside the search logic, I’ll create a common `Filter` interface.”

“Every concrete filter implements `match()`. This keeps the search class independent of specific filter types.”

“This also makes the design extensible. For example, later I could add a creation-date filter without modifying the search traversal.”

```cpp
class Filter {
public:
    virtual bool match(File &file) = 0;

    virtual ~Filter() = default;
};
```

---

## 4. Add the name filter

“The first concrete filter matches a file by exact name.”

“The filter stores the expected name, and `match()` compares it with the current file.”

```cpp
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
```

---

## 5. Add the extension filter

“The extension filter follows the same contract. It stores the required extension and checks whether the file has that extension.”

```cpp
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
```

---

## 6. Add the size filter

“Size searching is slightly more complex because the requirement may include operators such as greater than, less than, or equal to.”

“I’ll store both the target size and the comparison operator inside `SizeFilter`.”

“For interview simplicity, I’m representing the operator as a string. In production code, I could replace this with an enum to avoid invalid values.”

```cpp
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
```

---

## 7. Model directories

“Next, I need to represent the filesystem hierarchy.”

“A directory can contain files and other directories, so I’ll create a `FileSystem` class that stores both collections.”

“The structure naturally forms a tree, where the root directory is the starting point.”

```cpp
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
};
```

---

## 8. Add directory operations

“I’ll add simple operations to insert and remove files and to add subdirectories.”

“For the current scope, deleting a file performs a linear search through the files in that directory.”

“If deletion by name were very frequent, I could use an `unordered_map`, but a vector keeps traversal and the interview implementation simple.”

```cpp
// These functions belong inside the FileSystem class.

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
```

---

## 9. Create the search class

“Now I’ll separate the search behavior into a `Search` class.”

“The search object receives the root directory, a collection of filters, and the condition used to combine them.”

“I’m storing `Filter*` because each object may be a different concrete filter, but all of them support the common `match()` operation.”

```cpp
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
};
```

---

## 10. Combine filters using AND or OR

“Before traversing directories, I’ll add a helper method that checks whether one file satisfies the filters.”

“This function belongs to the `Search` class.”

“If there are no filters, every file matches. If there is one filter, I directly evaluate it.”

“For AND, every filter must match. For OR, one successful filter is enough.”

“I’m using short-circuit evaluation, so I stop as soon as the final result is known.”

```cpp
// This function belongs inside the Search class.

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
```

---

## 11. Traverse the filesystem

“Now I’ll implement `findFiles()`, which also belongs to the `Search` class.”

“I’ll use BFS starting from the root directory. A queue stores the directories that still need to be processed.”

“For every directory, I first evaluate its files, and then I add its subdirectories to the queue.”

“DFS would also work here. I’m using BFS because it avoids recursive calls and makes the traversal explicit.”

```cpp
// This function belongs inside the Search class.

vector<string> findFiles() {
    vector<string> result;

    queue<FileSystem*> directories;
    directories.push(root);

    while (!directories.empty()) {
        FileSystem *currentDirectory = directories.front();
        directories.pop();

        for (File *file : currentDirectory->getFiles()) {
            if (checkConditions(*file)) {
                result.push_back(file->getName());
            }
        }

        for (FileSystem *subDirectory :
             currentDirectory->getSubDirectories()) {
            directories.push(subDirectory);
        }
    }

    return result;
}
```

---

## 12. Demonstrate a search

“To demonstrate the design, I’ll create a few files and add them to the root directory.”

“Here I’m searching for files whose extension is `java` OR whose size is at least 10.”

```cpp
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
```

---

## 13. Explain the output

“The size filter matches `abc`, `cde`, `def`, and `uvw` because their sizes are at least 10.”

“The extension filter also matches `uvw` because it is a Java file.”

“Since the condition is OR, a file only needs to satisfy one of the filters.”

---

## 14. Explain the design decisions

“I separated filters from traversal because matching logic and directory traversal are different responsibilities.”

“The `Filter` interface allows the search logic to work with any filter without knowing its concrete type.”

“The directory structure is represented as a tree, and BFS visits every directory and file once.”

“The design also supports adding new filters without changing the `Search` class.”

---

## 15. Complexity

“Let `F` be the total number of files, `D` the total number of directories, and `K` the number of filters.”

“BFS visits every directory and file once, and each file may be checked against up to `K` filters.”

“So the time complexity is `O(D + F × K)`.”

“The queue may contain multiple directories, so the traversal space is `O(D)` in the worst case, excluding the output.”

---

## 16. Edge cases

“I would handle an empty filesystem by returning an empty result.”

“If no filters are provided, the current implementation returns every file.”

“An invalid size operator returns false. In production, I would validate the operator in the constructor or use an enum.”

“If duplicate filenames can exist in different directories, returning only the name may be ambiguous. A follow-up improvement would be returning the complete path.”

---

## 17. Possible follow-ups

“If the interviewer asks for complete paths, I would store the current path along with each directory in the BFS queue.”

“If they ask for NOT conditions or nested expressions such as `A AND (B OR C)`, I would model composite filters such as `AndFilter`, `OrFilter`, and `NotFilter`.”

“If they ask for faster deletion or direct lookup within one directory, I could replace the file vector with an `unordered_map<string, File*>`.”

“If they ask about ownership and memory safety, I would replace raw pointers with `unique_ptr` or `shared_ptr`, depending on the ownership requirements.”
