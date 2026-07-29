## 1. Start with the required operations

“I’ll first identify the main operations. We need to create an order, rate an order, get the top-rated restaurants overall, and get the top-rated restaurants for a particular food item.”

```cpp
class Solution {
public:
    void order_food(string orderId,
                    string restaurantId,
                    string foodItemId);

    void rate_order(string orderId, int rating);

    vector<string> get_top_restaurants_by_food(
        string foodItemId
    );

    vector<string> get_top_rated_restaurants();
};
```

“I’ll keep `Solution` as the API exposed to the caller, while the internal classes handle order storage and rating calculations.”

---

## 2. Create the basic Order entity

“The main entity is an `Order`. For every order, I need its ID, restaurant ID, food item ID, and rating.”

“The rating starts at zero because the order has not been rated yet.”

```cpp
class Order {
private:
    string orderId;
    string restaurantId;
    string foodItemId;
    int rating;

public:
    Order(string orderId,
          string restaurantId,
          string foodItemId,
          int rating) {
        this->orderId = orderId;
        this->restaurantId = restaurantId;
        this->foodItemId = foodItemId;
        this->rating = rating;
    }

    void setRating(int rating) {
        this->rating = rating;
    }

    string getRestaurantId() const {
        return restaurantId;
    }

    string getFoodItemId() const {
        return foodItemId;
    }

    int getRating() const {
        return rating;
    }
};
```

“I’m keeping the fields private and exposing only the operations required by the rest of the system.”

---

## 3. Store rating aggregates instead of every rating

“To calculate averages, I do not need to store every individual rating. I only need the total sum and number of ratings.”

“This gives constant-time updates and constant-time average calculation.”

```cpp
class Rating {
private:
    int sum;
    int count;

public:
    Rating(int sum = 0, int count = 0) {
        this->sum = sum;
        this->count = count;
    }

    void add(int rating) {
        sum += rating;
        count++;
    }

    double getAverageRating() const {
        if (count == 0) {
            return 0;
        }

        double average =
            static_cast<double>(sum) / count;

        return round(average * 10.0) / 10.0;
    }
};
```

“I explicitly cast to `double` to avoid integer division, and I round to one decimal place based on the requirement.”

---

## 4. Separate the two kinds of rankings

“There are two independent views of the same rating event.”

“One view maintains overall restaurant ratings, and the other maintains restaurant ratings grouped by food item.”

“I don’t want the order-management class to directly contain both ranking implementations because that would tightly couple unrelated responsibilities.”

```cpp
class RateOrderObserver {
public:
    virtual void update(Order* order) = 0;
    virtual ~RateOrderObserver() = default;
};
```

“I’ll use a small observer interface. Any component interested in a newly rated order implements `update()`.”

“This also makes it easy to add another view later, such as top restaurants by city, without changing the order manager.”

---

## 5. Maintain overall restaurant ratings

“For overall ratings, I need fast lookup using the restaurant ID, so I’ll use an `unordered_map`.”

“The key is the restaurant ID and the value stores its accumulated rating.”

```cpp
class MostRatedRestaurants
    : public RateOrderObserver {

private:
    // restaurantId -> accumulated rating
    unordered_map<string, Rating> ratings;

public:
    void update(Order* order) override {
        string restaurantId =
            order->getRestaurantId();

        ratings[restaurantId].add(
            order->getRating()
        );
    }
};
```

“When an order is rated, this observer extracts the restaurant ID and updates its aggregate in average constant time.”

---

## 6. Return the top restaurants

“To return the top restaurants, I collect the restaurant IDs and sort them using their average ratings.”

“Restaurants with higher averages come first. If two averages are equal, I use the smaller restaurant ID as a deterministic tie-breaker.”

```cpp
// Belongs to MostRatedRestaurants
vector<string> getRestaurants(int n) {
    vector<string> restaurants;

    for (auto& entry : ratings) {
        restaurants.push_back(entry.first);
    }

    sort(restaurants.begin(),
         restaurants.end(),
         [this](const string& first,
                const string& second) {

        double firstRating =
            ratings[first].getAverageRating();

        double secondRating =
            ratings[second].getAverageRating();

        if (firstRating != secondRating) {
            return firstRating > secondRating;
        }

        return first < second;
    });

    if (restaurants.size() > n) {
        restaurants.resize(n);
    }

    return restaurants;
}
```

“For an interview-sized implementation, sorting during retrieval keeps the code simple.”

“If reads become very frequent and the number of restaurants becomes very large, we could later maintain an ordered structure or use a heap depending on the exact update and query requirements.”

---

## 7. Maintain ratings by food item

“For food-specific rankings, I need one additional level of grouping.”

“The outer map is keyed by food item, and the inner map is keyed by restaurant.”

```cpp
class MostRatedRestaurantsByFood
    : public RateOrderObserver {

private:
    // foodItemId -> restaurantId -> Rating
    unordered_map<
        string,
        unordered_map<string, Rating>
    > ratings;

public:
    void update(Order* order) override {
        string foodItemId =
            order->getFoodItemId();

        string restaurantId =
            order->getRestaurantId();

        ratings[foodItemId][restaurantId].add(
            order->getRating()
        );
    }
};
```

“This allows me to directly access all restaurant ratings for a particular food item.”

---

## 8. Get top restaurants for one food item

“First, I check whether the food item has any ratings. If not, I return an empty result.”

“Then I sort only the restaurants that have ratings for that food item.”

```cpp
// Belongs to MostRatedRestaurantsByFood
vector<string> getRestaurants(
    string foodItemId,
    int n
) {
    if (ratings.find(foodItemId)
        == ratings.end()) {
        return {};
    }

    auto& restaurantRatings =
        ratings[foodItemId];

    vector<string> restaurants;

    for (auto& entry : restaurantRatings) {
        restaurants.push_back(entry.first);
    }

    sort(restaurants.begin(),
         restaurants.end(),
         [&restaurantRatings](
             const string& first,
             const string& second
         ) {
        double firstRating =
            restaurantRatings[first]
                .getAverageRating();

        double secondRating =
            restaurantRatings[second]
                .getAverageRating();

        if (firstRating != secondRating) {
            return firstRating > secondRating;
        }

        return first < second;
    });

    if (restaurants.size() > n) {
        restaurants.resize(n);
    }

    return restaurants;
}
```

“The lambda captures `restaurantRatings` by reference because that is the specific map used while comparing restaurants for this food item.”

---

## 9. Add the order manager

“Now I need a class responsible for creating orders and updating their ratings.”

“I’ll store orders in an `unordered_map` because all rating operations begin with an order ID, and I want average constant-time lookup.”

```cpp
class OrdersManager {
private:
    // orderId -> Order
    unordered_map<string, Order> orders;

    vector<RateOrderObserver*> observers;

public:
    void orderFood(
        string orderId,
        string restaurantId,
        string foodItemId
    ) {
        Order order(
            orderId,
            restaurantId,
            foodItemId,
            0
        );

        orders.insert_or_assign(
            orderId,
            order
        );
    }
};
```

“I use `insert_or_assign` so that the provided code safely inserts a new order or replaces an existing order with the same ID.”

“In a production system, duplicate order IDs might instead be rejected.”

---

## 10. Rate the order and notify the observers

“When an order receives a rating, the manager updates the order and publishes the same event to every registered rating view.”

```cpp
// Belongs to OrdersManager
void rateOrder(string orderId, int rating) {
    Order* order = &orders.at(orderId);

    order->setRating(rating);

    notifyAll(order);
}
```

“The `at()` call fails if the order does not exist. In production code, I could check first and return an error, but here I’m assuming the caller provides a valid order ID.”

```cpp
// Belongs to OrdersManager
void addObserver(
    RateOrderObserver* observer
) {
    observers.push_back(observer);
}

private:

void notifyAll(Order* order) {
    for (RateOrderObserver* observer
         : observers) {
        observer->update(order);
    }
}
```

“The order manager does not know how each ranking is maintained. It only knows that each observer supports `update()`.”

---

## 11. Connect everything in Solution

“Finally, `Solution` owns all the objects and connects the order manager to both ranking observers.”

```cpp
class Solution {
private:
    OrdersManager ordersManager;

    MostRatedRestaurants
        mostRatedRestaurants;

    MostRatedRestaurantsByFood
        mostRatedRestaurantsByFood;

public:
    Solution() {
        ordersManager.addObserver(
            &mostRatedRestaurants
        );

        ordersManager.addObserver(
            &mostRatedRestaurantsByFood
        );
    }
};
```

“These objects are owned by `Solution`, so the observer pointers remain valid for the lifetime of the solution.”

---

## 12. Expose the required APIs

“The public methods are now mostly delegation methods. This keeps the caller-facing API simple while preserving separation internally.”

```cpp
// Belongs to Solution
void order_food(
    string orderId,
    string restaurantId,
    string foodItemId
) {
    ordersManager.orderFood(
        orderId,
        restaurantId,
        foodItemId
    );
}

void rate_order(
    string orderId,
    int rating
) {
    ordersManager.rateOrder(
        orderId,
        rating
    );
}
```

```cpp
// Belongs to Solution
vector<string>
get_top_rated_restaurants() {
    return mostRatedRestaurants
        .getRestaurants(20);
}

vector<string>
get_top_restaurants_by_food(
    string foodItemId
) {
    return mostRatedRestaurantsByFood
        .getRestaurants(foodItemId, 20);
}
```

“The limit of 20 is part of the external requirement, while the internal ranking classes accept a generic `n` so they remain reusable.”

---

## 13. Explain the complete flow

“The end-to-end flow is straightforward.”

“First, `order_food()` creates an unrated order inside `OrdersManager`.”

“Later, `rate_order()` finds that order and sets its rating.”

“The manager then notifies both observers.”

“One observer updates the restaurant’s overall rating, and the other updates the restaurant’s rating for that particular food item.”

“When a top-restaurants API is called, the appropriate observer sorts its current aggregated data and returns the first 20 restaurant IDs.”

---

## 14. Complexity

“Creating an order is average `O(1)` because it is inserted into an unordered map.”

“Rating an order is average `O(1)` for the order lookup, and each observer performs an average `O(1)` map update.”

“If there are `R` rated restaurants, getting the overall top restaurants takes `O(R log R)` because of sorting.”

“For a particular food item with `K` rated restaurants, the query takes `O(K log K)`.”

“Space usage is proportional to the number of orders, restaurants, and food-item–restaurant combinations.”

---

## 15. Mention important assumptions

“I’m assuming a rating is added only once per order.”

“In the current implementation, rating the same order again would add the new rating without removing the previous contribution.”

“If re-rating is required, I would store whether the order was previously rated and update the aggregates by subtracting the old rating before adding the new one.”

“I would also validate that ratings are within the accepted range, such as one to five, and return an error for unknown order IDs.”

---

## 16. Explain why no heap is used

“I considered using a heap, but restaurant averages change whenever a new rating arrives.”

“A standard heap does not efficiently update an existing restaurant’s position unless I add extra bookkeeping or allow stale entries.”

“Also, this API needs deterministic tie-breaking and returns rankings for many different food items.”

“For this interview implementation, constant-time rating updates followed by sorting when a top-20 query arrives is simpler and less error-prone.”

“If ranking queries heavily outnumber rating updates, I could maintain an ordered set keyed by average rating and restaurant ID, removing and reinserting a restaurant whenever its average changes.”
