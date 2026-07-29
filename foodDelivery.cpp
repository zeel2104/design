#include <bits/stdc++.h>
using namespace std;

class Order;

/*
    Observer interface.

    Any class that needs to receive order-rating updates
    must implement update().
*/
class RateOrderObserver {
public:
    virtual void update(Order* order) = 0;
    virtual ~RateOrderObserver() = default;
};


/*
    Stores one order.

    Rating is initially 0 because the order
    has not been rated yet.
*/
class Order {
private:
    string orderId;
    string restaurantId;
    string foodItemId;
    int rating;

public:
    Order(string orderId, string restaurantId,
          string foodItemId, int rating) {
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


/*
    Stores the sum and count of ratings.

    Example:
    Ratings = 4, 3, 5
    Sum = 12, Count = 3, Average = 4.0
*/
class Rating {
private:
    int sum;
    int count;

public:
    Rating(int sum = 0, int count = 0) {
        this->sum = sum;
        this->count = count;
    }

    double getAverageRating() const {
        if (count <= 0) {
            return 0;
        }

        double average = static_cast<double>(sum) / count;

        // Round the average to one decimal place.
        return round(average * 10.0) / 10.0;
    }

    void add(int rating) {
        sum += rating;
        count++;
    }
};


/*
    Maintains the overall rating of every restaurant.

    restaurantId -> Rating
*/
class MostRatedRestaurants : public RateOrderObserver {
private:
    unordered_map<string, Rating> ratings;

public:
    /*
        Called whenever an order receives a rating.
    */
    void update(Order* order) override {
        string restaurantId = order->getRestaurantId();

        ratings[restaurantId].add(order->getRating());
    }

    /*
        Returns the top N restaurants.

        Higher average rating comes first.
        If ratings are equal, smaller restaurant ID comes first.
    */
    vector<string> getRestaurants(int n) {
        vector<string> restaurants;

        for (auto& entry : ratings) {
            restaurants.push_back(entry.first);
        }

        sort(restaurants.begin(), restaurants.end(),
             [this](const string& first, const string& second) {
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
};


/*
    Maintains restaurant ratings for each food item.

    foodItemId -> restaurantId -> Rating
*/
class MostRatedRestaurantsByFood : public RateOrderObserver {
private:
    unordered_map<string, unordered_map<string, Rating>> ratings;

public:
    /*
        Called whenever an order receives a rating.
    */
    void update(Order* order) override {
        string foodItemId = order->getFoodItemId();
        string restaurantId = order->getRestaurantId();

        ratings[foodItemId][restaurantId].add(order->getRating());
    }

    /*
        Returns the top N restaurants for a food item.
    */
    vector<string> getRestaurants(string foodItemId, int n) {
        if (ratings.find(foodItemId) == ratings.end()) {
            return {};
        }

        vector<string> restaurants;
        auto& restaurantRatings = ratings[foodItemId];

        for (auto& entry : restaurantRatings) {
            restaurants.push_back(entry.first);
        }

        sort(restaurants.begin(), restaurants.end(),
             [&restaurantRatings](const string& first,
                                  const string& second) {
                 double firstRating =
                     restaurantRatings[first].getAverageRating();

                 double secondRating =
                     restaurantRatings[second].getAverageRating();

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
};


/*
    Creates orders and updates their ratings.

    This class acts as the subject in the Observer pattern.
*/
class OrdersManager {
private:
    // orderId -> Order
    unordered_map<string, Order> orders;

    // Classes that need to receive rating updates.
    vector<RateOrderObserver*> observers;

public:
    /*
        Creates and stores a new order.
    */
    void orderFood(string orderId, string restaurantId,
                   string foodItemId) {
        Order order(orderId, restaurantId, foodItemId, 0);

        orders.insert_or_assign(orderId, order);
    }

    /*
        Updates the order's rating and informs all observers.
    */
    void rateOrder(string orderId, int rating) {
        Order* order = &orders.at(orderId);

        order->setRating(rating);
        notifyAll(order);
    }

    /*
        Registers a class that needs rating updates.
    */
    void addObserver(RateOrderObserver* observer) {
        observers.push_back(observer);
    }

private:
    /*
        Sends the updated order to every observer.
    */
    void notifyAll(Order* order) {
        for (RateOrderObserver* observer : observers) {
            observer->update(order);
        }
    }
};


/*
    Entry point used by the caller.

    It connects the order manager with both
    restaurant-ranking observers.
*/
class Solution {
private:
    OrdersManager ordersManager;
    MostRatedRestaurants mostRatedRestaurants;
    MostRatedRestaurantsByFood mostRatedRestaurantsByFood;

public:
    Solution() {
        ordersManager.addObserver(&mostRatedRestaurants);
        ordersManager.addObserver(&mostRatedRestaurantsByFood);
    }

    void order_food(string orderId, string restaurantId,
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
        return mostRatedRestaurantsByFood.getRestaurants(
            foodItemId,
            20
        );
    }

    vector<string> get_top_rated_restaurants() {
        return mostRatedRestaurants.getRestaurants(20);
    }
};