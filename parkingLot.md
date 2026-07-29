## 1. Start with the required operations

“I’ll first define the scope. The parking lot has multiple floors, and each floor contains spots arranged by row and column.

For this implementation, I need four operations:

* park a two-wheeler or four-wheeler,
* remove a vehicle,
* get the free-spot count for a vehicle type on a floor,
* and search for a vehicle using its vehicle number or ticket ID.

I’ll leave payments, reservations, entry gates, and concurrency outside the current scope.”

```cpp
class Solution {
public:
    void init(
        Helper* helper,
        vector<vector<vector<string>>>& parking
    );

    string park(
        int vehicle_type,
        string vehicle_number,
        string ticket_id
    );

    int remove_vehicle(
        string spot_id,
        string vehicle_number,
        string ticket_id
    );

    int get_free_spots_count(
        int floor,
        int vehicle_type
    );

    string search_vehicle(
        string vehicle_number,
        string ticket_id
    );
};
```

---

## 2. Identify the main classes

“I’ll keep the design small and use three main classes.

`ParkingSpot` represents one physical spot.

`ParkingFloor` owns all the spots on one floor and handles parking and removal within that floor.

`SearchManager` maintains the mappings required to search by vehicle number or ticket ID.

Finally, `Solution` coordinates all the floors.”

```cpp
class ParkingSpot;
class ParkingFloor;
class SearchManager;
class Solution;
```

---

## 3. Define the helper dependency

“The platform already controls the format of a spot ID, so I won’t recreate that logic.

I’ll use the provided `Helper` abstraction to convert a floor-row-column location into a spot ID and convert a spot ID back into its location.”

```cpp
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
```

---

## 4. Build the smallest entity: `ParkingSpot`

“I’ll start with the smallest entity.

A parking spot needs its unique ID, the vehicle type it supports, and whether it is currently occupied.

The spot itself is responsible only for changing and exposing its own state.”

```cpp
class ParkingSpot {
private:
    string spot_id;
    int vehicle_type;
    bool is_spot_parked;

public:
    ParkingSpot(string spot_id, int vehicle_type) {
        this->spot_id = spot_id;
        this->vehicle_type = vehicle_type;
        this->is_spot_parked = false;
    }

    bool is_parked() {
        return is_spot_parked;
    }

    void park_vehicle() {
        is_spot_parked = true;
    }

    void remove_vehicle() {
        is_spot_parked = false;
    }

    string get_spot_id() {
        return spot_id;
    }

    int get_vehicle_type() {
        return vehicle_type;
    }
};
```

---

## 5. Design `ParkingFloor`

“Next, I’ll model one floor.

Because the input and spot IDs are based on rows and columns, I’ll preserve the same structure using a two-dimensional vector.

Some cells may not represent valid parking spots, so I’ll store `nullptr` for those cells.”

```cpp
class ParkingFloor {
private:
    vector<vector<ParkingSpot*>> parking_spots;
    unordered_map<int, int> free_spots_count;

public:
    ParkingFloor(
        int floor,
        vector<vector<string>>& parking_floor,
        vector<int>& vehicle_types,
        Helper* helper
    );
};
```

---

## 6. Track free counts separately

“I also need to display the number of available spots by vehicle type.

Instead of scanning the complete floor whenever the count is requested, I’ll maintain an `unordered_map` from vehicle type to free-spot count.

This makes the count operation average constant time.”

```cpp
private:
    // Vehicle type -> currently available spots
    unordered_map<int, int> free_spots_count;
```

---

## 7. Initialize one floor

“In the `ParkingFloor` constructor, I’ll translate the input representation into `ParkingSpot` objects.

A value ending in `1` represents a valid parking spot. I’ll extract the vehicle type from the part before the dash.

For every valid spot, I create a `ParkingSpot` and increment the corresponding free count.”

```cpp
// This constructor belongs to ParkingFloor.
ParkingFloor(
    int floor,
    vector<vector<string>>& parking_floor,
    vector<int>& vehicle_types,
    Helper* helper
) {
    int rows = parking_floor.size();
    int cols = parking_floor[0].size();

    parking_spots.resize(
        rows,
        vector<ParkingSpot*>(cols, nullptr)
    );

    for (int vehicle_type : vehicle_types) {
        free_spots_count[vehicle_type] = 0;
    }

    for (int row = 0; row < rows; row++) {
        for (int col = 0;
             col < parking_floor[row].size();
             col++) {

            string value = parking_floor[row][col];

            if (!value.empty() && value.back() == '1') {
                int dash_position = value.find('-');

                int vehicle_type =
                    stoi(value.substr(0, dash_position));

                string spot_id =
                    helper->get_spot_id(floor, row, col);

                parking_spots[row][col] =
                    new ParkingSpot(spot_id, vehicle_type);

                free_spots_count[vehicle_type]++;
            }
        }
    }
}
```

---

## 8. Return the free-spot count

“This method belongs to `ParkingFloor`.

Since the count is already maintained in the map, I simply return it. For an unsupported vehicle type, I return zero.”

```cpp
// ParkingFloor method
int get_free_spots_count(int vehicle_type) {
    if (free_spots_count.find(vehicle_type) ==
        free_spots_count.end()) {
        return 0;
    }

    return free_spots_count[vehicle_type];
}
```

---

## 9. Park within one floor

“Now I’ll implement parking within a single floor.

I first check the maintained free count. If it is zero, I can skip scanning this floor.

Otherwise, I scan in row-major order and take the first empty spot supporting the requested vehicle type.

Once found, I mark it occupied, decrement the count, and return its spot ID.”

```cpp
// ParkingFloor method
string park(
    int vehicle_type,
    string vehicle_number,
    string ticket_id
) {
    if (get_free_spots_count(vehicle_type) == 0) {
        return "";
    }

    for (int row = 0;
         row < parking_spots.size();
         row++) {

        for (int col = 0;
             col < parking_spots[row].size();
             col++) {

            ParkingSpot* spot = parking_spots[row][col];

            if (spot != nullptr &&
                !spot->is_parked() &&
                spot->get_vehicle_type() == vehicle_type) {

                spot->park_vehicle();
                free_spots_count[vehicle_type]--;

                return spot->get_spot_id();
            }
        }
    }

    return "";
}
```

“The vehicle number and ticket ID are not used inside `ParkingFloor` because this class only manages physical spots. Their search mappings will be maintained at the parking-lot level.”

---

## 10. Remove from one floor

“For removal, the spot ID will already have been converted into a row and column.

This method belongs to `ParkingFloor`. I validate the location, ensure that the cell contains an occupied spot, mark it empty, and increment the appropriate free count.”

```cpp
// ParkingFloor method
int remove_vehicle(int row, int col) {
    if (row < 0 || row >= parking_spots.size()) {
        return 404;
    }

    if (col < 0 ||
        col >= parking_spots[row].size()) {
        return 404;
    }

    ParkingSpot* spot = parking_spots[row][col];

    if (spot == nullptr || !spot->is_parked()) {
        return 404;
    }

    int vehicle_type = spot->get_vehicle_type();

    spot->remove_vehicle();
    free_spots_count[vehicle_type]++;

    return 201;
}
```

---

## 11. Add vehicle search

“Next, I need to support searching using either the vehicle number or ticket ID.

Both values map to the same result, which is a spot ID, so I can use one hash map.

Whenever a vehicle is parked, I add two entries: vehicle number to spot ID and ticket ID to spot ID.”

```cpp
class SearchManager {
private:
    unordered_map<string, string> cache;

public:
    string search_vehicle(
        string vehicle_number,
        string ticket_id
    ) {
        if (!vehicle_number.empty()) {
            if (cache.find(vehicle_number) != cache.end()) {
                return cache[vehicle_number];
            }

            return "";
        }

        if (!ticket_id.empty()) {
            if (cache.find(ticket_id) != cache.end()) {
                return cache[ticket_id];
            }

            return "";
        }

        return "";
    }

    void index(
        string spot_id,
        string vehicle_number,
        string ticket_id
    ) {
        cache[vehicle_number] = spot_id;
        cache[ticket_id] = spot_id;
    }
};
```

“I separated this into `SearchManager` because searching and indexing are different responsibilities from managing the physical layout of a floor.”

---

## 12. Connect everything through `Solution`

“Now I’ll create the main coordinating class.

It stores the helper, supported vehicle types, all parking floors, and the search manager.”

```cpp
class Solution {
private:
    Helper* helper;
    vector<int> vehicle_types;
    vector<ParkingFloor*> floors;
    SearchManager search_manager;

public:
    void init(
        Helper* helper,
        vector<vector<vector<string>>>& parking
    );

    string park(
        int vehicle_type,
        string vehicle_number,
        string ticket_id
    );

    int remove_vehicle(
        string spot_id,
        string vehicle_number,
        string ticket_id
    );

    int get_free_spots_count(
        int floor,
        int vehicle_type
    );

    string search_vehicle(
        string vehicle_number,
        string ticket_id
    );
};
```

---

## 13. Initialize the parking lot

“In `Solution::init`, I store the helper, define the supported vehicle types, and create one `ParkingFloor` object for every floor in the input.”

```cpp
// Solution method
void init(
    Helper* helper,
    vector<vector<vector<string>>>& parking
) {
    this->helper = helper;
    vehicle_types = {2, 4};

    for (int floor = 0;
         floor < parking.size();
         floor++) {

        floors.push_back(
            new ParkingFloor(
                floor,
                parking[floor],
                vehicle_types,
                helper
            )
        );
    }
}
```

---

## 14. Park across multiple floors

“The floor class knows how to find a spot within itself, while `Solution` decides which floor to try.

I iterate through floors in order and ask each floor to park the vehicle.

When a floor returns a spot ID, I index the vehicle number and ticket ID before returning the result.”

```cpp
// Solution method
string park(
    int vehicle_type,
    string vehicle_number,
    string ticket_id
) {
    for (ParkingFloor* floor : floors) {
        string result_spot_id =
            floor->park(
                vehicle_type,
                vehicle_number,
                ticket_id
            );

        if (result_spot_id != "") {
            search_manager.index(
                result_spot_id,
                vehicle_number,
                ticket_id
            );

            return result_spot_id;
        }
    }

    return "";
}
```

“This gives us a simple first-available policy: lowest floor first, followed by row-major order within that floor.”

---

## 15. Search through `Solution`

“The public search method simply delegates to `SearchManager`.

The search takes average constant time because it uses an `unordered_map`.”

```cpp
// Solution method
string search_vehicle(
    string vehicle_number,
    string ticket_id
) {
    return search_manager.search_vehicle(
        vehicle_number,
        ticket_id
    );
}
```

---

## 16. Remove using any available identifier

“For removal, the caller may directly provide the spot ID.

If the spot ID is missing, I search using the vehicle number or ticket ID.

Once I have a spot ID, I use the helper to extract its floor, row, and column. Then I delegate the actual state change to the correct `ParkingFloor`.”

```cpp
// Solution method
int remove_vehicle(
    string spot_id,
    string vehicle_number,
    string ticket_id
) {
    string search_spot_id = spot_id;

    if (search_spot_id == "") {
        search_spot_id =
            search_vehicle(vehicle_number, ticket_id);
    }

    if (search_spot_id == "") {
        return 404;
    }

    vector<int> location =
        helper->get_spot_location(search_spot_id);

    if (location.size() < 3 || location[0] < 0) {
        return 404;
    }

    int floor = location[0];
    int row = location[1];
    int col = location[2];

    if (floor < 0 || floor >= floors.size()) {
        return 404;
    }

    return floors[floor]->remove_vehicle(row, col);
}
```

---

## 17. Expose the count through `Solution`

“For the free-count operation, `Solution` validates the floor number and delegates the request to that floor.”

```cpp
// Solution method
int get_free_spots_count(
    int floor,
    int vehicle_type
) {
    if (floor < 0 || floor >= floors.size()) {
        return 0;
    }

    return floors[floor]
        ->get_free_spots_count(vehicle_type);
}
```

---

## 18. Mention memory cleanup

“Because this version uses raw pointers, each owning class should delete the objects it creates.

`ParkingFloor` owns its spots, and `Solution` owns its floors.”

```cpp
// ParkingFloor destructor
~ParkingFloor() {
    for (int row = 0;
         row < parking_spots.size();
         row++) {

        for (int col = 0;
             col < parking_spots[row].size();
             col++) {

            delete parking_spots[row][col];
        }
    }
}
```

```cpp
// Solution destructor
~Solution() {
    for (ParkingFloor* floor : floors) {
        delete floor;
    }
}
```

---

## 19. Explain the complete parking flow

“The complete parking flow is:

`Solution` checks floors in order.

Each `ParkingFloor` checks whether that vehicle type has any free spots.

It finds the first matching empty `ParkingSpot`, marks it occupied, and updates the count.

`Solution` then indexes the vehicle number and ticket ID and returns the spot ID.”

```cpp
Solution::park()
    -> ParkingFloor::park()
        -> ParkingSpot::park_vehicle()
    -> SearchManager::index()
```

---

## 20. Explain the complete removal flow

“The removal flow is:

Use the provided spot ID, or search for it using the vehicle number or ticket ID.

Convert that spot ID into floor, row, and column.

Ask the corresponding floor to remove the vehicle.

The floor marks the spot empty and increments its free count.”

```cpp
Solution::remove_vehicle()
    -> SearchManager::search_vehicle()   // when needed
    -> Helper::get_spot_location()
    -> ParkingFloor::remove_vehicle()
    -> ParkingSpot::remove_vehicle()
```

---

## 21. Complexity

“Searching by vehicle number or ticket ID is average `O(1)` because of the hash map.

Getting a free count is also average `O(1)`.

Removing a vehicle is `O(1)` after the spot ID is converted into a location.

Parking can scan all spots in the worst case, so it is `O(F × R × C)`, where `F` is the number of floors and `R × C` is the number of cells per floor.

The overall space complexity is `O(F × R × C + V)`, where `V` is the number of indexed vehicles or tickets.”

---

## 22. Important trade-off

“The free count tells me immediately whether a floor has capacity, but it does not directly tell me where the next free spot is.

Therefore, parking may still require a linear scan.

If parking performance becomes important, I could maintain a queue or ordered set of available spots for each vehicle type. For the current requirements, the linear scan keeps the implementation simple.”

---

## 23. Historical search behavior

“One detail in this implementation is that search entries are not removed when a vehicle leaves.

That means search behaves like historical lookup and can still return the last spot associated with a vehicle number or ticket ID.

If the requirement were to search only currently parked vehicles, I would remove those mappings during `remove_vehicle`.”

---

## 24. Concurrency follow-up

“This implementation assumes single-threaded execution.

In a concurrent version, two requests could select the same free spot before either one updates it.

I would protect spot allocation and removal with locking, or perform the spot-state update atomically. I would also make indexing part of the same logical transaction so the physical spot state and search mappings remain consistent.”

---

## 25. Final summary

“To summarize, I separated the system by responsibility.

`ParkingSpot` owns the state of one spot.

`ParkingFloor` owns the physical layout and free counts for one floor.

`SearchManager` supports lookup through hash maps.

`Solution` coordinates operations across floors.

The current design is simple enough to implement in an interview, while leaving clear extension points for better allocation policies, payments, concurrency, and additional vehicle types.”
