# Locker Management System — 30-Minute Interview Version

## 1. Basic intuition

What I would say:

> “The system should add lockers of different sizes, assign a package to the smallest available locker that can fit it, and free the locker when the package is removed.
>
> I’ll keep `Package` and `Locker` as simple entities.
>
> `LockerManagementSystem` will store all lockers and coordinate assignment and removal.(Adds lockers
Finds a suitable locker
Assigns packages
Removes packages)
>

1.“Then I’ll assign the smallest available locker that can fit the package, so larger lockers are preserved.”

2. Should the system choose any compatible locker, or the smallest compatible locker?”

3. Can the same package not be assigned to more than one locker?”

4. Are locker IDs and package IDs unique?

5. Do we only need small, medium, and large sizes for now?”

> I’ll also maintain a package-to-locker map so package removal can be done without scanning every locker.”
>“I have two simple entities: Package and Locker. The LockerManagementSystem coordinates the operations. I store lockers in a hash map for fast lookup and maintain a package-to-locker map so package removal is average O(1). During assignment, I scan the lockers and choose the smallest available locker that can fit the package. This avoids wasting larger lockers. Assignment is O(L), while adding lockers and removing packages are average O(1).”
>
> Package Class stores packageid and size
>Locker class stores lockerId, size, Package* package(The package pointer tells us whether the locker is occupied:)

---

## 2. Main classes

> “I need:
>
> - `Size` enum
> - `Package`
> - `Locker`
> - `LockerManagementSystem`”

---

# 3. Build the solution step by step

## Step 1: Define locker and package sizes

What I would say:

> “Both packages and lockers use the same size values. Since the enum is ordered from small to large, checking whether a locker can fit a package becomes simple.”

```cpp
enum Size {
    SMALL,
    MEDIUM,
    LARGE
};
```

---

## Step 2: Create the `Package` entity

What I would say:

> “A package only needs an ID and its size.”

```cpp
class Package {
    string packageId;
    Size size;

public:
    Package(string packageId, Size size) {
        this->packageId = packageId;
        this->size = size;
    }

    string getPackageId() {
        return packageId;
    }

    Size getSize() {
        return size;
    }
};
```

---

## Step 3: Create the `Locker` entity

What I would say:

> “A locker stores its ID, size, and the package currently assigned to it.
>
> If the package pointer is `nullptr`, the locker is available.”

```cpp
class Locker {
    string lockerId;
    Size size;
    Package* package;
    //Pointers allow the same objects to be referenced without copying them.

public:
    Locker(string lockerId, Size size) {
        this->lockerId = lockerId;
        this->size = size;
        package = nullptr;
    }

    string getLockerId() {
        return lockerId;
    }

    Size getSize() {
        return size;
    }

    bool isAvailable() {
        return package == nullptr;
    }
};
```

---

## Step 4: Check whether a package fits

> “A package can use a locker of the same size or a larger size.
>
> The locker must also be free.”

```cpp
bool canFit(Package* newPackage) {
    return isAvailable() &&
           size >= newPackage->getSize();
}
```

This method belongs inside `Locker`.

---

## Step 5: Assign and remove a package

What I would say:

> “The locker itself should control its occupancy state.”

```cpp
bool assignPackage(Package* newPackage) {
    if (newPackage == nullptr || !canFit(newPackage)) {
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

These methods belong inside `Locker`.

---

## Step 6: Create the management class

What I would say:

> “The main class stores lockers by locker ID and also stores which locker contains each package.”

```cpp
class LockerManagementSystem {
   // locker ID → Locker object
    // locker ID -> locker
    //This provides average O(1) locker lookup.
    unordered_map<string, Locker*> lockers;
    //This lets us immediately determine which locker contains a package.

W   //ithout this second map, removing a package would require scanning every locker.

    //"P1" → "L1"
    

    // package ID -> locker ID
    unordered_map<string, string> packageToLocker;
};
```

### Design decision

> “The locker map gives fast locker lookup.
>
> The package-to-locker map makes package removal average O(1).”

---

## Step 7: Add a locker

What I would say:

> “Adding a locker is a simple map insertion. Duplicate locker IDs are rejected.”

```cpp

//If the package ID already exists in the map, it is already inside a locker.

//This prevents the same package from being assigned twice.
bool addLocker(string lockerId, Size size) {
    if (lockers.count(lockerId)) {
        return false;
    }

    lockers[lockerId] = new Locker(lockerId, size);
    return true;
}
```

This method belongs inside `LockerManagementSystem`.

---

## Step 8: Assign a package

What I would say:

> “I’ll scan all lockers and choose the smallest available locker that can fit the package.
>
> This avoids wasting a large locker on a small package.”

```cpp
string assignPackage(Package* package) {
    if (package == nullptr) {
        return "";
    }

    string packageId = package->getPackageId();

    // The same package cannot be assigned twice.
    if (packageToLocker.count(packageId)) {
        return "";
    }

    Locker* selectedLocker = nullptr;
    //The system loops through all lockers:
    for (auto& entry : lockers) {
        Locker* locker = entry.second;
    //It ignores lockers that cannot fit the package:

        if (!locker->canFit(package)) {
            continue;
        }
    //Then it selects the smallest compatible locker:

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
```

---

## Step 9: Remove a package

What I would say:

> “To remove a package, I first use the package-to-locker map, then ask that locker to remove it.”

```cpp
Package* removePackage(string packageId) {
    if (!packageToLocker.count(packageId)) {
        return nullptr;
    }

    //First, find the locker ID:

    string lockerId = packageToLocker[packageId];
    //Then find the locker:
    Locker* locker = lockers[lockerId];

    Package* removedPackage = locker->removePackage();

    //packageToLocker.erase(packageId);

    packageToLocker.erase(packageId);

    return removedPackage;
}
```

---

# 4. Complete code

```cpp
#include <bits/stdc++.h>
using namespace std;

/*
    Shared size values for packages and lockers.
*/
enum Size {
    SMALL,
    MEDIUM,
    LARGE
};


/*
    Represents one package.
*/
class Package {
    string packageId;
    Size size;

public:
    Package(string packageId, Size size) {
        this->packageId = packageId;
        this->size = size;
    }

    string getPackageId() {
        return packageId;
    }

    Size getSize() {
        return size;
    }
};


/*
    Represents one locker.
*/
class Locker {
    string lockerId;
    Size size;

    // nullptr means the locker is free.
    Package* package;

public:
    Locker(string lockerId, Size size) {
        this->lockerId = lockerId;
        this->size = size;
        package = nullptr;
    }

    string getLockerId() {
        return lockerId;
    }

    Size getSize() {
        return size;
    }

    bool isAvailable() {
        return package == nullptr;
    }

    /*
        A locker can fit a package of the same
        or a smaller size.
    */
    bool canFit(Package* newPackage) {
        return isAvailable() &&
               size >= newPackage->getSize();
    }

    /*
        Assigns a package if the locker is compatible.
    */
    bool assignPackage(Package* newPackage) {
        if (newPackage == nullptr ||
            !canFit(newPackage)) {
            return false;
        }

        package = newPackage;
        return true;
    }

    /*
        Frees the locker and returns the package.
    */
    Package* removePackage() {
        if (isAvailable()) {
            return nullptr;
        }

        Package* removedPackage = package;
        package = nullptr;

        return removedPackage;
    }
};


/*
    Main class that manages lockers and packages.
*/
class LockerManagementSystem {
    // locker ID -> locker
    unordered_map<string, Locker*> lockers;

    // package ID -> locker ID
    unordered_map<string, string> packageToLocker;

public:
    /*
        Adds a new locker.
    */
    bool addLocker(string lockerId, Size size) {
        if (lockers.count(lockerId)) {
            return false;
        }

        lockers[lockerId] =
            new Locker(lockerId, size);

        return true;
    }

    /*
        Assigns the smallest compatible locker.
    */
    string assignPackage(Package* package) {
        if (package == nullptr) {
            return "";
        }

        string packageId = package->getPackageId();

        // Prevent duplicate assignment.
        if (packageToLocker.count(packageId)) {
            return "";
        }

        Locker* selectedLocker = nullptr;

        for (auto& entry : lockers) {
            Locker* locker = entry.second;

            if (!locker->canFit(package)) {
                continue;
            }

            if (selectedLocker == nullptr ||
                locker->getSize() <
                selectedLocker->getSize()) {
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

    /*
        Removes a package and frees its locker.
    */
    Package* removePackage(string packageId) {
        if (!packageToLocker.count(packageId)) {
            return nullptr;
        }

        string lockerId =
            packageToLocker[packageId];

        Locker* locker = lockers[lockerId];

        Package* removedPackage =
            locker->removePackage();

        packageToLocker.erase(packageId);

        return removedPackage;
    }

    //Lockers are dynamically created:, Therefore, they must eventually be deleted:
    //This prevents memory leaks.The system does not delete packages because packages are created outside the system in main().
    ~LockerManagementSystem() {
        for (auto& entry : lockers) {
            delete entry.second;
        }
    }
};


int main() {
    LockerManagementSystem system;

    system.addLocker("L1", SMALL);
    system.addLocker("L2", MEDIUM);
    system.addLocker("L3", LARGE);

    Package p1("P1", SMALL);
    Package p2("P2", MEDIUM);
    Package p3("P3", LARGE);

    cout << "P1 assigned to: "
         << system.assignPackage(&p1)
         << endl;

    cout << "P2 assigned to: "
         << system.assignPackage(&p2)
         << endl;

    Package* removed =
        system.removePackage("P1");

    if (removed != nullptr) {
        cout << removed->getPackageId()
             << " removed successfully"
             << endl;
    }

    cout << "P3 assigned to: "
         << system.assignPackage(&p3)
         << endl;

    return 0;
}
```

---

# 5. Complexity

Let `L` be the number of lockers.

What I would say:

> “Adding a locker takes average O(1).
>
> Assigning a package takes O(L) because I scan all lockers to find the smallest compatible one.
>
> Removing a package takes average O(1) because I maintain the package-to-locker map.
>
> The storage complexity is O(L + P), where P is the number of assigned packages.”

---

# 6. Important design decisions

## Why does `Locker` store the package?

> “The locker owns its occupancy state. If the package pointer is null, the locker is free.”

## Why choose the smallest compatible locker?

> “It preserves larger lockers for packages that actually need them.”

## Why use two hash maps?

> “One map stores lockers by locker ID.
>
> The second map directly finds the locker containing a package.”

## Why use raw pointers?

> “For this 30-minute interview version, packages and lockers are created outside and remain alive during the required operations.
>
> In production, I would use smart pointers or explicit ownership.”

## Why not use an abstract class?

> “The current requirements have only one kind of locker assignment behavior.
>
> I would not add inheritance unless the interviewer asks for different assignment strategies.”

---

# 7. Follow-up requirements

## Faster assignment

What I would say:

> “I would keep separate sets or queues of available lockers for each size.
>
> Then I could check the package’s exact size first and only move to larger sizes when needed.”

## Different assignment strategies

What I would say:

> “I could introduce an abstract `LockerAssignmentStrategy` with implementations such as smallest-fit or nearest-locker.”

## Expired packages

What I would say:

> “I would store an expiry time with the assignment and add a cleanup process that frees expired lockers.”

## Concurrency

What I would say:

> “Assignment must be atomic so two requests cannot select the same locker.
>
> In production, I would use locking or a database transaction.”

---

# 8. Final interview summary

What I would say:

> “`Package` and `Locker` are simple entities.
>
> `LockerManagementSystem` adds lockers, assigns the smallest compatible locker, and removes packages.
>
> A package-to-locker map makes removal average O(1).
>
> Assignment is O(L) in this simple interview version and can be optimized later using separate available-locker collections by size.”
