# Simple Elevator System — 30-Minute Interview Version

## 1. Basic intuition

> “The elevator can be in one of three states: moving up, moving down, or idle.
>
> The behavior changes depending on the current state.
>
> For example, while moving up, requests above the current floor should be handled first, while lower-floor requests can wait for the downward journey.
>
> I’ll use the State design pattern so that each state contains its own request-handling and movement decisions.
>
> The `Elevator` class will store the current floor and pending floor requests, while the state classes decide how the elevator behaves.”

---

## 2. Main classes

> “I need:
>
> * `ElevatorState` abstract class
> * `IdleState`
> * `MovingUpState`
> * `MovingDownState`
> * `Elevator`
> * `ElevatorSystem` as the main class used by the client”

---

# 3. Build the solution step by step

## Step 1: Elevator state interface

What I would say:

> “Every elevator state should decide two things:
>
> * How to add a new floor request
> * What the elevator should do during the next movement step
>
> Therefore, I’ll create an abstract state class with `addRequest()` and `move()`.”

```cpp
class Elevator;

class ElevatorState {
public:
    virtual void addRequest(
        Elevator& elevator,
        int floor
    ) = 0;

    virtual void move(Elevator& elevator) = 0;

    virtual string getName() = 0;

    virtual ~ElevatorState() = default;
};
```

---

## Step 2: Elevator entity

What I would say:

> “The elevator stores its current floor, maximum floor, current state, and pending requests.
>
> I’ll keep two ordered sets:
>
> * `upRequests` stores floors that should be visited while moving upward
> * `downRequests` stores floors that should be visited while moving downward”

```cpp
class Elevator {
    int currentFloor;
    int maxFloor;

    set<int> upRequests;

    // Highest requested floor is processed first.
    set<int, greater<int>> downRequests;

    unique_ptr<ElevatorState> state;
};
```

### Why use two sets?

> “A set automatically keeps requests ordered and prevents duplicate floor requests.
>
> The upward set is sorted in ascending order, while the downward set is sorted in descending order.”

---

## Step 3: Idle state

What I would say:

> “When the elevator is idle, the first new request decides the direction.
>
> If the requested floor is above the current floor, the elevator starts moving up.
>
> If it is below, the elevator starts moving down.
>
> If it is the current floor, the doors can open immediately.”

```cpp
class IdleState : public ElevatorState {
public:
    void addRequest(
        Elevator& elevator,
        int floor
    ) override;

    void move(Elevator& elevator) override;

    string getName() override {
        return "Idle";
    }
};
```

---

## Step 4: Moving-up state

What I would say:

> “While moving up, any request above the current floor can be handled during the current upward journey.
>
> A request below the current floor is stored for the later downward journey.
>
> When no upward request remains, the elevator either changes direction or becomes idle.”

```cpp
class MovingUpState : public ElevatorState {
public:
    void addRequest(
        Elevator& elevator,
        int floor
    ) override;

    void move(Elevator& elevator) override;

    string getName() override {
        return "Moving Up";
    }
};
```

---

## Step 5: Moving-down state

What I would say:

> “The downward state works symmetrically.
>
> Requests below the current floor are handled during the current journey.
>
> Requests above the current floor are saved for the next upward journey.”

```cpp
class MovingDownState : public ElevatorState {
public:
    void addRequest(
        Elevator& elevator,
        int floor
    ) override;

    void move(Elevator& elevator) override;

    string getName() override {
        return "Moving Down";
    }
};
```

---

## Step 6: Elevator system

What I would say:

> “`ElevatorSystem` provides a simple interface to submit requests and run the elevator.
>
> For this problem, I’ll keep one elevator.
>
> A multiple-elevator system would additionally need a scheduler to select the best elevator.”

```cpp
class ElevatorSystem {
    Elevator elevator;

public:
    ElevatorSystem(int maxFloor)
        : elevator(maxFloor) {
    }

    void requestFloor(int floor) {
        elevator.addRequest(floor);
    }

    void run() {
        elevator.run();
    }
};
```

---

# 4. Complete code

```cpp
#include <bits/stdc++.h>
using namespace std;

class Elevator;


/*
    Abstract state class.

    Every state decides:
    1. How to handle a new floor request.
    2. How the elevator moves during the next step.
*/
class ElevatorState {
public:
    virtual void addRequest(
        Elevator& elevator,
        int floor
    ) = 0;

    virtual void move(Elevator& elevator) = 0;

    virtual string getName() = 0;

    virtual ~ElevatorState() = default;
};


/*
    Forward declarations of concrete states.
*/
class IdleState;
class MovingUpState;
class MovingDownState;


/*
    Represents one elevator.
*/
class Elevator {
private:
    int currentFloor;
    int maxFloor;

    // Floors to visit while moving upward.
    set<int> upRequests;

    // Floors to visit while moving downward.
    set<int, greater<int>> downRequests;

    unique_ptr<ElevatorState> state;

public:
    Elevator(int maxFloor);

    int getCurrentFloor() {
        return currentFloor;
    }

    /*
        Changes the current elevator state.
    */
    void setState(unique_ptr<ElevatorState> newState) {
        state = move(newState);
    }

    string getStateName() {
        return state->getName();
    }

    /*
        Validates and delegates the request
        to the current state.
    */
    void addRequest(int floor) {
        if (floor < 0 || floor > maxFloor) {
            cout << "Invalid floor: "
                 << floor << endl;
            return;
        }

        cout << "Request received for floor "
             << floor << endl;

        state->addRequest(*this, floor);
    }

    /*
        Adds a floor to the upward request queue.
    */
    void addUpRequest(int floor) {
        upRequests.insert(floor);
    }

    /*
        Adds a floor to the downward request queue.
    */
    void addDownRequest(int floor) {
        downRequests.insert(floor);
    }

    bool hasUpRequests() {
        return !upRequests.empty();
    }

    bool hasDownRequests() {
        return !downRequests.empty();
    }

    /*
        Checks whether the current floor
        is requested in the upward queue.
    */
    bool shouldStopWhileMovingUp() {
        return upRequests.count(currentFloor);
    }

    /*
        Checks whether the current floor
        is requested in the downward queue.
    */
    bool shouldStopWhileMovingDown() {
        return downRequests.count(currentFloor);
    }

    /*
        Removes the current floor from
        the upward request queue.
    */
    void completeUpRequest() {
        upRequests.erase(currentFloor);
    }

    /*
        Removes the current floor from
        the downward request queue.
    */
    void completeDownRequest() {
        downRequests.erase(currentFloor);
    }

    /*
        Opens the doors at the current floor.
    */
    void openDoors() {
        cout << "Elevator stopped at floor "
             << currentFloor
             << ". Doors opened."
             << endl;
    }

    /*
        Moves upward by one floor.
    */
    void moveUpOneFloor() {
        if (currentFloor < maxFloor) {
            currentFloor++;

            cout << "Elevator moved up to floor "
                 << currentFloor << endl;
        }
    }

    /*
        Moves downward by one floor.
    */
    void moveDownOneFloor() {
        if (currentFloor > 0) {
            currentFloor--;

            cout << "Elevator moved down to floor "
                 << currentFloor << endl;
        }
    }

    /*
        Executes one movement decision
        using the current state.
    */
    void moveOneStep() {
        state->move(*this);
    }

    /*
        Runs until all requests are completed
        and the elevator becomes idle.
    */
    void run() {
        while (hasUpRequests() ||
               hasDownRequests() ||
               getStateName() != "Idle") {
            moveOneStep();
        }

        cout << "Elevator is idle at floor "
             << currentFloor << endl;
    }
};


/*
    Behavior when the elevator is idle.
*/
class IdleState : public ElevatorState {
public:
    void addRequest(
        Elevator& elevator,
        int floor
    ) override;

    void move(Elevator& elevator) override;

    string getName() override {
        return "Idle";
    }
};


/*
    Behavior when the elevator is moving upward.
*/
class MovingUpState : public ElevatorState {
public:
    void addRequest(
        Elevator& elevator,
        int floor
    ) override;

    void move(Elevator& elevator) override;

    string getName() override {
        return "Moving Up";
    }
};


/*
    Behavior when the elevator is moving downward.
*/
class MovingDownState : public ElevatorState {
public:
    void addRequest(
        Elevator& elevator,
        int floor
    ) override;

    void move(Elevator& elevator) override;

    string getName() override {
        return "Moving Down";
    }
};


/*
    Elevator starts at the ground floor
    in the idle state.
*/
Elevator::Elevator(int maxFloor) {
    this->currentFloor = 0;
    this->maxFloor = maxFloor;
    this->state = make_unique<IdleState>();
}


/*
    Idle state request handling.

    The new request decides the direction.
*/
void IdleState::addRequest(
    Elevator& elevator,
    int floor
) {
    int currentFloor = elevator.getCurrentFloor();

    if (floor == currentFloor) {
        elevator.openDoors();
        return;
    }

    if (floor > currentFloor) {
        elevator.addUpRequest(floor);
        elevator.setState(
            make_unique<MovingUpState>()
        );
    } else {
        elevator.addDownRequest(floor);
        elevator.setState(
            make_unique<MovingDownState>()
        );
    }
}


/*
    If requests are already available,
    idle state selects a direction.
*/
void IdleState::move(Elevator& elevator) {
    if (elevator.hasUpRequests()) {
        elevator.setState(
            make_unique<MovingUpState>()
        );
    } else if (elevator.hasDownRequests()) {
        elevator.setState(
            make_unique<MovingDownState>()
        );
    }
}


/*
    Moving-up request handling.

    Higher floors belong to the current journey.
    Lower floors wait for the downward journey.
*/
void MovingUpState::addRequest(
    Elevator& elevator,
    int floor
) {
    int currentFloor = elevator.getCurrentFloor();

    if (floor == currentFloor) {
        elevator.openDoors();
    } else if (floor > currentFloor) {
        elevator.addUpRequest(floor);
    } else {
        elevator.addDownRequest(floor);
    }
}


/*
    Moving-up behavior.
*/
void MovingUpState::move(Elevator& elevator) {
    /*
        Stop if the current floor
        was requested.
    */
    if (elevator.shouldStopWhileMovingUp()) {
        elevator.completeUpRequest();
        elevator.openDoors();
    }

    /*
        Continue upward while upward
        requests remain.
    */
    if (elevator.hasUpRequests()) {
        elevator.moveUpOneFloor();
        return;
    }

    /*
        No upward requests remain.
        Change direction if downward
        requests are waiting.
    */
    if (elevator.hasDownRequests()) {
        elevator.setState(
            make_unique<MovingDownState>()
        );
    } else {
        elevator.setState(
            make_unique<IdleState>()
        );
    }
}


/*
    Moving-down request handling.

    Lower floors belong to the current journey.
    Higher floors wait for the upward journey.
*/
void MovingDownState::addRequest(
    Elevator& elevator,
    int floor
) {
    int currentFloor = elevator.getCurrentFloor();

    if (floor == currentFloor) {
        elevator.openDoors();
    } else if (floor < currentFloor) {
        elevator.addDownRequest(floor);
    } else {
        elevator.addUpRequest(floor);
    }
}


/*
    Moving-down behavior.
*/
void MovingDownState::move(Elevator& elevator) {
    /*
        Stop if the current floor
        was requested.
    */
    if (elevator.shouldStopWhileMovingDown()) {
        elevator.completeDownRequest();
        elevator.openDoors();
    }

    /*
        Continue downward while downward
        requests remain.
    */
    if (elevator.hasDownRequests()) {
        elevator.moveDownOneFloor();
        return;
    }

    /*
        No downward requests remain.
        Change direction if upward
        requests are waiting.
    */
    if (elevator.hasUpRequests()) {
        elevator.setState(
            make_unique<MovingUpState>()
        );
    } else {
        elevator.setState(
            make_unique<IdleState>()
        );
    }
}


/*
    Main class used by the client.
*/
class ElevatorSystem {
private:
    Elevator elevator;

public:
    ElevatorSystem(int maxFloor)
        : elevator(maxFloor) {
    }

    void requestFloor(int floor) {
        elevator.addRequest(floor);
    }

    void moveOneStep() {
        elevator.moveOneStep();
    }

    void run() {
        elevator.run();
    }
};


int main() {
    ElevatorSystem system(10);

    system.requestFloor(5);
    system.requestFloor(3);
    system.requestFloor(8);

    system.run();

    cout << endl;

    system.requestFloor(2);
    system.requestFloor(6);

    system.run();

    return 0;
}
```

---

# 5. Example flow

Suppose the elevator starts at floor `0`.

Requests arrive for floors:

```text
5, 3, 8
```

What I would say:

> “The elevator is initially idle.
>
> The request for floor 5 changes its state to moving up.
>
> Requests for floors 3 and 8 are also above the current floor, so they are inserted into the upward request set.
>
> Because the set is sorted, the elevator stops at floors 3, 5, and 8 in that order.
>
> Once no upward requests remain, it becomes idle.”

Expected movement:

```text
0 → 1 → 2 → 3 → 4 → 5 → 6 → 7 → 8
```

Stops:

```text
3, 5, 8
```

---

# 6. Complexity

Let:

* `R` be the number of pending floor requests
* `F` be the number of floors moved

What I would say:

> “Adding a floor request takes O(log R), because the request is inserted into an ordered set.
>
> Checking whether the elevator should stop at the current floor takes O(log R).
>
> Processing all movement takes approximately O(F log R), because the elevator checks requests while moving across floors.
>
> The system stores O(R) pending requests.”

---

# 7. Important design decisions

## Why use the State design pattern?

> “The elevator behaves differently depending on whether it is idle, moving up, or moving down.
>
> Without the State pattern, the `Elevator` class would contain many state-based `if-else` conditions.
>
> Keeping each behavior in a separate state class makes the transitions and movement logic easier to understand.”

---

## Why use an abstract state class?

> “All states support the same operations, but implement them differently.
>
> Every state must decide how to add a request and how to perform the next movement step.
>
> The abstract class provides one common interface for those behaviors.”

---

## Why use two request sets?

> “The elevator needs to remember both the current direction’s requests and requests waiting in the opposite direction.
>
> `upRequests` is sorted from lower to higher floors.
>
> `downRequests` is sorted from higher to lower floors.
>
> This naturally supports the elevator’s directional movement.”

---

## Why not use one normal queue?

> “Requests should not always be handled in arrival order.
>
> If the elevator is moving upward, it should handle nearby upward floors before reversing direction.
>
> A normal queue would make directional scheduling difficult.”

---

## Why use a set instead of a priority queue?

> “Both can provide ordered floor processing, but a set also prevents duplicate requests and lets us remove or search for a specific floor.
>
> For this simple design, that makes the implementation easier.”

---

## Why use `unique_ptr` for the state?

> “The elevator owns exactly one current state.
>
> `unique_ptr` automatically deletes the previous state whenever the elevator transitions to a new state.”

---

## Does the elevator stop immediately when a request matches its current floor?

> “Yes. If a request is made for the current floor, the elevator opens its doors immediately instead of adding a movement request.”

---

# 8. State transitions

What I would say:

> “The main state transitions are:
>
> * Idle to Moving Up when the requested floor is above
> * Idle to Moving Down when the requested floor is below
> * Moving Up to Moving Down when upward requests finish but downward requests remain
> * Moving Down to Moving Up when downward requests finish but upward requests remain
> * Moving Up or Moving Down to Idle when all requests are complete”

```text
                    higher request
           ┌──────────────────────────┐
           │                          ▼
        +------+                 +-----------+
        | Idle |                 | Moving Up |
        +------+                 +-----------+
           ▲                          │
           │                          │ no upward requests
           │ no requests              │ but downward requests
           │                          ▼
        +-------------+         +-------------+
        | Moving Down |◄────────| Direction   |
        +-------------+         | Change      |
           ▲                     +-------------+
           │
           │ lower request
           └──────────────────────────
```

---

# 9. Edge cases

## Invalid floor

> “A request below zero or above the maximum floor is rejected.”

## Duplicate request

> “The ordered set automatically ignores duplicate requests.”

## Request for current floor

> “The doors open immediately.”

## Opposite-direction request

> “The request is stored in the opposite-direction set and handled after the current journey finishes.”

## No pending requests

> “The elevator transitions to idle.”

## Request arrives while elevator is moving

> “The current state decides whether the floor belongs to the current journey or the next reverse journey.”

---

# 10. Follow-up requirements

## Support multiple elevators

What I would say:

> “I would store multiple `Elevator` objects inside `ElevatorSystem`.
>
> I would introduce an elevator-selection strategy that chooses the closest suitable elevator based on current floor, direction, and pending requests.”

Possible interface:

```cpp
class ElevatorSelectionStrategy {
public:
    virtual Elevator* selectElevator(
        vector<Elevator*>& elevators,
        int requestedFloor
    ) = 0;

    virtual ~ElevatorSelectionStrategy() = default;
};
```

---

## Support external up and down buttons

What I would say:

> “An external request should contain both the floor and desired direction.
>
> I would create a request object with `floor` and `direction` instead of accepting only a floor number.”

```cpp
enum Direction {
    UP,
    DOWN
};

class ExternalRequest {
    int floor;
    Direction direction;
};
```

---

## Support internal elevator buttons

What I would say:

> “Internal requests contain only the destination floor because the passenger is already inside the elevator.
>
> External requests contain the pickup floor and desired direction.”

---

## Add emergency state

What I would say:

> “I would add an `EmergencyState` implementing `ElevatorState`.
>
> In that state, normal requests would be rejected and the elevator could move to a designated safe floor.”

---

## Add maintenance mode

What I would say:

> “I would add a `MaintenanceState` where the elevator does not accept passenger requests.
>
> The existing elevator code would not need major changes because behavior is encapsulated inside state classes.”

---

## Improve elevator selection

What I would say:

> “For multiple elevators, the scheduler could prefer:
>
> * An idle elevator closest to the requested floor
> * An elevator already moving toward the requested floor
> * The elevator with the fewest pending requests
>
> I would keep this logic behind a scheduling strategy so the algorithm can change independently.”

---

# 11. Possible interviewer questions

## Why not keep an enum for the state?

> “An enum is enough when the state only represents data.
>
> Here, each state contains different behavior for accepting requests and moving.
>
> Using separate state classes avoids putting all behavior inside large conditional blocks.”

---

## Is this a real-world elevator algorithm?

> “This is a simplified directional scheduling algorithm.
>
> It completes requests in the current direction before reversing.
>
> A production elevator system would also consider passenger load, door timing, emergency handling, multiple elevators, request priority, hardware failures, and concurrent events.”

---

## Can a moving-up elevator accept a request below it?

> “Yes, but it does not reverse immediately.
>
> The lower-floor request is saved in the downward request set and handled after the upward journey completes.”

---

## What happens if a request above arrives while moving down?

> “It is stored in the upward request set.
>
> Once all downward requests finish, the elevator changes to the moving-up state.”

---

## Is this Strategy or State pattern?

> “This is primarily the State pattern because the elevator changes its behavior when its internal state changes.
>
> A selection algorithm for choosing among multiple elevators would be a good use of the Strategy pattern.”

---

# 12. Final interview summary

What I would say:

> “The elevator stores its current floor, pending upward requests, pending downward requests, and current state.
>
> `IdleState`, `MovingUpState`, and `MovingDownState` contain the behavior specific to each elevator state.
>
> New requests are added to either the current-direction set or the opposite-direction set.
>
> The elevator completes requests in its current direction before reversing.
>
> The State pattern keeps movement and request-handling logic out of one large `if-else` block.
>
> The current solution supports one elevator, and it can be extended with a scheduling strategy for multiple elevators.”
