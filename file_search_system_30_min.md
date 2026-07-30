# File Search System — 30-Minute Interview Version

## 1. Basic intuition

What I would say:

> “The system should search files by name, extension, or size. It should also support combining filters using AND or OR and search inside subdirectories.
>
> I’ll represent every search condition using one abstract `Filter` class.
>
> `NameFilter`, `ExtensionFilter`, and `SizeFilter` will implement their own matching logic.
>
> A `Directory` stores files and subdirectories.
>
> The `Search` class traverses all directories using BFS and applies the selected filters.”

---

## 2. Main classes

> “I need:
>
> - `File` to store file information
> - `Filter` as the abstract class
> - `NameFilter`, `ExtensionFilter`, and `SizeFilter`
> - `Directory` to store files and subdirectories
> - `Search` to traverse directories and apply filters”

---

# 3. Build the solution step by step

## Step 1: Create the `File` entity

What I would say:

> “A file only needs its name, extension, and size for the current requirements.”

```cpp
class File {
    string name;
    string extension;
    int size;

public:
    File(string name, string extension, int size) {
        this->name = name;
        this->extension = extension;
        this->size = size;
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

## Step 2: Create one abstract `Filter` class

What I would say:

> “Different search conditions have different matching logic, but all of them answer the same question: does this file match?
>
> So I’ll define one abstract `Filter` class with a `match()` method.”

```cpp
class Filter {
public:
    virtual bool match(File& file) = 0;
    virtual ~Filter() = default;
};
```

### Design decision

> “This is the Strategy pattern. Each filter contains one search strategy, and the search logic works with the common `Filter` interface.”

---

## Step 3: Add name and extension filters

What I would say:

> “Each filter stores the expected value and compares it with the file.”

```cpp
class NameFilter : public Filter {
    string name;

public:
    NameFilter(string name) {
        this->name = name;
    }

    bool match(File& file) override {
        return file.getName() == name;
    }
};

class ExtensionFilter : public Filter {
    string extension;

public:
    ExtensionFilter(string extension) {
        this->extension = extension;
    }

    bool match(File& file) override {
        return file.getExtension() == extension;
    }
};
```

---

## Step 4: Add the size filter

What I would say:

> “The size filter also stores an operator because the interviewer may ask for conditions such as greater than, less than, or equal to.”

```cpp
class SizeFilter : public Filter {
    int size;
    string operation;

public:
    SizeFilter(int size, string operation) {
        this->size = size;
        this->operation = operation;
    }

    bool match(File& file) override {
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

        return false;
    }
};
```

### Design decision

> “I am keeping the operator as a string because it is simple to type in an interview. An enum would be safer in production.”

---

## Step 5: Represent directories

What I would say:

> “A directory can contain files and more directories, so I store both in vectors.”

```cpp
class Directory {
    string name;
    vector<File*> files;
    vector<Directory*> subDirectories;

public:
    Directory(string name) {
        this->name = name;
    }

    void addFile(File* file) {
        files.push_back(file);
    }

    void addSubDirectory(Directory* directory) {
        subDirectories.push_back(directory);
    }

    vector<File*>& getFiles() {
        return files;
    }

    vector<Directory*>& getSubDirectories() {
        return subDirectories;
    }
};
```

### Design decision

> “I use pointers because the directory stores references to existing file and directory objects. For a short interview solution, I avoid ownership and smart-pointer complexity.”

---

## Step 6: Apply AND and OR conditions

What I would say:

> “The search object receives a list of filters and a condition.
>
> For AND, every filter must match.
>
> For OR, at least one filter must match.”

```cpp
bool matches(File& file) {
    if (filters.empty())
        return true;

    if (condition == "AND") {
        for (Filter* filter : filters) {
            if (!filter->match(file))
                return false;
        }

        return true;
    }

    // Default behavior is OR.
    for (Filter* filter : filters) {
        if (filter->match(file))
            return true;
    }

    return false;
}
```

This method belongs inside `Search`.

---

## Step 7: Search through directories

What I would say:

> “I’ll use BFS with a queue. I process files in the current directory and push its subdirectories into the queue.”

```cpp
vector<string> findFiles() {
    vector<string> result;
    queue<Directory*> pending;

    pending.push(root);

    while (!pending.empty()) {
        Directory* current = pending.front();
        pending.pop();

        // Check every file in this directory.
        for (File* file : current->getFiles()) {
            if (matches(*file)) {
                result.push_back(file->getName());
            }
        }

        // Visit subdirectories later.
        for (Directory* directory :
             current->getSubDirectories()) {
            pending.push(directory);
        }
    }

    return result;
}
```

---

# 4. Complete code

```cpp
#include <bits/stdc++.h>
using namespace std;

/*
    Stores the searchable properties of one file.
*/
class File {
    string name;
    string extension;
    int size;

public:
    File(string name, string extension, int size) {
        this->name = name;
        this->extension = extension;
        this->size = size;
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
    Abstract filter.

    Every search condition must implement match().
*/
class Filter {
public:
    virtual bool match(File& file) = 0;
    virtual ~Filter() = default;
};


/*
    Matches a file by exact name.
*/
class NameFilter : public Filter {
    string name;

public:
    NameFilter(string name) {
        this->name = name;
    }

    bool match(File& file) override {
        return file.getName() == name;
    }
};


/*
    Matches a file by extension.
*/
class ExtensionFilter : public Filter {
    string extension;

public:
    ExtensionFilter(string extension) {
        this->extension = extension;
    }

    bool match(File& file) override {
        return file.getExtension() == extension;
    }
};


/*
    Matches a file using a size condition.
*/
class SizeFilter : public Filter {
    int size;
    string operation;

public:
    SizeFilter(int size, string operation) {
        this->size = size;
        this->operation = operation;
    }

    bool match(File& file) override {
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

        return false;
    }
};


/*
    Represents one directory.

    A directory contains files and subdirectories.
*/
class Directory {
    string name;
    vector<File*> files;
    vector<Directory*> subDirectories;

public:
    Directory(string name) {
        this->name = name;
    }

    void addFile(File* file) {
        files.push_back(file);
    }

    void addSubDirectory(Directory* directory) {
        subDirectories.push_back(directory);
    }

    vector<File*>& getFiles() {
        return files;
    }

    vector<Directory*>& getSubDirectories() {
        return subDirectories;
    }
};


/*
    Traverses directories and applies filters.
*/
class Search {
    Directory* root;
    vector<Filter*> filters;
    string condition;

    /*
        Checks whether a file satisfies the filters.
    */
    bool matches(File& file) {
        if (filters.empty())
            return true;

        if (condition == "AND") {
            // Every filter must match.
            for (Filter* filter : filters) {
                if (!filter->match(file))
                    return false;
            }

            return true;
        }

        // Default condition is OR.
        for (Filter* filter : filters) {
            if (filter->match(file))
                return true;
        }

        return false;
    }

public:
    Search(Directory* root,
           vector<Filter*> filters,
           string condition = "OR") {
        this->root = root;
        this->filters = filters;
        this->condition = condition;
    }

    /*
        Searches the complete directory tree using BFS.
    */
    vector<string> findFiles() {
        vector<string> result;
        queue<Directory*> pending;

        pending.push(root);

        while (!pending.empty()) {
            Directory* current = pending.front();
            pending.pop();

            // Check files in the current directory.
            for (File* file : current->getFiles()) {
                if (matches(*file)) {
                    result.push_back(file->getName());
                }
            }

            // Add subdirectories for later processing.
            for (Directory* directory :
                 current->getSubDirectories()) {
                pending.push(directory);
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

    Directory root("/");
    Directory documents("documents");

    root.addFile(&f1);
    root.addFile(&f2);
    root.addFile(&f3);

    documents.addFile(&f4);
    documents.addFile(&f5);

    root.addSubDirectory(&documents);

    /*
        Search:
        extension is "java" OR size >= 10
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
```

---

# 5. Complexity

Let:

- `D` be the number of directories
- `F` be the total number of files
- `K` be the number of filters

What I would say:

> “BFS visits every directory and every file once.
>
> For every file, it may evaluate all K filters.
>
> Therefore, the time complexity is O(D + F × K).
>
> The BFS queue can store up to O(D) directories, and the result can contain up to O(F) file names.”

---

# 6. Important design decisions

## Why use an abstract `Filter` class?

> “Name, extension, and size have different matching logic, but the search class should treat all of them in the same way.
>
> The abstract class lets me add another filter later without changing the traversal logic.”

## Why is `Search` a class instead of a function?

> “Search has state: the root directory, filters, and AND/OR condition.
>
> Keeping them together in one class avoids passing the same arguments through multiple helper functions.”

## Why use BFS?

> “Both BFS and DFS work because I only need to visit every directory.
>
> I chose BFS because the queue-based traversal is simple and avoids recursive calls.”

## Why not combine files and directories into one abstract node?

> “That would model a real file system more closely using the Composite pattern, but it adds more inheritance and code.
>
> For a 30-minute interview, separate `File` and `Directory` classes are enough for the stated requirements.”

## Why not use smart pointers?

> “For this interview version, objects are created in `main()` and remain alive during the search.
>
> In production, I would use smart pointers to make ownership explicit.”

---

# 7. Follow-up requirements

## Add another filter

What I would say:

> “I can create another class derived from `Filter` and implement `match()`. The `Search` class does not change.”

Example:

```cpp
class PrefixFilter : public Filter {
    string prefix;

public:
    PrefixFilter(string prefix) {
        this->prefix = prefix;
    }

    bool match(File& file) override {
        return file.getName().find(prefix) == 0;
    }
};
```

---

## Support nested AND and OR expressions

What I would say:

> “The current design supports one global AND or OR condition.
>
> For expressions such as `(name AND size) OR extension`, I would create composite filters like `AndFilter` and `OrFilter`, where each composite contains other filters.”

---

## Return complete file paths

What I would say:

> “I would also store the current directory path in the BFS queue and append the file name when a match is found.”

---

# 8. Final interview summary

What I would say:

> “I modeled each search rule using one abstract `Filter` class.
>
> Concrete filters implement name, extension, and size matching.
>
> A directory stores files and subdirectories.
>
> The `Search` class traverses the directory tree using BFS and combines filters using AND or OR.
>
> The design is simple enough for a 30-minute interview and allows new filters to be added without changing the search traversal.”
