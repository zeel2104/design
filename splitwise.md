# Expense Sharing App Like Splitwise — 30-Minute Interview Version

I’m following the same interview structure and level of detail as your Pizza Pricing System template. 

---

## 1. Basic intuition

> “The system should allow users to create groups, record expenses, and track who owes whom.
>
> For every expense, one user pays the complete amount, and the amount is divided equally among all participants.
>
> Instead of recalculating every expense whenever balances are requested, I’ll maintain a running balance between every pair of users.
>
> If Alice owes Bob $20, I can store that directly as a balance relationship.
>
> I’ll keep `User`, `Group`, and `Expense` as simple entities.
>
> `ExpenseSharingService` will coordinate user registration, group creation, expense addition, and balance display.”

---

## 2. Clarifying questions

What I would ask the interviewer:

> “Should an expense always be divided equally, or do we need exact and percentage splits?”

For the current requirement:

> “I’ll initially support only equal splitting.”

---

> “Can the payer also be one of the participants?”

Assumption:

> “Yes. Usually, the payer is also included in the expense.”

---

> “Should balances be maintained globally or separately for each group?”

Assumption:

> “I’ll maintain global balances, but every expense will belong to a group.”

---

> “Do we need payment settlement in the current version?”

Assumption:

> “The core requirement is adding expenses and displaying balances. I’ll discuss settlement as a follow-up.”

---

## 3. Main classes

> “I need:
>
> * `User`
> * `Expense`
> * `Group`
> * `Split` abstract class
> * `EqualSplit`
> * `SplitStrategy` abstract class
> * `EqualSplitStrategy`
> * `BalanceSheet`
> * `ExpenseSharingService` as the main service”

---

## Why do I need abstract classes?

> “Today, the requirement supports only equal splitting.
>
> However, a Splitwise-like application commonly adds exact amount and percentage splits later.
>
> Therefore, I’ll use a small `SplitStrategy` abstraction. The service depends on the strategy instead of implementing equal-split calculations directly.
>
> I’ll also use a `Split` abstraction so that `EqualSplit`, `ExactSplit`, and `PercentageSplit` can be added without changing the expense model.”

---

# 4. Build the solution step by step

## Step 1: User entity

What I would say:

> “A user only needs an ID and a name for the current requirements. I’ll use the ID as the unique identifier throughout the system.”

```cpp
class User {
private:
    string userId;
    string name;

public:
    User(string userId, string name) {
        this->userId = userId;
        this->name = name;
    }

    string getUserId() const {
        return userId;
    }

    string getName() const {
        return name;
    }
};
```

---

## Step 2: Split abstraction

What I would say:

> “A split represents one participant’s share in an expense.
>
> I’m using an abstract class because future split types may store additional information. For example, a percentage split would also store a percentage.”

```cpp
class Split {
protected:
    User* user;
    double amount;

public:
    Split(User* user) {
        this->user = user;
        this->amount = 0;
    }

    User* getUser() const {
        return user;
    }

    double getAmount() const {
        return amount;
    }

    void setAmount(double amount) {
        this->amount = amount;
    }

    virtual ~Split() = default;
};
```

---

## Step 3: Equal split

What I would say:

> “For an equal split, the individual amount is calculated by the strategy, so this class only identifies that the participant has an equal share.”

```cpp
class EqualSplit : public Split {
public:
    EqualSplit(User* user) : Split(user) {
    }
};
```

---

## Step 4: Split strategy

What I would say:

> “The calculation of participant shares is business logic.
>
> I’ll isolate it behind `SplitStrategy`, so adding exact or percentage splitting later will not require changing the main service.”

```cpp
class SplitStrategy {
public:
    virtual bool calculateSplits(
        double totalAmount,
        vector<Split*>& splits
    ) = 0;

    virtual ~SplitStrategy() = default;
};
```

---

## Step 5: Equal-split strategy

What I would say:

> “For equal splitting, I divide the total by the number of participants and assign the same amount to every split.
>
> Because floating-point division may leave a small remainder, I assign the remaining amount to the last participant.”

```cpp
class EqualSplitStrategy : public SplitStrategy {
public:
    bool calculateSplits(
        double totalAmount,
        vector<Split*>& splits
    ) override {
        if (totalAmount <= 0 || splits.empty())
            return false;

        double individualShare =
            totalAmount / splits.size();

        double assignedAmount = 0;

        for (int i = 0; i < splits.size(); i++) {
            if (i == splits.size() - 1) {
                // Give the final participant any
                // floating-point remainder.
                splits[i]->setAmount(
                    totalAmount - assignedAmount
                );
            } else {
                splits[i]->setAmount(individualShare);
                assignedAmount += individualShare;
            }
        }

        return true;
    }
};
```

### Interview note

For production code:

> “I would store money in cents using an integer instead of `double`, because floating-point values can introduce precision issues.”

---

## Step 6: Expense entity

What I would say:

> “An expense stores who paid, the total amount, the description, and every participant’s calculated share.”

```cpp
class Expense {
private:
    string expenseId;
    string description;
    double totalAmount;
    User* paidBy;
    vector<Split*> splits;

public:
    Expense(
        string expenseId,
        string description,
        double totalAmount,
        User* paidBy,
        vector<Split*> splits
    ) {
        this->expenseId = expenseId;
        this->description = description;
        this->totalAmount = totalAmount;
        this->paidBy = paidBy;
        this->splits = splits;
    }

    string getExpenseId() const {
        return expenseId;
    }

    string getDescription() const {
        return description;
    }

    double getTotalAmount() const {
        return totalAmount;
    }

    User* getPaidBy() const {
        return paidBy;
    }

    const vector<Split*>& getSplits() const {
        return splits;
    }
};
```

---

## Step 7: Group entity

What I would say:

> “A group contains users and the expenses added to that group.
>
> I’ll use a map for members so I can check membership in constant average time.”

```cpp
class Group {
private:
    string groupId;
    string groupName;

    // user ID -> User
    unordered_map<string, User*> members;

    vector<Expense*> expenses;

public:
    Group(string groupId, string groupName) {
        this->groupId = groupId;
        this->groupName = groupName;
    }

    string getGroupId() const {
        return groupId;
    }

    string getGroupName() const {
        return groupName;
    }

    bool addMember(User* user) {
        if (user == nullptr ||
            members.count(user->getUserId())) {
            return false;
        }

        members[user->getUserId()] = user;
        return true;
    }

    bool hasMember(string userId) const {
        return members.count(userId);
    }

    void addExpense(Expense* expense) {
        expenses.push_back(expense);
    }

    const vector<Expense*>& getExpenses() const {
        return expenses;
    }
};
```

---

## Step 8: Balance sheet

What I would say:

> “The balance sheet maintains the current net balance between users.
>
> `balances[A][B] = 20` means B owes A $20.
>
> In other words, the outer user is owed money by the inner user.
>
> Whenever an expense is added, I update both directions so that querying balances remains simple.”

```cpp
class BalanceSheet {
private:
    /*
        balances[A][B] = amount means
        B owes A this amount.
    */
    unordered_map<
        string,
        unordered_map<string, double>
    > balances;

public:
    void addDebt(
        string creditorId,
        string debtorId,
        double amount
    ) {
        if (creditorId == debtorId || amount <= 0)
            return;

        /*
            First adjust any opposite debt.

            Example:
            If Alice already owes Bob $30,
            and now Bob owes Alice $50,
            the net result is Bob owes Alice $20.
        */
        double oppositeDebt =
            balances[debtorId][creditorId];

        if (oppositeDebt >= amount) {
            balances[debtorId][creditorId] -= amount;

            if (abs(
                    balances[debtorId][creditorId]
                ) < 0.000001) {
                balances[debtorId].erase(creditorId);
            }

            return;
        }

        if (oppositeDebt > 0) {
            amount -= oppositeDebt;
            balances[debtorId].erase(creditorId);
        }

        balances[creditorId][debtorId] += amount;
    }

    double getAmountOwed(
        string creditorId,
        string debtorId
    ) const {
        auto creditorEntry =
            balances.find(creditorId);

        if (creditorEntry == balances.end())
            return 0;

        auto debtorEntry =
            creditorEntry->second.find(debtorId);

        if (debtorEntry ==
            creditorEntry->second.end()) {
            return 0;
        }

        return debtorEntry->second;
    }

    const unordered_map<
        string,
        unordered_map<string, double>
    >& getBalances() const {
        return balances;
    }
};
```

---

## Balance interpretation

Suppose:

* Alice pays $90
* Alice, Bob, and Charlie participate
* Each person’s share is $30

Then:

* Alice paid her own $30 share, so she does not owe herself anything
* Bob owes Alice $30
* Charlie owes Alice $30

We store:

```text
balances["alice"]["bob"] = 30
balances["alice"]["charlie"] = 30
```

---

## Step 9: Expense sharing service

What I would say:

> “`ExpenseSharingService` is the main coordinator.
>
> It stores users, groups, and the balance sheet.
>
> When an expense is added, it validates all IDs, creates participant splits, delegates split calculation to the strategy, stores the expense, and updates the balances.”

```cpp
class ExpenseSharingService {
private:
    unordered_map<string, User*> users;
    unordered_map<string, Group*> groups;

    BalanceSheet balanceSheet;

    int nextExpenseId = 1;

public:
    bool addUser(string userId, string name);

    bool createGroup(
        string groupId,
        string groupName
    );

    bool addUserToGroup(
        string groupId,
        string userId
    );

    bool addEqualExpense(
        string groupId,
        string paidByUserId,
        double totalAmount,
        vector<string> participantIds,
        string description
    );

    void showAllBalances() const;

    void showBalancesForUser(
        string userId
    ) const;
};
```

---

# 5. Complete code

```cpp
#include <bits/stdc++.h>
using namespace std;


/*
    Represents one registered user.
*/
class User {
private:
    string userId;
    string name;

public:
    User(string userId, string name) {
        this->userId = userId;
        this->name = name;
    }

    string getUserId() const {
        return userId;
    }

    string getName() const {
        return name;
    }
};


/*
    Abstract representation of one user's
    share in an expense.
*/
class Split {
protected:
    User* user;
    double amount;

public:
    Split(User* user) {
        this->user = user;
        this->amount = 0;
    }

    User* getUser() const {
        return user;
    }

    double getAmount() const {
        return amount;
    }

    void setAmount(double amount) {
        this->amount = amount;
    }

    virtual ~Split() = default;
};


/*
    Represents a participant whose expense
    share is calculated equally.
*/
class EqualSplit : public Split {
public:
    EqualSplit(User* user) : Split(user) {
    }
};


/*
    Strategy abstraction for calculating
    participant shares.
*/
class SplitStrategy {
public:
    virtual bool calculateSplits(
        double totalAmount,
        vector<Split*>& splits
    ) = 0;

    virtual ~SplitStrategy() = default;
};


/*
    Divides the total amount equally
    among all participants.
*/
class EqualSplitStrategy : public SplitStrategy {
public:
    bool calculateSplits(
        double totalAmount,
        vector<Split*>& splits
    ) override {
        if (totalAmount <= 0 || splits.empty())
            return false;

        double individualShare =
            totalAmount / splits.size();

        double assignedAmount = 0;

        for (int i = 0; i < splits.size(); i++) {
            /*
                Give the final participant any
                floating-point remainder.
            */
            if (i == splits.size() - 1) {
                splits[i]->setAmount(
                    totalAmount - assignedAmount
                );
            } else {
                splits[i]->setAmount(individualShare);
                assignedAmount += individualShare;
            }
        }

        return true;
    }
};


/*
    Represents one expense added to a group.
*/
class Expense {
private:
    string expenseId;
    string description;
    double totalAmount;
    User* paidBy;

    vector<Split*> splits;

public:
    Expense(
        string expenseId,
        string description,
        double totalAmount,
        User* paidBy,
        vector<Split*> splits
    ) {
        this->expenseId = expenseId;
        this->description = description;
        this->totalAmount = totalAmount;
        this->paidBy = paidBy;
        this->splits = splits;
    }

    string getExpenseId() const {
        return expenseId;
    }

    string getDescription() const {
        return description;
    }

    double getTotalAmount() const {
        return totalAmount;
    }

    User* getPaidBy() const {
        return paidBy;
    }

    const vector<Split*>& getSplits() const {
        return splits;
    }
};


/*
    Represents a group of users and
    its expense history.
*/
class Group {
private:
    string groupId;
    string groupName;

    // user ID -> User
    unordered_map<string, User*> members;

    vector<Expense*> expenses;

public:
    Group(string groupId, string groupName) {
        this->groupId = groupId;
        this->groupName = groupName;
    }

    string getGroupId() const {
        return groupId;
    }

    string getGroupName() const {
        return groupName;
    }

    /*
        Adds a user only if the user is not
        already a member of the group.
    */
    bool addMember(User* user) {
        if (user == nullptr)
            return false;

        string userId = user->getUserId();

        if (members.count(userId))
            return false;

        members[userId] = user;
        return true;
    }

    bool hasMember(string userId) const {
        return members.count(userId);
    }

    void addExpense(Expense* expense) {
        expenses.push_back(expense);
    }

    const vector<Expense*>& getExpenses() const {
        return expenses;
    }
};


/*
    Stores the net balance between every
    pair of users.

    balances[A][B] = amount means:
    B owes A this amount.
*/
class BalanceSheet {
private:
    unordered_map<
        string,
        unordered_map<string, double>
    > balances;

public:
    /*
        Adds a debt after first cancelling
        any debt in the opposite direction.
    */
    void addDebt(
        string creditorId,
        string debtorId,
        double amount
    ) {
        if (creditorId == debtorId || amount <= 0)
            return;

        /*
            Check whether the creditor currently
            owes money to the debtor.
        */
        double oppositeDebt =
            balances[debtorId][creditorId];

        /*
            The existing opposite debt completely
            cancels the new debt.
        */
        if (oppositeDebt >= amount) {
            balances[debtorId][creditorId] -= amount;

            if (abs(
                    balances[debtorId][creditorId]
                ) < 0.000001) {
                balances[debtorId].erase(creditorId);
            }

            return;
        }

        /*
            The new debt is larger, so remove
            the existing opposite debt and store
            only the remaining net debt.
        */
        if (oppositeDebt > 0) {
            amount -= oppositeDebt;
            balances[debtorId].erase(creditorId);
        }

        balances[creditorId][debtorId] += amount;
    }

    /*
        Returns how much debtor owes creditor.
    */
    double getAmountOwed(
        string creditorId,
        string debtorId
    ) const {
        auto creditorEntry =
            balances.find(creditorId);

        if (creditorEntry == balances.end())
            return 0;

        auto debtorEntry =
            creditorEntry->second.find(debtorId);

        if (debtorEntry ==
            creditorEntry->second.end()) {
            return 0;
        }

        return debtorEntry->second;
    }

    const unordered_map<
        string,
        unordered_map<string, double>
    >& getBalances() const {
        return balances;
    }
};


/*
    Main service that manages users, groups,
    expenses, and balances.
*/
class ExpenseSharingService {
private:
    unordered_map<string, User*> users;
    unordered_map<string, Group*> groups;

    BalanceSheet balanceSheet;

    int nextExpenseId;

public:
    ExpenseSharingService() {
        nextExpenseId = 1;
    }

    /*
        Registers a new user.
    */
    bool addUser(
        string userId,
        string name
    ) {
        if (userId.empty() ||
            name.empty() ||
            users.count(userId)) {
            return false;
        }

        users[userId] =
            new User(userId, name);

        return true;
    }

    /*
        Creates an empty group.
    */
    bool createGroup(
        string groupId,
        string groupName
    ) {
        if (groupId.empty() ||
            groupName.empty() ||
            groups.count(groupId)) {
            return false;
        }

        groups[groupId] =
            new Group(groupId, groupName);

        return true;
    }

    /*
        Adds an existing user to an
        existing group.
    */
    bool addUserToGroup(
        string groupId,
        string userId
    ) {
        if (!groups.count(groupId) ||
            !users.count(userId)) {
            return false;
        }

        return groups[groupId]->addMember(
            users[userId]
        );
    }

    /*
        Adds an expense that is divided equally
        among the provided participants.
    */
    bool addEqualExpense(
        string groupId,
        string paidByUserId,
        double totalAmount,
        vector<string> participantIds,
        string description
    ) {
        /*
            Validate the basic expense data.
        */
        if (!groups.count(groupId) ||
            !users.count(paidByUserId) ||
            totalAmount <= 0 ||
            participantIds.empty()) {
            return false;
        }

        Group* group = groups[groupId];

        /*
            The payer must belong to the group.
        */
        if (!group->hasMember(paidByUserId))
            return false;

        /*
            Prevent duplicate participants.
        */
        unordered_set<string> uniqueParticipants;

        vector<Split*> splits;

        /*
            Validate every participant and
            create an equal split for each one.
        */
        for (string participantId : participantIds) {
            if (!users.count(participantId) ||
                !group->hasMember(participantId) ||
                uniqueParticipants.count(
                    participantId
                )) {
                return false;
            }

            uniqueParticipants.insert(participantId);

            splits.push_back(
                new EqualSplit(users[participantId])
            );
        }

        /*
            Calculate how much each participant
            should pay.
        */
        EqualSplitStrategy strategy;

        if (!strategy.calculateSplits(
                totalAmount,
                splits
            )) {
            return false;
        }

        string expenseId =
            "expense_" +
            to_string(nextExpenseId++);

        Expense* expense = new Expense(
            expenseId,
            description,
            totalAmount,
            users[paidByUserId],
            splits
        );

        group->addExpense(expense);

        /*
            Every participant except the payer
            owes their share to the payer.
        */
        for (Split* split : splits) {
            string participantId =
                split->getUser()->getUserId();

            if (participantId == paidByUserId)
                continue;

            balanceSheet.addDebt(
                paidByUserId,
                participantId,
                split->getAmount()
            );
        }

        return true;
    }

    /*
        Displays every non-zero balance
        in the system.
    */
    void showAllBalances() const {
        bool hasBalance = false;

        for (const auto& creditorEntry :
             balanceSheet.getBalances()) {
            string creditorId =
                creditorEntry.first;

            for (const auto& debtorEntry :
                 creditorEntry.second) {
                string debtorId =
                    debtorEntry.first;

                double amount =
                    debtorEntry.second;

                if (amount <= 0)
                    continue;

                hasBalance = true;

                cout << users.at(debtorId)->getName()
                     << " owes "
                     << users.at(creditorId)->getName()
                     << ": $"
                     << fixed
                     << setprecision(2)
                     << amount
                     << endl;
            }
        }

        if (!hasBalance) {
            cout << "No outstanding balances."
                 << endl;
        }
    }

    /*
        Displays balances involving one user.
    */
    void showBalancesForUser(
        string userId
    ) const {
        if (!users.count(userId)) {
            cout << "User not found." << endl;
            return;
        }

        bool hasBalance = false;

        /*
            Find users who owe money
            to the requested user.
        */
        auto creditorEntry =
            balanceSheet.getBalances().find(userId);

        if (creditorEntry !=
            balanceSheet.getBalances().end()) {
            for (const auto& debtorEntry :
                 creditorEntry->second) {
                string debtorId =
                    debtorEntry.first;

                double amount =
                    debtorEntry.second;

                if (amount <= 0)
                    continue;

                hasBalance = true;

                cout << users.at(debtorId)->getName()
                     << " owes "
                     << users.at(userId)->getName()
                     << ": $"
                     << fixed
                     << setprecision(2)
                     << amount
                     << endl;
            }
        }

        /*
            Find users to whom the requested
            user owes money.
        */
        for (const auto& entry :
             balanceSheet.getBalances()) {
            string creditorId = entry.first;

            auto debtorEntry =
                entry.second.find(userId);

            if (debtorEntry == entry.second.end() ||
                debtorEntry->second <= 0) {
                continue;
            }

            hasBalance = true;

            cout << users.at(userId)->getName()
                 << " owes "
                 << users.at(creditorId)->getName()
                 << ": $"
                 << fixed
                 << setprecision(2)
                 << debtorEntry->second
                 << endl;
        }

        if (!hasBalance) {
            cout << users.at(userId)->getName()
                 << " has no outstanding balances."
                 << endl;
        }
    }
};


int main() {
    ExpenseSharingService service;

    /*
        Register users.
    */
    service.addUser("u1", "Alice");
    service.addUser("u2", "Bob");
    service.addUser("u3", "Charlie");

    /*
        Create a group and add members.
    */
    service.createGroup(
        "g1",
        "Weekend Trip"
    );

    service.addUserToGroup("g1", "u1");
    service.addUserToGroup("g1", "u2");
    service.addUserToGroup("g1", "u3");

    /*
        Alice pays $90 for all three users.

        Each person's share = $30.

        Bob owes Alice $30.
        Charlie owes Alice $30.
    */
    service.addEqualExpense(
        "g1",
        "u1",
        90.0,
        {"u1", "u2", "u3"},
        "Dinner"
    );

    cout << "After dinner:" << endl;
    service.showAllBalances();

    cout << endl;

    /*
        Bob pays $60 for Alice and Bob.

        Each person's share = $30.

        Alice now owes Bob $30.

        This cancels the existing:
        Bob owes Alice $30.
    */
    service.addEqualExpense(
        "g1",
        "u2",
        60.0,
        {"u1", "u2"},
        "Taxi"
    );

    cout << "After taxi:" << endl;
    service.showAllBalances();

    cout << endl;

    cout << "Alice's balances:" << endl;
    service.showBalancesForUser("u1");

    return 0;
}
```

---

# 6. Expected output

```text
After dinner:
Bob owes Alice: $30.00
Charlie owes Alice: $30.00

After taxi:
Charlie owes Alice: $30.00

Alice's balances:
Charlie owes Alice: $30.00
```

---

# 7. Expense workflow

What I would say while explaining:

> “Suppose Alice pays $90 for Alice, Bob, and Charlie.
>
> First, the service validates that the group and payer exist.
>
> Then it verifies that every participant belongs to the group.
>
> The equal-split strategy calculates $30 per person.
>
> Alice’s own $30 share does not create a balance because she already paid it.
>
> Bob owes Alice $30, and Charlie owes Alice $30.
>
> The expense is stored in the group, while the balance sheet stores the current net debts.”

---

# 8. Why maintain net balances?

Consider:

1. Alice pays for Bob, so Bob owes Alice $30.
2. Bob later pays for Alice, so Alice owes Bob $20.

Without netting, we might store:

```text
Bob owes Alice $30
Alice owes Bob $20
```

With netting, we store:

```text
Bob owes Alice $10
```

What I would say:

> “Net balances are easier for users to understand and reduce unnecessary settlements.”

---

# 9. Complexity

Let:

* `P` be the number of participants in the new expense
* `U` be the number of users
* `B` be the number of non-zero balance relationships

### Add a user

> “Adding a user takes O(1) average time because users are stored in an unordered map.”

### Create a group

> “Creating a group takes O(1) average time.”

### Add a user to a group

> “Adding a member takes O(1) average time.”

### Add an equal expense

> “Adding an expense takes O(P), because I validate every participant, create their splits, and update their balances.”

### Display one user’s balances

> “In this simple implementation, displaying one user’s balances can take O(B), because I may scan the balance sheet to find both incoming and outgoing balances.”

### Display all balances

> “Displaying all balances takes O(B).”

### Space complexity

> “Users and groups require O(U + G) space.
>
> Expenses require space proportional to the expense history and their participant splits.
>
> The balance sheet requires O(B), where B is the number of non-zero user-to-user balance relationships.”

---

# 10. Important design decisions

## Why use a split strategy?

> “Equal, exact, and percentage splitting use different calculation and validation rules.
>
> Keeping this behind `SplitStrategy` prevents the main expense service from becoming a large `if-else` block.”

---

## Why have both `Split` and `SplitStrategy`?

> “A `Split` stores one participant’s final share.
>
> A `SplitStrategy` calculates and validates the shares for the complete expense.
>
> They have separate responsibilities.”

---

## Why not calculate balances from expense history every time?

> “Recalculating from every historical expense would become increasingly expensive.
>
> Instead, I update the balance sheet whenever an expense is created, making balance retrieval much faster.”

---

## Why store groups separately from balances?

> “A group organizes members and expense history.
>
> A balance represents the net financial relationship between users.
>
> Users may share multiple groups, but they still need one understandable net balance.”

---

## Why store direct pairwise balances?

> “It is straightforward to update and explain in an interview.
>
> For a production system, I could also maintain a ledger of immutable balance transactions and compute materialized balances from it.”

---

## Why use raw pointers?

> “I used raw pointers to keep the interview code shorter and focused on the design.
>
> In production C++, I would use `unique_ptr` or `shared_ptr` and define clear ownership.”

---

## Why should money use integer cents?

> “`double` can introduce precision errors.
>
> In production, $10.25 should be stored as 1025 cents using an integer.”

A production version could use:

```cpp
using Money = long long;
```

---

# 11. Edge cases

## Duplicate participant

> “I reject the expense if the same user appears more than once, because that would incorrectly charge them multiple shares.”

---

## Participant not in the group

> “Every participant and the payer must belong to the group.”

---

## Invalid amount

> “Zero and negative expense amounts are rejected.”

---

## Empty participant list

> “An expense must have at least one participant.”

---

## Payer not included in the split

> “The code allows it.
>
> For example, a parent may pay for two other people without participating personally.
>
> In that case, every participant owes their calculated share to the payer.”

---

## Expense only for the payer

> “No balance is created because the payer does not owe money to themselves.”

---

## Opposite debts

> “Opposite debts are automatically cancelled to maintain one net relationship.”

---

## Concurrent expenses

What I would say:

> “In a real multi-user system, concurrent expenses could update the same user pair simultaneously.
>
> I would update the expense and balances inside one database transaction and lock or atomically update the relevant balance rows.”

---

## Duplicate API request

> “A retried request could accidentally create the same expense twice.
>
> I would accept an idempotency key and reject repeated requests with the same key.”

---

# 12. Follow-up requirements

## Support exact splits

What I would say:

> “I would create `ExactSplit` and `ExactSplitStrategy`.
>
> The strategy would verify that the sum of all participant amounts equals the total expense.”

```cpp
class ExactSplit : public Split {
public:
    ExactSplit(
        User* user,
        double amount
    ) : Split(user) {
        setAmount(amount);
    }
};
```

---

## Support percentage splits

What I would say:

> “I would create `PercentageSplit`, which stores the percentage, and a `PercentageSplitStrategy`.
>
> The strategy would verify that all percentages add up to 100.”

```cpp
class PercentageSplit : public Split {
private:
    double percentage;

public:
    PercentageSplit(
        User* user,
        double percentage
    ) : Split(user) {
        this->percentage = percentage;
    }

    double getPercentage() const {
        return percentage;
    }
};
```

---

## Settle a payment

What I would say:

> “I would add a `settlePayment` operation.
>
> If Bob owes Alice $30 and Bob pays $20, the balance becomes $10.
>
> If Bob pays the entire $30, I remove the relationship.”

Possible service method:

```cpp
bool settlePayment(
    string paidByUserId,
    string paidToUserId,
    double amount
);
```

---

## Unequal contributors

What I would say:

> “The current design assumes one payer.
>
> To support multiple contributors, I would represent payments separately from splits:
>
> * `PaymentContribution`: who paid how much
> * `Split`: who consumed how much
>
> Then the balance engine would compare every user’s payment against their share.”

---

## Simplify group debts

Suppose:

```text
Bob owes Alice $50
Charlie owes Bob $50
```

The system could simplify this to:

```text
Charlie owes Alice $50
```

What I would say:

> “This requires debt simplification across multiple users rather than only pairwise netting.
>
> I would calculate every user’s total net amount, separate creditors and debtors, and greedily match them.”

---

## Delete or edit an expense

What I would say:

> “I would first reverse the original expense’s balance updates, modify or remove the expense, and then apply the new balance changes.
>
> In production, I would preferably store immutable ledger entries and create reversal transactions instead of silently modifying financial history.”

---

# 13. Design patterns used

## Strategy pattern

Used by:

```cpp
SplitStrategy
EqualSplitStrategy
```

What I would say:

> “The Strategy pattern allows the expense service to use different split calculations without knowing their internal logic.”

---

## Service pattern

Used by:

```cpp
ExpenseSharingService
```

What I would say:

> “The service coordinates entities and business operations such as group creation, expense addition, and balance updates.”

---

## Repository pattern — production follow-up

What I would say:

> “For persistent storage, I would introduce repositories such as `UserRepository`, `GroupRepository`, and `ExpenseRepository`.
>
> This would separate database operations from business logic.”

---

# 14. Final interview summary

> “`User` represents a registered person.
>
> `Group` stores members and expense history.
>
> `Expense` records the payer, amount, description, and participant splits.
>
> `SplitStrategy` encapsulates how an expense is divided, and the current implementation uses equal splitting.
>
> `BalanceSheet` maintains net pairwise balances, where one user is the creditor and another is the debtor.
>
> `ExpenseSharingService` validates the request, calculates the splits, stores the expense, and updates balances.
>
> Adding an expense takes O(P), where P is the number of participants.
>
> The design is simple enough to implement in a 30-minute interview while remaining extensible for exact splits, percentage splits, settlements, and debt simplification.”
