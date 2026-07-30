# Movie Booking System — 45–60 Minute Interview Version

## 1. Basic intuition

> “The system should allow cinemas and shows to be added, customers to view available seats, book selected seats, and cancel an existing booking.
>
> I’ll keep `Cinema`, `Screen`, `Seat`, `Show`, and `Booking` as the main entities.
>
> A physical seat belongs to a screen, but its booking status belongs to a particular show. For example, seat A1 can be booked for the 6 PM show but available for the 9 PM show.
>
> `MovieBookingSystem` will act as the main service and coordinate show creation, seat availability, booking, and cancellation.
>
> I’ll keep payments, temporary seat locks, notifications, pricing, and automatic seat selection outside the initial scope.”

---

## 2. Clarifying questions

What I would ask:

> “Should customers select exact seat IDs, or should the system automatically allocate seats?”
>
> “Do we need to handle payments and refunds?”
>
> “Should cancellation make the seats available again?”
>
> “Can the same screen host multiple shows at different times?”
>
> “Do we need temporary seat locking while payment is in progress?”
>
> “Should the solution be in-memory for this interview?”

### Assumptions

> “For the initial implementation:
>
> * Customers will select exact seat IDs.
> * Payment and refunds are outside scope.
> * Cancellation releases the seats.
> * A screen can have multiple shows.
> * Temporary seat locking is outside scope.
> * The solution will use in-memory storage.”

---

## 3. Scoped functional requirements

The initial version will support:

```text
1. Add a cinema.
2. Add screens and seats to a cinema.
3. Add a movie show to a screen.
4. List shows for a movie.
5. View available seats for a show.
6. Book selected seats.
7. Cancel an existing booking.
```

### Out of scope

```text
1. User authentication
2. Payment and refund processing
3. Temporary seat locking
4. Dynamic pricing
5. Notifications
6. Coupons and discounts
7. Automatic adjacent-seat selection
8. Distributed database handling
```

---

## 4. Main classes

> “I need:
>
> * `BookingStatus` enum
> * `Seat`
> * `Screen`
> * `Cinema`
> * `Show`
> * `Booking`
> * `MovieBookingSystem` as the main service”

### Why no abstract class initially?

> “The scoped requirements do not currently have multiple interchangeable behaviors.
>
> Adding an abstract class only for the sake of using a design pattern would increase complexity.
>
> If seat allocation, pricing, or payment behavior becomes configurable later, I can introduce Strategy interfaces for those responsibilities.”

---

# 5. Build the solution step by step

## Step 1: Booking status and seat

What I would say:

> “A seat is a physical entity inside a screen. It only stores its identity and location.
>
> I will not store `isBooked` inside `Seat`, because booking status changes for every show.”

```cpp
enum BookingStatus {
    CONFIRMED,
    CANCELLED
};

class Seat {
    string seatId;
    int row;
    int column;

public:
    Seat(string seatId, int row, int column) {
        this->seatId = seatId;
        this->row = row;
        this->column = column;
    }

    string getSeatId() const {
        return seatId;
    }

    int getRow() const {
        return row;
    }

    int getColumn() const {
        return column;
    }
};
```

### Important design decision

> “`Seat` represents the physical seat.
>
> Whether that seat is booked is stored inside `Show`, because availability is different for every show.”

---

## Step 2: Screen

What I would say:

> “A screen belongs to a cinema and contains a fixed collection of physical seats.”

```cpp
class Screen {
    string screenId;
    vector<Seat> seats;

public:
    Screen(string screenId) {
        this->screenId = screenId;
    }

    string getScreenId() const {
        return screenId;
    }

    void addSeat(const Seat& seat) {
        seats.push_back(seat);
    }

    const vector<Seat>& getSeats() const {
        return seats;
    }

    bool hasSeat(const string& seatId) const {
        for (const Seat& seat : seats) {
            if (seat.getSeatId() == seatId)
                return true;
        }

        return false;
    }
};
```

### Design decision

> “For simplicity, I store seats in a vector.
>
> Checking whether a seat exists takes O(S), where S is the number of seats in the screen.
>
> If this lookup becomes frequent, I can maintain an additional `unordered_map` from seat ID to seat.”

---

## Step 3: Cinema

What I would say:

> “A cinema has an ID, name, city, and one or more screens.”

```cpp
class Cinema {
    string cinemaId;
    string name;
    string city;

    // screen ID -> screen
    unordered_map<string, Screen> screens;

public:
    Cinema(
        string cinemaId,
        string name,
        string city
    ) {
        this->cinemaId = cinemaId;
        this->name = name;
        this->city = city;
    }

    string getCinemaId() const {
        return cinemaId;
    }

    string getName() const {
        return name;
    }

    string getCity() const {
        return city;
    }

    bool addScreen(const Screen& screen) {
        string screenId = screen.getScreenId();

        if (screens.count(screenId))
            return false;

        screens.emplace(screenId, screen);
        return true;
    }

    bool hasScreen(const string& screenId) const {
        return screens.count(screenId);
    }

    bool screenHasSeat(
        const string& screenId,
        const string& seatId
    ) const {
        auto it = screens.find(screenId);

        if (it == screens.end())
            return false;

        return it->second.hasSeat(seatId);
    }

    const vector<Seat>& getSeats(
        const string& screenId
    ) const {
        return screens.at(screenId).getSeats();
    }
};
```

### Design decision

> “I use a map from screen ID to `Screen` so that a particular screen can be found efficiently.”

---

## Step 4: Show

What I would say:

> “A show represents one movie playing on one screen at a particular time.
>
> The show stores the seat IDs that are currently booked.”

```cpp
class Show {
    string showId;
    string movieName;
    string cinemaId;
    string screenId;
    string startTime;

    // Seats booked specifically for this show.
    unordered_set<string> bookedSeatIds;

public:
    Show(
        string showId,
        string movieName,
        string cinemaId,
        string screenId,
        string startTime
    ) {
        this->showId = showId;
        this->movieName = movieName;
        this->cinemaId = cinemaId;
        this->screenId = screenId;
        this->startTime = startTime;
    }

    string getShowId() const {
        return showId;
    }

    string getMovieName() const {
        return movieName;
    }

    string getCinemaId() const {
        return cinemaId;
    }

    string getScreenId() const {
        return screenId;
    }

    string getStartTime() const {
        return startTime;
    }

    bool isSeatAvailable(
        const string& seatId
    ) const {
        return !bookedSeatIds.count(seatId);
    }

    void bookSeat(const string& seatId) {
        bookedSeatIds.insert(seatId);
    }

    void releaseSeat(const string& seatId) {
        bookedSeatIds.erase(seatId);
    }
};
```

### Design decision

> “I store only booked seat IDs instead of storing a boolean for every seat.
>
> A seat is available when its ID is not present in the booked-seat set.”

---

## Step 5: Booking

What I would say:

> “A booking records the show, selected seat IDs, and current status.
>
> I retain cancelled bookings instead of deleting them so that booking history is preserved.”

```cpp
class Booking {
    string bookingId;
    string showId;
    vector<string> seatIds;
    BookingStatus status;

public:
    Booking(
        string bookingId,
        string showId,
        vector<string> seatIds
    ) {
        this->bookingId = bookingId;
        this->showId = showId;
        this->seatIds = seatIds;
        this->status = CONFIRMED;
    }

    string getBookingId() const {
        return bookingId;
    }

    string getShowId() const {
        return showId;
    }

    const vector<string>& getSeatIds() const {
        return seatIds;
    }

    BookingStatus getStatus() const {
        return status;
    }

    void cancel() {
        status = CANCELLED;
    }
};
```

---

## Step 6: Movie booking system

What I would say:

> “`MovieBookingSystem` acts as the main service.
>
> It stores cinemas, shows, and bookings.
>
> It validates cross-entity operations such as whether a show refers to a valid screen and whether requested seats belong to that screen.”

```cpp
class MovieBookingSystem {
    // cinema ID -> cinema
    unordered_map<string, Cinema> cinemas;

    // show ID -> show
    unordered_map<string, Show> shows;

    // booking ID -> booking
    unordered_map<string, Booking> bookings;
};
```

---

# 6. Main operations

## Add a cinema

What I would say:

> “Cinema IDs must be unique. The cinema already contains its screens and seats.”

```cpp
bool addCinema(const Cinema& cinema) {
    string cinemaId = cinema.getCinemaId();

    if (cinemas.count(cinemaId))
        return false;

    cinemas.emplace(cinemaId, cinema);
    return true;
}
```

---

## Add a show

What I would say:

> “Before adding a show, I validate that the cinema exists and that the referenced screen belongs to that cinema.”

```cpp
bool addShow(const Show& show) {
    if (shows.count(show.getShowId()))
        return false;

    auto cinemaIt =
        cinemas.find(show.getCinemaId());

    if (cinemaIt == cinemas.end())
        return false;

    if (!cinemaIt->second.hasScreen(
            show.getScreenId()
        ))
        return false;

    shows.emplace(show.getShowId(), show);
    return true;
}
```

---

## List shows for a movie

What I would say:

> “For the scoped version, I scan all shows and return shows matching the movie name.
>
> At scale, I would maintain a movie-to-show index.”

```cpp
vector<string> listShows(
    const string& movieName
) const {
    vector<string> result;

    for (const auto& entry : shows) {
        const Show& show = entry.second;

        if (show.getMovieName() == movieName)
            result.push_back(show.getShowId());
    }

    return result;
}
```

---

## View available seats

What I would say:

> “I retrieve the physical seats from the show’s screen and filter out the seat IDs already booked for that show.”

```cpp
vector<string> getAvailableSeats(
    const string& showId
) const {
    vector<string> availableSeats;

    auto showIt = shows.find(showId);

    if (showIt == shows.end())
        return availableSeats;

    const Show& show = showIt->second;

    auto cinemaIt =
        cinemas.find(show.getCinemaId());

    if (cinemaIt == cinemas.end())
        return availableSeats;

    const vector<Seat>& seats =
        cinemaIt->second.getSeats(
            show.getScreenId()
        );

    for (const Seat& seat : seats) {
        if (show.isSeatAvailable(
                seat.getSeatId()
            )) {
            availableSeats.push_back(
                seat.getSeatId()
            );
        }
    }

    return availableSeats;
}
```

---

## Book selected seats

What I would say:

> “First, I validate the booking ID, show, requested seats, and availability.
>
> I validate all seats before modifying the show. This avoids a partial booking where some seats are booked before another requested seat fails validation.
>
> Only after all validations pass do I mark the seats as booked and create the booking.”

```cpp
bool bookSeats(
    const string& bookingId,
    const string& showId,
    const vector<string>& seatIds
) {
    if (bookings.count(bookingId))
        return false;

    if (seatIds.empty())
        return false;

    auto showIt = shows.find(showId);

    if (showIt == shows.end())
        return false;

    Show& show = showIt->second;

    auto cinemaIt =
        cinemas.find(show.getCinemaId());

    if (cinemaIt == cinemas.end())
        return false;

    const Cinema& cinema = cinemaIt->second;

    // Prevent duplicate seat IDs in one request.
    unordered_set<string> uniqueSeatIds;

    for (const string& seatId : seatIds) {
        if (uniqueSeatIds.count(seatId))
            return false;

        uniqueSeatIds.insert(seatId);

        // Validate that the physical seat exists.
        if (!cinema.screenHasSeat(
                show.getScreenId(),
                seatId
            ))
            return false;

        // Validate that the seat is available.
        if (!show.isSeatAvailable(seatId))
            return false;
    }

    // All validation passed, so update the show.
    for (const string& seatId : seatIds)
        show.bookSeat(seatId);

    Booking booking(
        bookingId,
        showId,
        seatIds
    );

    bookings.emplace(bookingId, booking);
    return true;
}
```

### Important booking principle

> “Validate everything first and modify everything second.”

---

## Cancel a booking

What I would say:

> “For cancellation, I find the booking and verify that it is currently confirmed.
>
> I release all corresponding seats from the show and mark the booking cancelled.”

```cpp
bool cancelBooking(
    const string& bookingId
) {
    auto bookingIt =
        bookings.find(bookingId);

    if (bookingIt == bookings.end())
        return false;

    Booking& booking = bookingIt->second;

    if (booking.getStatus() == CANCELLED)
        return false;

    auto showIt =
        shows.find(booking.getShowId());

    if (showIt == shows.end())
        return false;

    Show& show = showIt->second;

    for (const string& seatId :
         booking.getSeatIds()) {
        show.releaseSeat(seatId);
    }

    booking.cancel();
    return true;
}
```

---

# 7. Complete code

```cpp
#include <bits/stdc++.h>
using namespace std;


/*
    Current state of a booking.
*/
enum BookingStatus {
    CONFIRMED,
    CANCELLED
};


/*
    Represents one physical seat inside a screen.

    Booking status is intentionally not stored here
    because availability is different for every show.
*/
class Seat {
private:
    string seatId;
    int row;
    int column;

public:
    Seat(
        string seatId,
        int row,
        int column
    ) {
        this->seatId = seatId;
        this->row = row;
        this->column = column;
    }

    string getSeatId() const {
        return seatId;
    }

    int getRow() const {
        return row;
    }

    int getColumn() const {
        return column;
    }
};


/*
    Represents one screen inside a cinema.
*/
class Screen {
private:
    string screenId;
    vector<Seat> seats;

public:
    Screen(string screenId) {
        this->screenId = screenId;
    }

    string getScreenId() const {
        return screenId;
    }

    /*
        Adds a physical seat to the screen.
    */
    void addSeat(const Seat& seat) {
        seats.push_back(seat);
    }

    const vector<Seat>& getSeats() const {
        return seats;
    }

    /*
        Checks whether the seat physically
        belongs to this screen.
    */
    bool hasSeat(
        const string& seatId
    ) const {
        for (const Seat& seat : seats) {
            if (seat.getSeatId() == seatId)
                return true;
        }

        return false;
    }
};


/*
    Represents one cinema containing
    one or more screens.
*/
class Cinema {
private:
    string cinemaId;
    string name;
    string city;

    // screen ID -> screen
    unordered_map<string, Screen> screens;

public:
    Cinema(
        string cinemaId,
        string name,
        string city
    ) {
        this->cinemaId = cinemaId;
        this->name = name;
        this->city = city;
    }

    string getCinemaId() const {
        return cinemaId;
    }

    string getName() const {
        return name;
    }

    string getCity() const {
        return city;
    }

    /*
        Adds a screen if its ID is unique
        inside this cinema.
    */
    bool addScreen(const Screen& screen) {
        string screenId = screen.getScreenId();

        if (screens.count(screenId))
            return false;

        screens.emplace(screenId, screen);
        return true;
    }

    bool hasScreen(
        const string& screenId
    ) const {
        return screens.count(screenId);
    }

    /*
        Checks whether a particular seat exists
        inside the given screen.
    */
    bool screenHasSeat(
        const string& screenId,
        const string& seatId
    ) const {
        auto it = screens.find(screenId);

        if (it == screens.end())
            return false;

        return it->second.hasSeat(seatId);
    }

    /*
        Returns all physical seats of a screen.

        The caller should first validate that
        the screen exists.
    */
    const vector<Seat>& getSeats(
        const string& screenId
    ) const {
        return screens.at(screenId).getSeats();
    }
};


/*
    Represents one movie show on a screen
    at a particular time.
*/
class Show {
private:
    string showId;
    string movieName;
    string cinemaId;
    string screenId;
    string startTime;

    // Seat IDs booked specifically for this show.
    unordered_set<string> bookedSeatIds;

public:
    Show(
        string showId,
        string movieName,
        string cinemaId,
        string screenId,
        string startTime
    ) {
        this->showId = showId;
        this->movieName = movieName;
        this->cinemaId = cinemaId;
        this->screenId = screenId;
        this->startTime = startTime;
    }

    string getShowId() const {
        return showId;
    }

    string getMovieName() const {
        return movieName;
    }

    string getCinemaId() const {
        return cinemaId;
    }

    string getScreenId() const {
        return screenId;
    }

    string getStartTime() const {
        return startTime;
    }

    /*
        A seat is available when it is not
        present in the booked-seat set.
    */
    bool isSeatAvailable(
        const string& seatId
    ) const {
        return !bookedSeatIds.count(seatId);
    }

    void bookSeat(
        const string& seatId
    ) {
        bookedSeatIds.insert(seatId);
    }

    void releaseSeat(
        const string& seatId
    ) {
        bookedSeatIds.erase(seatId);
    }
};


/*
    Represents one confirmed or cancelled booking.
*/
class Booking {
private:
    string bookingId;
    string showId;
    vector<string> seatIds;
    BookingStatus status;

public:
    Booking(
        string bookingId,
        string showId,
        vector<string> seatIds
    ) {
        this->bookingId = bookingId;
        this->showId = showId;
        this->seatIds = seatIds;
        this->status = CONFIRMED;
    }

    string getBookingId() const {
        return bookingId;
    }

    string getShowId() const {
        return showId;
    }

    const vector<string>& getSeatIds() const {
        return seatIds;
    }

    BookingStatus getStatus() const {
        return status;
    }

    void cancel() {
        status = CANCELLED;
    }
};


/*
    Main service that coordinates cinemas,
    shows, seat availability, booking,
    and cancellation.
*/
class MovieBookingSystem {
private:
    // cinema ID -> cinema
    unordered_map<string, Cinema> cinemas;

    // show ID -> show
    unordered_map<string, Show> shows;

    // booking ID -> booking
    unordered_map<string, Booking> bookings;

public:
    /*
        Adds a cinema if its ID is unique.
    */
    bool addCinema(
        const Cinema& cinema
    ) {
        string cinemaId =
            cinema.getCinemaId();

        if (cinemas.count(cinemaId))
            return false;

        cinemas.emplace(cinemaId, cinema);
        return true;
    }

    /*
        Adds a show only when the cinema
        and screen are valid.
    */
    bool addShow(
        const Show& show
    ) {
        if (shows.count(show.getShowId()))
            return false;

        auto cinemaIt =
            cinemas.find(show.getCinemaId());

        if (cinemaIt == cinemas.end())
            return false;

        if (!cinemaIt->second.hasScreen(
                show.getScreenId()
            ))
            return false;

        shows.emplace(show.getShowId(), show);
        return true;
    }

    /*
        Lists show IDs for a particular movie.
    */
    vector<string> listShows(
        const string& movieName
    ) const {
        vector<string> result;

        for (const auto& entry : shows) {
            const Show& show = entry.second;

            if (show.getMovieName() ==
                movieName) {
                result.push_back(
                    show.getShowId()
                );
            }
        }

        return result;
    }

    /*
        Returns all currently available
        seat IDs for a show.
    */
    vector<string> getAvailableSeats(
        const string& showId
    ) const {
        vector<string> availableSeats;

        auto showIt = shows.find(showId);

        if (showIt == shows.end())
            return availableSeats;

        const Show& show = showIt->second;

        auto cinemaIt =
            cinemas.find(show.getCinemaId());

        if (cinemaIt == cinemas.end())
            return availableSeats;

        const vector<Seat>& seats =
            cinemaIt->second.getSeats(
                show.getScreenId()
            );

        for (const Seat& seat : seats) {
            if (show.isSeatAvailable(
                    seat.getSeatId()
                )) {
                availableSeats.push_back(
                    seat.getSeatId()
                );
            }
        }

        return availableSeats;
    }

    /*
        Books selected seats only after
        validating the entire request.
    */
    bool bookSeats(
        const string& bookingId,
        const string& showId,
        const vector<string>& seatIds
    ) {
        // Booking IDs must be unique.
        if (bookings.count(bookingId))
            return false;

        if (seatIds.empty())
            return false;

        auto showIt = shows.find(showId);

        if (showIt == shows.end())
            return false;

        Show& show = showIt->second;

        auto cinemaIt =
            cinemas.find(show.getCinemaId());

        if (cinemaIt == cinemas.end())
            return false;

        const Cinema& cinema =
            cinemaIt->second;

        /*
            Prevent duplicate seat IDs
            inside the same request.
        */
        unordered_set<string> uniqueSeatIds;

        /*
            Validate every requested seat
            before changing any state.
        */
        for (const string& seatId : seatIds) {
            if (uniqueSeatIds.count(seatId))
                return false;

            uniqueSeatIds.insert(seatId);

            if (!cinema.screenHasSeat(
                    show.getScreenId(),
                    seatId
                ))
                return false;

            if (!show.isSeatAvailable(seatId))
                return false;
        }

        /*
            All validation succeeded,
            so mark every seat as booked.
        */
        for (const string& seatId : seatIds)
            show.bookSeat(seatId);

        Booking booking(
            bookingId,
            showId,
            seatIds
        );

        bookings.emplace(
            bookingId,
            booking
        );

        return true;
    }

    /*
        Cancels a confirmed booking and
        releases all of its seats.
    */
    bool cancelBooking(
        const string& bookingId
    ) {
        auto bookingIt =
            bookings.find(bookingId);

        if (bookingIt == bookings.end())
            return false;

        Booking& booking =
            bookingIt->second;

        if (booking.getStatus() ==
            CANCELLED)
            return false;

        auto showIt =
            shows.find(booking.getShowId());

        if (showIt == shows.end())
            return false;

        Show& show = showIt->second;

        for (const string& seatId :
             booking.getSeatIds()) {
            show.releaseSeat(seatId);
        }

        booking.cancel();
        return true;
    }
};


int main() {
    MovieBookingSystem system;

    /*
        Create seats for one screen.
    */
    Screen screen1("SCREEN-1");

    screen1.addSeat(Seat("A1", 1, 1));
    screen1.addSeat(Seat("A2", 1, 2));
    screen1.addSeat(Seat("A3", 1, 3));
    screen1.addSeat(Seat("B1", 2, 1));
    screen1.addSeat(Seat("B2", 2, 2));
    screen1.addSeat(Seat("B3", 2, 3));

    /*
        Create a cinema and add the screen.
    */
    Cinema cinema(
        "CINEMA-1",
        "PVR Downtown",
        "Seattle"
    );

    cinema.addScreen(screen1);
    system.addCinema(cinema);

    /*
        Add two shows on the same screen.

        Their seat availability is independent.
    */
    Show eveningShow(
        "SHOW-1",
        "Inception",
        "CINEMA-1",
        "SCREEN-1",
        "06:00 PM"
    );

    Show nightShow(
        "SHOW-2",
        "Inception",
        "CINEMA-1",
        "SCREEN-1",
        "09:00 PM"
    );

    system.addShow(eveningShow);
    system.addShow(nightShow);

    /*
        Book A1 and A2 for the evening show.
    */
    bool booked = system.bookSeats(
        "BOOKING-1",
        "SHOW-1",
        {"A1", "A2"}
    );

    cout << "Booking successful: "
         << booked
         << endl;

    /*
        A1 cannot be booked again
        for the same show.
    */
    bool duplicateBooking =
        system.bookSeats(
            "BOOKING-2",
            "SHOW-1",
            {"A1"}
        );

    cout << "Second booking successful: "
         << duplicateBooking
         << endl;

    /*
        A1 is still available for the
        separate 9 PM show.
    */
    bool nightBooking =
        system.bookSeats(
            "BOOKING-3",
            "SHOW-2",
            {"A1"}
        );

    cout << "Night booking successful: "
         << nightBooking
         << endl;

    /*
        Cancel the first booking.
        A1 and A2 become available again
        for SHOW-1.
    */
    bool cancelled =
        system.cancelBooking("BOOKING-1");

    cout << "Cancellation successful: "
         << cancelled
         << endl;

    /*
        Display available seats for SHOW-1.
    */
    vector<string> availableSeats =
        system.getAvailableSeats("SHOW-1");

    cout << "Available seats for SHOW-1: ";

    for (const string& seatId :
         availableSeats) {
        cout << seatId << " ";
    }

    cout << endl;

    return 0;
}
```

---

# 8. End-to-end workflow

## Creating the cinema

```text
Create seats
    ↓
Add seats to a screen
    ↓
Add screen to a cinema
    ↓
Add cinema to MovieBookingSystem
```

## Creating a show

```text
Receive show details
    ↓
Validate show ID is unique
    ↓
Validate cinema exists
    ↓
Validate screen exists in cinema
    ↓
Store the show
```

## Booking seats

```text
Receive booking ID, show ID, and seat IDs
    ↓
Validate booking ID is unique
    ↓
Validate show exists
    ↓
Validate every physical seat exists
    ↓
Validate every seat is available
    ↓
Mark all seats booked
    ↓
Create and store booking
```

## Cancelling a booking

```text
Find booking
    ↓
Validate it is confirmed
    ↓
Find its show
    ↓
Release booked seats
    ↓
Mark booking cancelled
```

---

# 9. Complexity

Let:

* `C` be the number of cinemas
* `H` be the number of shows
* `S` be the number of seats in a screen
* `B` be the number of seats in one booking

What I would say:

> “Adding a cinema takes average O(1).
>
> Adding a show takes average O(1), because cinema and screen lookups use hash maps.
>
> Listing shows for a movie takes O(H), because the current version scans all shows.
>
> Getting available seats takes O(S).
>
> Booking seats takes O(B × S) in this simple version because each requested seat is checked against the screen’s seat vector.
>
> Cancelling a booking takes O(B).
>
> The system stores O(C + H + total bookings + total booked seats).”

### Possible optimization

> “If I store the seats of a screen in an `unordered_map` or `unordered_set`, booking validation becomes average O(B) instead of O(B × S).”

---

# 10. Important design decisions

## Why is booking status not stored in `Seat`?

> “A `Seat` is a physical seat inside a screen.
>
> The same seat can be booked for one show and available for another show.
>
> Therefore, booking state belongs to `Show`, not `Seat`.”

---

## Why use selected seat IDs instead of automatic allocation?

> “Allowing customers to select exact seats keeps the core booking flow simple.
>
> Automatic adjacent-seat allocation is a separate algorithmic responsibility and can be added later through a seat-selection strategy.”

---

## Why keep one `MovieBookingSystem` service?

> “For this interview-sized implementation, one service avoids unnecessary manager and repository boilerplate.
>
> In a larger system, I could separate it into `CinemaService`, `ShowService`, and `BookingService`.”

---

## Why keep cancelled bookings?

> “A cancelled booking may still be needed for history, auditing, refunds, and customer support.
>
> Therefore, I update its status instead of deleting it.”

---

## Why validate all seats before booking any?

> “If I update seats one by one and later discover an invalid seat, the system could leave a partially completed booking.
>
> Validating the full request before modification provides atomic behavior within this in-memory implementation.”

---

## Why not use an abstract class?

> “The current requirements do not yet contain interchangeable algorithms.
>
> I would add an abstraction only when there are multiple implementations, such as multiple seat-allocation strategies or payment methods.”

---

# 11. Concurrency discussion

What I would say:

> “The current code is correct for a single-threaded in-memory interview implementation.
>
> In a concurrent system, two users could check the same seat simultaneously and both see it as available.
>
> The availability check and booking update must therefore be one atomic operation.”

### Race condition

```text
User A checks A1 → available
User B checks A1 → available
User A books A1
User B books A1
```

### In one C++ process

> “I can maintain one mutex per show and lock it during the complete validation-and-booking operation.”

Conceptually:

```cpp
lock show
validate all seats
book all seats
create booking
unlock show
```

### In a distributed production system

> “I would use a database transaction with row-level locking or optimistic locking using a version number.
>
> I could also create a temporary seat hold with an expiration time while payment is being processed.”

---

# 12. Edge cases

## Duplicate booking ID

```text
Reject the request.
```

## Invalid show ID

```text
Reject the request.
```

## Empty seat list

```text
Reject the request.
```

## Duplicate seat IDs in one request

```text
Example: {"A1", "A1"}

Reject the complete request.
```

## Seat does not belong to the screen

```text
Reject the complete request.
```

## One requested seat is already booked

```text
Reject the entire booking.

Do not partially book the remaining seats.
```

## Cancelling an unknown booking

```text
Return false.
```

## Cancelling the same booking twice

```text
Return false on the second attempt.
```

## Same seat in different shows

```text
Allowed because each show maintains
its own booked-seat set.
```

---

# 13. Follow-up requirements

## Add temporary seat locking

What I would say:

> “I would introduce a `SeatHold` object containing the show ID, seat IDs, user ID, and expiration time.
>
> Seats would move through `AVAILABLE`, `HELD`, and `BOOKED` states.
>
> An expired hold would release the seats automatically.”

---

## Add payment processing

What I would say:

> “I would first hold the seats, initiate payment, and confirm the booking only after payment succeeds.
>
> If payment fails or times out, I would release the seat hold.”

Possible abstraction:

```cpp
class PaymentProcessor {
public:
    virtual bool pay(
        string userId,
        double amount
    ) = 0;

    virtual ~PaymentProcessor() = default;
};
```

---

## Add automatic seat selection

What I would say:

> “I would introduce a `SeatSelectionStrategy` abstract class.”

```cpp
class SeatSelectionStrategy {
public:
    virtual vector<string> selectSeats(
        const vector<string>& availableSeats,
        int seatCount
    ) = 0;

    virtual ~SeatSelectionStrategy() = default;
};
```

Possible implementations:

```text
FirstAvailableStrategy
AdjacentSeatStrategy
BestViewSeatStrategy
```

---

## Add different seat prices

What I would say:

> “I would add a `SeatType`, such as regular, premium, or recliner, and associate a base price with each type.
>
> The final show price could be calculated using a separate pricing strategy.”

---

## Add search by city and movie

What I would say:

> “The current system scans all shows.
>
> For faster searches, I would maintain indexes such as:
>
> * movie name → show IDs
> * city → cinema IDs
> * cinema ID → show IDs”

---

## Split the service into multiple classes

What I would say:

> “When the application grows, I would separate responsibilities into:
>
> * `CinemaService`
> * `ShowService`
> * `BookingService`
> * `PaymentService`
>
> For the scoped interview version, one main service keeps the code manageable.”

---

# 14. What was intentionally removed from the larger solution?

```text
1. Observer pattern
2. Separate manager classes
3. Separate listing and cache classes
4. Automatic continuous-seat allocation
5. Fallback non-continuous allocation
6. Payment handling
7. Temporary seat locking
8. Free-seat count cache
9. Complex show sorting
10. Notification handling
```

What I would say:

> “These are valid extensions, but they are not necessary to demonstrate the central object model and booking workflow.
>
> I would first complete the core design and add these only if the interviewer asks.”

---

# 15. Suggested interview time allocation

## First 5–8 minutes

```text
Ask clarifying questions
Confirm assumptions
Define scope
```

## Next 5–7 minutes

```text
Identify entities
Explain relationships
Highlight that booking state belongs to Show
```

## Next 25–30 minutes

```text
Implement:
- Seat
- Screen
- Cinema
- Show
- Booking
- MovieBookingSystem
- bookSeats()
- cancelBooking()
```

## Next 5–8 minutes

```text
Walk through one booking
Discuss validation and complexity
```

## Remaining time

```text
Discuss concurrency
Add one requested extension
```

---

# 16. Final interview summary

What I would say:

> “`Seat` represents a physical seat, and `Screen` contains those seats.
>
> `Cinema` contains screens.
>
> `Show` connects a movie to a cinema screen at a specific time and maintains the booked seats for that particular show.
>
> `Booking` records the selected seats and booking status.
>
> `MovieBookingSystem` coordinates show creation, availability checks, booking, and cancellation.
>
> The key design decision is that availability belongs to the show rather than the physical seat.
>
> The design is small enough for a 45–60 minute interview, while payment, temporary locks, pricing, concurrency, and automatic seat allocation can be added as follow-up requirements.”
