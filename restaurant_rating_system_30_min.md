# Restaurant Rating System — 30-Minute Interview Version

## 1. Basic intuition

What I would say:

> “The system supports four operations: place an order, rate an order, get top restaurants overall, and get top restaurants for a food item.
>
> An order connects a restaurant and a food item. When the order is rated, I notify two components:
>
> - one maintains overall restaurant ratings
> - one maintains restaurant ratings by food item
>
> I’ll use a small abstract observer class because both components react to the same rating event.”

---

## 2. Main classes

> “I need:
>
> - `Order` to store order details
> - `Rating` to store rating sum and count
> - `RateOrderObserver` as the abstract class
> - `MostRatedRestaurants` for overall ratings
> - `MostRatedRestaurantsByFood` for food-specific ratings
> - `OrdersManager` to create and rate orders
> - `Solution` as the entry point”

---

## 3. Abstract observer

What I would say:

> “Both ranking classes need to update whenever an order is rated, so I’ll give them one common interface.”

```cpp
class RateOrderObserver {
public:
    virtual void update(Order* order) = 0;
    virtual ~RateOrderObserver() = default;
};
```

---

## 4. Rating and Order

What I would say:

> “`Rating` stores sum and count so I can update and calculate averages in O(1). `Order` stores the restaurant, food item, and current rating.”

```cpp
class Rating {
public:
    int sum = 0;
    int count = 0;

    void add(int value) {
        sum += value;
        count++;
    }

    double average() {
        if (count == 0) return 0;
        return (double) sum / count;
    }
};

class Order {
public:
    string orderId;
    string restaurantId;
    string foodItemId;
    int rating;

    Order(string orderId, string restaurantId,
          string foodItemId) {
        this->orderId = orderId;
        this->restaurantId = restaurantId;
        this->foodItemId = foodItemId;
        rating = 0;
    }
};
```

---

## 5. Overall restaurant ratings

What I would say:

> “I use a map from restaurant ID to its aggregate rating. When an order is rated, I update that restaurant.”

```cpp
class MostRatedRestaurants : public RateOrderObserver {
    unordered_map<string, Rating> ratings;

public:
    void update(Order* order) override {
        ratings[order->restaurantId].add(order->rating);
    }

    vector<string> getRestaurants(int n) {
        vector<string> result;

        for (auto& entry : ratings) {
            result.push_back(entry.first);
        }

        sort(result.begin(), result.end(),
             [this](string a, string b) {
                 double avgA = ratings[a].average();
                 double avgB = ratings[b].average();

                 if (avgA != avgB) return avgA > avgB;
                 return a < b;
             });

        if (result.size() > n) result.resize(n);
        return result;
    }
};
```

---

## 6. Ratings by food item

What I would say:

> “For food-specific rankings, I need two keys: food item and restaurant.”

```cpp
class MostRatedRestaurantsByFood : public RateOrderObserver {
    unordered_map<string,
        unordered_map<string, Rating>> ratings;

public:
    void update(Order* order) override {
        ratings[order->foodItemId]
               [order->restaurantId]
                   .add(order->rating);
    }

    vector<string> getRestaurants(string foodItemId, int n) {
        if (ratings.find(foodItemId) == ratings.end()) {
            return {};
        }

        auto& restaurantRatings = ratings[foodItemId];
        vector<string> result;

        for (auto& entry : restaurantRatings) {
            result.push_back(entry.first);
        }

        sort(result.begin(), result.end(),
             [&restaurantRatings](string a, string b) {
                 double avgA = restaurantRatings[a].average();
                 double avgB = restaurantRatings[b].average();

                 if (avgA != avgB) return avgA > avgB;
                 return a < b;
             });

        if (result.size() > n) result.resize(n);
        return result;
    }
};
```

---

## 7. Orders manager

What I would say:

> “`OrdersManager` stores orders. When an order is rated, it updates the order and notifies all observers.”

```cpp
class OrdersManager {
    unordered_map<string, Order> orders;
    vector<RateOrderObserver*> observers;

public:
    void orderFood(string orderId,
                   string restaurantId,
                   string foodItemId) {
        orders.emplace(
            orderId,
            Order(orderId, restaurantId, foodItemId)
        );
    }

    void rateOrder(string orderId, int rating) {
        Order& order = orders.at(orderId);
        order.rating = rating;
        notifyAll(&order);
    }

    void addObserver(RateOrderObserver* observer) {
        observers.push_back(observer);
    }

private:
    void notifyAll(Order* order) {
        for (RateOrderObserver* observer : observers) {
            observer->update(order);
        }
    }
};
```

---

# 8. Complete code

```cpp
#include <bits/stdc++.h>
using namespace std;

class Order;

/*
    Abstract observer.
    Any class interested in rating updates implements update().
*/
class RateOrderObserver {
public:
    virtual void update(Order* order) = 0;
    virtual ~RateOrderObserver() = default;
};

/*
    Stores rating sum and count.
*/
class Rating {
public:
    int sum = 0;
    int count = 0;

    void add(int value) {
        sum += value;
        count++;
    }

    double average() {
        if (count == 0) return 0;
        return (double) sum / count;
    }
};

/*
    Stores one order.
*/
class Order {
public:
    string orderId;
    string restaurantId;
    string foodItemId;
    int rating;

    Order(string orderId,
          string restaurantId,
          string foodItemId) {
        this->orderId = orderId;
        this->restaurantId = restaurantId;
        this->foodItemId = foodItemId;
        rating = 0;
    }
};

/*
    Maintains overall restaurant ratings.
*/
class MostRatedRestaurants : public RateOrderObserver {
    unordered_map<string, Rating> ratings;

public:
    void update(Order* order) override {
        ratings[order->restaurantId].add(order->rating);
    }

    vector<string> getRestaurants(int n) {
        vector<string> result;

        // Collect all restaurant IDs.
        for (auto& entry : ratings) {
            result.push_back(entry.first);
        }

        // Higher average first; restaurant ID breaks ties.
        sort(result.begin(), result.end(),
             [this](string a, string b) {
                 double avgA = ratings[a].average();
                 double avgB = ratings[b].average();

                 if (avgA != avgB) return avgA > avgB;
                 return a < b;
             });

        if (result.size() > n) result.resize(n);
        return result;
    }
};

/*
    Maintains restaurant ratings for each food item.
*/
class MostRatedRestaurantsByFood : public RateOrderObserver {
    unordered_map<string,
        unordered_map<string, Rating>> ratings;

public:
    void update(Order* order) override {
        ratings[order->foodItemId]
               [order->restaurantId]
                   .add(order->rating);
    }

    vector<string> getRestaurants(string foodItemId, int n) {
        if (ratings.find(foodItemId) == ratings.end()) {
            return {};
        }

        auto& restaurantRatings = ratings[foodItemId];
        vector<string> result;

        // Collect restaurants serving this food.
        for (auto& entry : restaurantRatings) {
            result.push_back(entry.first);
        }

        // Higher average first; restaurant ID breaks ties.
        sort(result.begin(), result.end(),
             [&restaurantRatings](string a, string b) {
                 double avgA = restaurantRatings[a].average();
                 double avgB = restaurantRatings[b].average();

                 if (avgA != avgB) return avgA > avgB;
                 return a < b;
             });

        if (result.size() > n) result.resize(n);
        return result;
    }
};

/*
    Creates orders and sends rating updates to observers.
*/
class OrdersManager {
    unordered_map<string, Order> orders;
    vector<RateOrderObserver*> observers;

public:
    void orderFood(string orderId,
                   string restaurantId,
                   string foodItemId) {
        orders.emplace(
            orderId,
            Order(orderId, restaurantId, foodItemId)
        );
    }

    void rateOrder(string orderId, int rating) {
        // Find the order and update its rating.
        Order& order = orders.at(orderId);
        order.rating = rating;

        // Notify both rating components.
        notifyAll(&order);
    }

    void addObserver(RateOrderObserver* observer) {
        observers.push_back(observer);
    }

private:
    void notifyAll(Order* order) {
        for (RateOrderObserver* observer : observers) {
            observer->update(order);
        }
    }
};

/*
    Entry point exposed to the caller.
*/
class Solution {
    OrdersManager ordersManager;
    MostRatedRestaurants mostRatedRestaurants;
    MostRatedRestaurantsByFood mostRatedRestaurantsByFood;

public:
    Solution() {
        // Register both observers.
        ordersManager.addObserver(&mostRatedRestaurants);
        ordersManager.addObserver(&mostRatedRestaurantsByFood);
    }

    void order_food(string orderId,
                    string restaurantId,
                    string foodItemId) {
        ordersManager.orderFood(
            orderId,
            restaurantId,
            foodItemId
        );
    }

    void rate_order(string orderId, int rating) {
        ordersManager.rateOrder(orderId, rating);
    }

    vector<string> get_top_restaurants_by_food(
        string foodItemId
    ) {
        return mostRatedRestaurantsByFood
            .getRestaurants(foodItemId, 20);
    }

    vector<string> get_top_rated_restaurants() {
        return mostRatedRestaurants
            .getRestaurants(20);
    }
};
```

---

## 9. Complexity

> “Placing an order takes average O(1).
>
> Rating an order takes average O(1), because order lookup and both rating updates use hash maps.
>
> Getting top restaurants overall takes O(R log R), where R is the number of rated restaurants.
>
> Getting top restaurants for a food item takes O(F log F), where F is the number of restaurants rated for that food item.”

---

## 10. Final design explanation

> “I used Observer because two separate ranking components need to react whenever an order is rated.
>
> `OrdersManager` does not know how ratings are stored. It only notifies observers.
>
> `Rating` stores sum and count, which keeps updates O(1).
>
> I sort only when the top restaurants are requested because that is simpler than maintaining a heap with changing averages.”
