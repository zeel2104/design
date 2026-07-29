## 1. Clarify the requirements

“I’ll first clarify the expected behavior.

We need to:

* add lockers of different sizes,
* assign a package to an available locker,
* choose a locker that can fit the package,
* and free the locker when the package is removed.

I’ll assume locker and package sizes are small, medium, and large.

I’ll also assume one locker can hold one package at a time.”

```cpp
#include <bits/stdc++.h>
using namespace std;

enum Size {
    SMALL,
    MEDIUM,
    LARGE
};
```

---

## 2. Start with the Package entity

“I’ll start with the simplest entity, `Package`.

A package needs an ID and a size. The size will be used while finding a compatible locker.”

```cpp
class Package {
private:
    string packageId;
    Size size;

public:
    Package(string packageId, Size size) {
        this->packageId = packageId;
        this->size = size;
    }

    string getPackageId() const {
        return packageId;
    }

    Size getSize() const {
        return size;
    }
};
```

---

## 3. Add the Locker entity

“Next, I need a `Locker`.

A locker has an ID, a size, and optionally stores a package.

Instead of maintaining both an occupied flag and a package pointer, I can determine whether the locker is occupied by checking whether a package is assigned.”

```cpp
class Locker {
private:
    string lockerId;
    Size size;
    Package* package;

public:
    Locker(string lockerId, Size size) {
        this->lockerId = lockerId;
        this->size = size;
        this->package = nullptr;
    }

    string getLockerId() const {
        return lockerId;
    }

    Size getSize() const {
        return size;
    }

    bool isAvailable() const {
        return package == nullptr;
    }
};
```

---

## 4. Decide how locker compatibility works

“I’ll allow a package to use a locker of the same or a larger size.

Since the enum values are ordered from small to large, the check is simple.”

```cpp
// This function belongs to the Locker class.
bool canFit(const Package& newPackage) const {
    return isAvailable() && size >= newPackage.getSize();
}
```

The `Locker` class now contains:

```cpp
class Locker {
private:
    string lockerId;
    Size size;
    Package* package;

public:
    Locker(string lockerId, Size size) {
        this->lockerId = lockerId;
        this->size = size;
        this->package = nullptr;
    }

    string getLockerId() const {
        return lockerId;
    }

    Size getSize() const {
        return size;
    }

    bool isAvailable() const {
        return package == nullptr;
    }

    bool canFit(const Package& newPackage) const {
        return isAvailable() && size >= newPackage.getSize();
    }
};
```

---

## 5. Add package assignment and removal

“The locker itself should control its occupancy state.

So `assignPackage` and `removePackage` belong inside the `Locker` class.

I’ll validate the operations so that we cannot assign a package to an occupied or incompatible locker.”

```cpp
// These functions belong to the Locker class.
bool assignPackage(Package* newPackage) {
    if (newPackage == nullptr || !canFit(*newPackage)) {
        return false;
    }

    package = newPackage;
    return true;
}

Package* removePackage() {
    if (isAvailable()) {
        return nullptr;
    }

    Package* removedPackage = package;
    package = nullptr;

    return removedPackage;
}
```

The completed `Locker` class is:

```cpp
class Locker {
private:
    string lockerId;
    Size size;
    Package* package;

public:
    Locker(string lockerId, Size size) {
        this->lockerId = lockerId;
        this->size = size;
        this->package = nullptr;
    }

    string getLockerId() const {
        return lockerId;
    }

    Size getSize() const {
        return size;
    }

    bool isAvailable() const {
        return package == nullptr;
    }

    bool canFit(const Package& newPackage) const {
        return isAvailable() && size >= newPackage.getSize();
    }

    bool assignPackage(Package* newPackage) {
        if (newPackage == nullptr || !canFit(*newPackage)) {
            return false;
        }

        package = newPackage;
        return true;
    }

    Package* removePackage() {
        if (isAvailable()) {
            return nullptr;
        }

        Package* removedPackage = package;
        package = nullptr;

        return removedPackage;
    }
};
```

---

## 6. Add the main management class

“Now I need a class that manages all lockers and coordinates assignment and removal.

I’ll call it `LockerManagementSystem`.

I’ll store lockers in an `unordered_map` because lockers are frequently accessed using their locker ID.”

```cpp
class LockerManagementSystem {
private:
    unordered_map<string, Locker*> lockers;

public:
    ~LockerManagementSystem() {
        for (auto& entry : lockers) {
            delete entry.second;
        }
    }
};
```

---

## 7. Add new lockers

“The `addLocker` function belongs to `LockerManagementSystem`.

I’ll reject duplicate locker IDs because the locker ID should uniquely identify a locker.”

```cpp
// This function belongs to LockerManagementSystem.
bool addLocker(string lockerId, Size size) {
    if (lockers.find(lockerId) != lockers.end()) {
        return false;
    }

    lockers[lockerId] = new Locker(lockerId, size);
    return true;
}
```

---

## 8. Assign a package

“For assignment, I want the smallest available locker that can fit the package.

This prevents a small package from unnecessarily occupying a large locker.

For this interview-sized solution, I’ll scan all lockers and remember the smallest compatible one.”

```cpp
// This function belongs to LockerManagementSystem.
string assignPackage(Package* package) {
    if (package == nullptr) {
        return "";
    }

    Locker* selectedLocker = nullptr;

    for (auto& entry : lockers) {
        Locker* locker = entry.second;

        if (!locker->canFit(*package)) {
            continue;
        }

        if (selectedLocker == nullptr ||
            locker->getSize() < selectedLocker->getSize()) {
            selectedLocker = locker;
        }
    }

    if (selectedLocker == nullptr) {
        return "";
    }

    selectedLocker->assignPackage(package);
    return selectedLocker->getLockerId();
}
```

---

## 9. Track which locker contains each package

“To remove a package, the caller may only know the package ID.

I don’t want to scan every locker, so I’ll maintain a second hash map from package ID to locker ID.

This gives direct lookup during package removal.”

```cpp
class LockerManagementSystem {
private:
    unordered_map<string, Locker*> lockers;

    // package ID -> locker ID
    unordered_map<string, string> packageToLocker;

public:
    // Other functions...
};
```

Now update assignment:

```cpp
// This function belongs to LockerManagementSystem.
string assignPackage(Package* package) {
    if (package == nullptr) {
        return "";
    }

    string packageId = package->getPackageId();

    if (packageToLocker.find(packageId) != packageToLocker.end()) {
        return "";
    }

    Locker* selectedLocker = nullptr;

    for (auto& entry : lockers) {
        Locker* locker = entry.second;

        if (!locker->canFit(*package)) {
            continue;
        }

        if (selectedLocker == nullptr ||
            locker->getSize() < selectedLocker->getSize()) {
            selectedLocker = locker;
        }
    }

    if (selectedLocker == nullptr) {
        return "";
    }

    selectedLocker->assignPackage(package);
    packageToLocker[packageId] = selectedLocker->getLockerId();

    return selectedLocker->getLockerId();
}
```

---

## 10. Free the locker

“The `removePackage` function also belongs to `LockerManagementSystem`.

I’ll use `packageToLocker` to find the locker directly, remove the package from it, and delete the mapping.”

```cpp
// This function belongs to LockerManagementSystem.
Package* removePackage(string packageId) {
    if (packageToLocker.find(packageId) == packageToLocker.end()) {
        return nullptr;
    }

    string lockerId = packageToLocker[packageId];
    Locker* locker = lockers[lockerId];

    Package* removedPackage = locker->removePackage();
    packageToLocker.erase(packageId);

    return removedPackage;
}
```

---

## 11. Complete management class

“At this point, the management class owns the lockers and handles the main use cases.”

```cpp
class LockerManagementSystem {
private:
    unordered_map<string, Locker*> lockers;

    // package ID -> locker ID
    unordered_map<string, string> packageToLocker;

public:
    ~LockerManagementSystem() {
        for (auto& entry : lockers) {
            delete entry.second;
        }
    }

    bool addLocker(string lockerId, Size size) {
        if (lockers.find(lockerId) != lockers.end()) {
            return false;
        }

        lockers[lockerId] = new Locker(lockerId, size);
        return true;
    }

    string assignPackage(Package* package) {
        if (package == nullptr) {
            return "";
        }

        string packageId = package->getPackageId();

        if (packageToLocker.find(packageId) != packageToLocker.end()) {
            return "";
        }

        Locker* selectedLocker = nullptr;

        for (auto& entry : lockers) {
            Locker* locker = entry.second;

            if (!locker->canFit(*package)) {
                continue;
            }

            if (selectedLocker == nullptr ||
                locker->getSize() < selectedLocker->getSize()) {
                selectedLocker = locker;
            }
        }

        if (selectedLocker == nullptr) {
            return "";
        }

        selectedLocker->assignPackage(package);
        packageToLocker[packageId] =
            selectedLocker->getLockerId();

        return selectedLocker->getLockerId();
    }

    Package* removePackage(string packageId) {
        if (packageToLocker.find(packageId) ==
            packageToLocker.end()) {
            return nullptr;
        }

        string lockerId = packageToLocker[packageId];
        Locker* locker = lockers[lockerId];

        Package* removedPackage = locker->removePackage();
        packageToLocker.erase(packageId);

        return removedPackage;
    }
};
```

---

## 12. Demonstrate the flow

“I’ll add a small driver to verify the main flows:

* add lockers,
* assign packages,
* reject assignment when no locker is available,
* and free a locker.”

```cpp
int main() {
    LockerManagementSystem system;

    system.addLocker("L1", SMALL);
    system.addLocker("L2", MEDIUM);
    system.addLocker("L3", LARGE);

    Package package1("P1", SMALL);
    Package package2("P2", MEDIUM);
    Package package3("P3", LARGE);

    string locker1 = system.assignPackage(&package1);
    cout << "P1 assigned to: " << locker1 << endl;

    string locker2 = system.assignPackage(&package2);
    cout << "P2 assigned to: " << locker2 << endl;

    Package* removedPackage = system.removePackage("P1");

    if (removedPackage != nullptr) {
        cout << removedPackage->getPackageId()
             << " removed successfully" << endl;
    }

    string locker3 = system.assignPackage(&package3);
    cout << "P3 assigned to: " << locker3 << endl;

    return 0;
}
```

---

## 13. Explain the complexity

“Adding a locker takes average O(1).

Removing a package takes average O(1) because I maintain a package-to-locker map.

Assigning a package currently takes O(L), where L is the number of lockers, because I scan the lockers to find the smallest compatible one.

For the expected interview scope, this is simple and sufficient.”

---

## 14. Mention a possible optimization

“If assignment becomes very frequent and there are many lockers, I can maintain separate sets or queues of available lockers for each size.

Then I could check the package’s exact size first and move to larger sizes only when required.

I would introduce that optimization only if the interviewer asks for better assignment performance.”

---

## 15. Mention concurrency only as a follow-up

“In a real warehouse, multiple workers could try to assign the same locker concurrently.

I would make locker assignment atomic using a lock or a database transaction.

For this implementation, I’m assuming single-threaded execution.”
