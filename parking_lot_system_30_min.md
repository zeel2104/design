# Parking Lot System — 30-Minute Interview Version

## 1. Basic intuition

What I would say:

> “The parking lot has multiple floors and supports two vehicle types: 2-wheelers and 4-wheelers.
>
> I need to park a vehicle, remove it, search using vehicle number or ticket ID, and return the number of free spots.
>
> I’ll keep one `ParkingFloor` object per floor. Each floor stores its parking spots and free-spot counts.
>
> A `SearchManager` stores mappings from vehicle number and ticket ID to spot ID.
>
> `Helper` is an abstract class because spot-ID generation and location parsing are platform-specific.”

---

## 2. Main classes

> “I need:
>
> - `Helper` as the abstract utility class
> - `ParkingSpot` for one spot
> - `SearchManager` for vehicle and ticket lookup
> - `ParkingFloor` for floor-level parking logic
> - `Solution` as the main parking-lot controller”

---

# 3. Build the solution step by step

## Step 1: Abstract helper

What I would say:

> “The platform decides how spot IDs are generated and converted back into floor, row, and column.
>
> Therefore, I’ll keep that behavior behind an abstract `Helper` class.”
>The parking system does not know how the platform formats spot IDs.

One platform may use:

F1-R2-C3

Another may use:

1-2-3
>So Helper hides this platform-specific logic from the parking classes.

```cpp
class Helper {
public:

    //floor, row, column → spot ID
    virtual string get_spot_id(
        int floor,
        int row,
        int col
    ) = 0;

    //spot ID → floor, row, column

    virtual vector<int> get_spot_location(
        const string& spot_id
    ) = 0;

    virtual ~Helper() = default;
};
```

---

## Step 2: Parking spot

What I would say:

> “Each parking spot stores its ID, supported vehicle type, and whether it is occupied.”

```cpp
class ParkingSpot {
    string spotId;
    int vehicleType;
    bool occupied;

public:
    ParkingSpot(string spotId, int vehicleType) {
        this->spotId = spotId;
        this->vehicleType = vehicleType;
        occupied = false;
    }

    bool isOccupied() {
        return occupied;
    }

    void parkVehicle() {
        occupied = true;
    }

    void removeVehicle() {
        occupied = false;
    }

    string getSpotId() {
        return spotId;
    }

    int getVehicleType() {
        return vehicleType;
    }
};
```

---

## Step 3: Search manager

What I would say:

> “To search quickly, I’ll map both vehicle number and ticket ID to the assigned spot ID.”

```cpp
class SearchManager {

    //vehicle number → spot ID
    //ticket ID → spot ID
    //"VA1234" → "F0-R1-C2"
    //"T100"   → "F0-R1-C2"
    unordered_map<string, string> spotByKey;



public:
    void add(string vehicleNumber,
             string ticketId,
             string spotId) {

            //Both values point to the same parking spot.

    //Adding the mappings:
        spotByKey[vehicleNumber] = spotId;
        spotByKey[ticketId] = spotId;
    }

    string search(string vehicleNumber,
                  string ticketId) {
        //First, the system checks the vehicle number:
        if (!vehicleNumber.empty() &&
            spotByKey.count(vehicleNumber)) {
            return spotByKey[vehicleNumber];
        }
        //If it is not found, it checks the ticket ID:
        //Because an unordered_map is used, lookup takes average: O(1)

        if (!ticketId.empty() &&
            spotByKey.count(ticketId)) {
            return spotByKey[ticketId];
        }

        return "";
    }
};
```

### Design decision

> “I use one hash map because both keys return the same value: the spot ID.”

---

## Step 4: Parking floor


> “Each floor stores its spots in a 2D vector and maintains free counts by vehicle type.”

>Each floor manages its own:

Spots
Rows and columns
Parking logic
Removal logic
Free counts

Without ParkingFloor, the main Solution class would handle every low-level operation and become too large.

The responsibility is divided as:

ParkingSpot  → one spot
ParkingFloor → one floor
Solution     → complete parking lot

```cpp
class ParkingFloor {
    vector<vector<ParkingSpot*>> spots;

    //vehicle type → number of free spots...EG 2 → 5, 4 → 3
    unordered_map<int, int> freeCount;
};
```

### Initialize the floor

What I would say:

> “I create a `ParkingSpot` only for valid entries in the input matrix.”
>“This constructor builds one parking floor from the input layout. I first create a 2D matrix where every position is initially empty. Then I scan the layout, identify valid parking spots, extract the supported vehicle type, generate a unique spot ID using the helper, create the spot, and update the free count.”

```cpp
ParkingFloor(
    int floorNumber,
    vector<vector<string>>& layout,
    Helper* helper
) {
     // Number of rows and columns on this floor.
    int rows = layout.size();
    int cols = layout[0].size();
      /*
        Create a 2D matrix of parking spot pointers.

        Initially every position is nullptr because
        not every location in the layout represents
        a valid parking spot.
    */

    spots.resize(
        rows,
        vector<ParkingSpot*>(cols, nullptr)
    );

    // Traverse every position in the floor layout.
    for (int row = 0; row < rows; row++) {
        for (int col = 0; col < cols; col++) {
            // Example values can be "2-1", "4-1", etc.
            string value = layout[row][col];

            /*
                A value ending with '1' represents
                a valid parking spot.

                Example:
                "2-1" -> valid spot for a 2-wheeler
                "4-1" -> valid spot for a 4-wheeler
            */

            if (!value.empty() && value.back() == '1') {
                // Find the '-' separating vehicle type and validity.
                int dash = value.find('-');
                // Extract the vehicle type from the value.
                int vehicleType =
                    stoi(value.substr(0, dash));

                    /*
                    Ask Helper to generate the unique spot ID
                    using floor, row, and column.
                */

                string spotId =
                    helper->get_spot_id(
                        floorNumber,
                        row,
                        col
                    );
                 /*
                    Create a ParkingSpot object and store
                    its address at this row and column.
                */

                spots[row][col] =
                    new ParkingSpot(
                        spotId,
                        vehicleType
                    );

                freeCount[vehicleType]++;
            }
        }
    }
}
```

---

## Step 5: Park a vehicle on a floor

What I would say:

> “I first check the free count. Then I scan the floor and use the first free spot matching the vehicle type.”

```cpp
string park(int vehicleType) {
    if (freeCount[vehicleType] == 0) {
        return "";
    }

    for (auto& row : spots) {
        for (ParkingSpot* spot : row) {
            if (spot != nullptr &&
                !spot->isOccupied() &&
                spot->getVehicleType() == vehicleType) {

                spot->parkVehicle();
                freeCount[vehicleType]--;

                return spot->getSpotId();
            }
        }
    }

    return "";
}
```

---

## Step 6: Remove a vehicle from a floor

What I would say:

> “After converting the spot ID into floor, row, and column, I validate the location and free the spot.”

```cpp
bool removeVehicle(int row, int col) {
    if (row < 0 || row >= spots.size())
        return false;

    if (col < 0 || col >= spots[row].size())
        return false;

    ParkingSpot* spot = spots[row][col];

    if (spot == nullptr || !spot->isOccupied())
        return false;

    spot->removeVehicle();
    freeCount[spot->getVehicleType()]++;

    return true;
}
```

---

## Step 7: Main parking-lot controller

What I would say:

> “The main class creates all floors, tries them one by one while parking, and uses the search manager for vehicle and ticket lookup.”

```cpp
class Solution {
    Helper* helper;
    vector<ParkingFloor*> floors;
    SearchManager searchManager;
};
```

---

# 4. Complete code

```cpp
#include <bits/stdc++.h>
using namespace std;

/*
    Platform-specific helper.
*/
class Helper {
public:
    virtual string get_spot_id(
        int floor,
        int row,
        int col
    ) = 0;

    virtual vector<int> get_spot_location(
        const string& spot_id
    ) = 0;

    virtual ~Helper() = default;
};


/*
    Represents one parking spot.
*/
class ParkingSpot {
    string spotId;
    int vehicleType;
    bool occupied;

public:
    ParkingSpot(string spotId, int vehicleType) {
        this->spotId = spotId;
        this->vehicleType = vehicleType;
        occupied = false;
    }

    bool isOccupied() {
        return occupied;
    }

    void parkVehicle() {
        occupied = true;
    }

    void removeVehicle() {
        occupied = false;
    }

    string getSpotId() {
        return spotId;
    }

    int getVehicleType() {
        return vehicleType;
    }
};


/*
    Maps vehicle number and ticket ID to spot ID.
*/
class SearchManager {
    unordered_map<string, string> spotByKey;

public:
    void add(string vehicleNumber,
             string ticketId,
             string spotId) {
        spotByKey[vehicleNumber] = spotId;
        spotByKey[ticketId] = spotId;
    }

    string search(string vehicleNumber,
                  string ticketId) {
        if (!vehicleNumber.empty() &&
            spotByKey.count(vehicleNumber)) {
            return spotByKey[vehicleNumber];
        }

        if (!ticketId.empty() &&
            spotByKey.count(ticketId)) {
            return spotByKey[ticketId];
        }

        return "";
    }
};


/*
    Manages all parking spots on one floor.
*/
class ParkingFloor {
    vector<vector<ParkingSpot*>> spots;

    // vehicle type -> available spot count
    unordered_map<int, int> freeCount;

public:
    ParkingFloor(
        int floorNumber,
        vector<vector<string>>& layout,
        Helper* helper
    ) {
        int rows = layout.size();
        int cols = layout[0].size();

        spots.resize(
            rows,
            vector<ParkingSpot*>(cols, nullptr)
        );

        for (int row = 0; row < rows; row++) {
            for (int col = 0; col < cols; col++) {
                string value = layout[row][col];

                // Entry ending with 1 represents a valid spot.
                if (!value.empty() && value.back() == '1') {
                    int dash = value.find('-');

                    // Example: "2-1" means a type-2 spot.
                    int vehicleType =
                        stoi(value.substr(0, dash));

                    string spotId =
                        helper->get_spot_id(
                            floorNumber,
                            row,
                            col
                        );

                    spots[row][col] =
                        new ParkingSpot(
                            spotId,
                            vehicleType
                        );

                    freeCount[vehicleType]++;
                }
            }
        }
    }

    int getFreeCount(int vehicleType) {
        return freeCount[vehicleType];
    }

    /*
        Parks at the first matching free spot.
    */
    string park(int vehicleType) {
        if (freeCount[vehicleType] == 0) {
            return "";
        }

        for (auto& row : spots) {
            for (ParkingSpot* spot : row) {
                if (spot != nullptr &&
                    !spot->isOccupied() &&
                    spot->getVehicleType() == vehicleType) {

                    spot->parkVehicle();
                    freeCount[vehicleType]--;

                    return spot->getSpotId();
                }
            }
        }

        return "";
    }

    /*
        Frees the selected parking spot.
    */
    bool removeVehicle(int row, int col) {
        if (row < 0 || row >= spots.size())
            return false;

        if (col < 0 || col >= spots[row].size())
            return false;

        ParkingSpot* spot = spots[row][col];

        if (spot == nullptr || !spot->isOccupied())
            return false;

        spot->removeVehicle();
        freeCount[spot->getVehicleType()]++;

        return true;
    }

    ~ParkingFloor() {
        for (auto& row : spots) {
            for (ParkingSpot* spot : row) {
                delete spot;
            }
        }
    }
};


/*
    Main parking-lot controller.
*/
class Solution {
    Helper* helper;
    vector<ParkingFloor*> floors;
    SearchManager searchManager;

public:
    /*
        Builds all parking floors.
    */
    void init(
        Helper* helper,
        vector<vector<vector<string>>>& parking
    ) {
        this->helper = helper;

        for (int floor = 0;
             floor < parking.size();
             floor++) {

            floors.push_back(
                new ParkingFloor(
                    floor,
                    parking[floor],
                    helper
                )
            );
        }
    }

    /*
        Tries each floor until a spot is found.
    */
    string park(
        int vehicleType,
        string vehicleNumber,
        string ticketId
    ) {
        for (ParkingFloor* floor : floors) {
            string spotId = floor->park(vehicleType);

            if (!spotId.empty()) {
                searchManager.add(
                    vehicleNumber,
                    ticketId,
                    spotId
                );

                return spotId;
            }
        }

        return "";
    }

    /*
        Removes a vehicle using spot ID or search details.
    */
    int remove_vehicle(
        string spotId,
        string vehicleNumber,
        string ticketId
    ) {
        if (spotId.empty()) {
            spotId = searchManager.search(
                vehicleNumber,
                ticketId
            );
        }

        if (spotId.empty()) {
            return 404;
        }

        vector<int> location =
            helper->get_spot_location(spotId);

        if (location.size() < 3) {
            return 404;
        }

        int floor = location[0];
        int row = location[1];
        int col = location[2];

        if (floor < 0 || floor >= floors.size()) {
            return 404;
        }

        bool removed =
            floors[floor]->removeVehicle(row, col);

        return removed ? 201 : 404;
    }

    /*
        Returns free spots for a vehicle type on one floor.
    */
    int get_free_spots_count(
        int floor,
        int vehicleType
    ) {
        if (floor < 0 || floor >= floors.size()) {
            return 0;
        }

        return floors[floor]
            ->getFreeCount(vehicleType);
    }

    /*
        Searches using vehicle number or ticket ID.
    */
    string search_vehicle(
        string vehicleNumber,
        string ticketId
    ) {
        return searchManager.search(
            vehicleNumber,
            ticketId
        );
    }

    ~Solution() {
        for (ParkingFloor* floor : floors) {
            delete floor;
        }
    }
};
```

---

# 5. Complexity

Let:

- `F` be the number of floors
- `R × C` be the number of positions on one floor

What I would say:

> “Parking may scan every floor and every spot, so the worst-case time is O(F × R × C).
>
> Searching by vehicle number or ticket ID takes average O(1).
>
> Removing a vehicle takes average O(1) after the helper converts the spot ID into its location.
>
> Getting the free count takes average O(1).
>
> Storage is O(F × R × C) for parking spots and O(V) for indexed vehicles.”

---

# 6. Important design decisions

## Why use the abstract `Helper` class?

> “Spot-ID formatting depends on the platform. The parking classes should not know whether the ID looks like `F1-R2-C3` or uses another format.
>
> `Helper` separates that platform-specific logic from parking logic.”

## Why use a separate `ParkingFloor` class?

> “Parking and removal happen at the floor level. Keeping that logic inside `ParkingFloor` prevents the main class from directly handling rows and columns.”

## Why use `SearchManager`?

> “Vehicle and ticket lookup is a separate responsibility. A hash map provides average O(1) search.”

## Why use a free-count map?

> “Before scanning a floor, I can quickly check whether that vehicle type has any available spots.”

## Why still scan spots?

> “The requirement only asks for the first available spot. Scanning keeps the implementation simple enough for 30 minutes.
>
> As a follow-up, I could maintain queues of available spots by vehicle type.”

---

# 7. Follow-up improvements

## Faster parking

What I would say:

> “I would store available spot IDs in a queue for each vehicle type. Then parking becomes O(1) instead of scanning the floor.”

## Remove stale search entries

What I would say:

> “After removing a vehicle, I would also delete its vehicle-number and ticket-ID mappings from `SearchManager`.”

## Support more vehicle types

What I would say:

> “No major class changes are required. The vehicle type is already stored as an integer and free counts are maintained in a map.”

## Prevent duplicate tickets or vehicle numbers

What I would say:

> “Before parking, I would check whether the vehicle number or ticket ID already exists in `SearchManager`.”

---

# 8. Final interview summary

What I would say:

> “The system is divided by responsibility.
>
> `ParkingSpot` represents one spot, `ParkingFloor` handles floor-level parking, `SearchManager` handles lookup, and `Solution` coordinates the workflow.
>
> `Helper` is abstract because spot-ID generation and parsing are platform-specific.
>
> Free counts and search operations are O(1), while parking scans spots in the simple 30-minute version.”
