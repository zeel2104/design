#include <bits/stdc++.h>
using namespace std;

/*
    Helper is assumed to provide these two utility methods.
    Their internal implementation depends on the platform.
*/
class Helper {
public:
    virtual string get_spot_id(int floor, int row, int col) = 0;
    virtual vector<int> get_spot_location(const string& spot_id) = 0;

    virtual ~Helper() = default;
};


/*
    Represents one parking spot.
*/
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


/*
    Stores mappings from vehicle number and ticket ID
    to the assigned parking spot.
*/
class SearchManager {
private:
    unordered_map<string, string> cache;

public:
    string search_vehicle(string vehicle_number, string ticket_id) {
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

    void index(string spot_id, string vehicle_number, string ticket_id) {
        cache[vehicle_number] = spot_id;
        cache[ticket_id] = spot_id;
    }
};


/*
    Represents one floor of the parking lot.
*/
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
            for (int col = 0; col < parking_floor[row].size(); col++) {
                string value = parking_floor[row][col];

                // A value ending in "1" represents a valid parking spot.
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

    int get_free_spots_count(int vehicle_type) {
        if (free_spots_count.find(vehicle_type) ==
            free_spots_count.end()) {
            return 0;
        }

        return free_spots_count[vehicle_type];
    }

    int remove_vehicle(int row, int col) {
        if (row < 0 || row >= parking_spots.size()) {
            return 404;
        }

        if (col < 0 || col >= parking_spots[row].size()) {
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

    string park(
        int vehicle_type,
        string vehicle_number,
        string ticket_id
    ) {
        if (get_free_spots_count(vehicle_type) == 0) {
            return "";
        }

        for (int row = 0; row < parking_spots.size(); row++) {
            for (int col = 0;
                 col < parking_spots[row].size();
                 col++) {

                ParkingSpot* spot = parking_spots[row][col];

                if (spot != nullptr &&
                    !spot->is_parked() &&
                    spot->get_vehicle_type() == vehicle_type) {

                    free_spots_count[vehicle_type]--;
                    spot->park_vehicle();

                    return spot->get_spot_id();
                }
            }
        }

        return "";
    }

    ~ParkingFloor() {
        for (int row = 0; row < parking_spots.size(); row++) {
            for (int col = 0;
                 col < parking_spots[row].size();
                 col++) {
                delete parking_spots[row][col];
            }
        }
    }
};


/*
    Main parking lot class.
*/
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
    ) {
        this->helper = helper;
        vehicle_types = {2, 4};

        for (int floor = 0; floor < parking.size(); floor++) {
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

    string search_vehicle(
        string vehicle_number,
        string ticket_id
    ) {
        return search_manager.search_vehicle(
            vehicle_number,
            ticket_id
        );
    }

    ~Solution() {
        for (ParkingFloor* floor : floors) {
            delete floor;
        }
    }
};