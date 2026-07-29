#include <bits/stdc++.h>

using namespace std;

/*
    Requirements:
        - Create a pizza with a size
        - Add multiple servings of toppings
        - Calculate the final price
        - Validate topping combinations
        - Support pricing and discount rules

    Examples of rules:
        - Cheese burst cannot be added to a small pizza
        - Cheese burst and mushroom cannot be added together
        - Corn gets 30% discount after 2 servings
        - Onion gets 20% discount on a large pizza
*/

enum PizzaSize {
    SMALL,
    MEDIUM,
    LARGE
};

class Topping {
    string name;
    double price;

public:
    Topping(string toppingName, double toppingPrice) {
        name = toppingName;
        price = toppingPrice;
    }

    string getName() {
        return name;
    }

    double getPrice() {
        return price;
    }
};

class Pizza {
    PizzaSize size;
    double basePrice;

    // Topping name -> number of servings
    map<string, int> toppings;

public:
    Pizza(PizzaSize pizzaSize) {
        size = pizzaSize;

        if (size == SMALL)
            basePrice = 8.0;
        else if (size == MEDIUM)
            basePrice = 10.0;
        else
            basePrice = 12.0;
    }

    PizzaSize getSize() {
        return size;
    }

    double getBasePrice() {
        return basePrice;
    }

    map<string, int>& getToppings() {
        return toppings;
    }

    int getToppingCount(string toppingName) {
        if (toppings.find(toppingName) == toppings.end())
            return 0;

        return toppings[toppingName];
    }

    void addTopping(string toppingName, int servings = 1) {
        toppings[toppingName] += servings;
    }

    bool hasTopping(string toppingName) {
        return toppings.find(toppingName) != toppings.end();
    }
};

/*
    ValidationRule:
        Used for rules that allow or prevent a topping
        from being added.
*/
class ValidationRule {
public:
    virtual bool validate(
        Pizza &pizza,
        string toppingName,
        string &errorMessage
    ) = 0;

    virtual ~ValidationRule() = default;
};

/*
    Cheese burst cannot be added to a small pizza.
*/
class CheeseBurstSizeRule : public ValidationRule {
public:
    bool validate(
        Pizza &pizza,
        string toppingName,
        string &errorMessage
    ) {
        if (
            toppingName == "cheese_burst" &&
            pizza.getSize() == SMALL
        ) {
            errorMessage =
                "Cheese burst cannot be added to a small pizza.";

            return false;
        }

        return true;
    }
};

/*
    Cheese burst and mushroom cannot exist together.
*/
class CheeseBurstMushroomRule : public ValidationRule {
public:
    bool validate(
        Pizza &pizza,
        string toppingName,
        string &errorMessage
    ) {
        if (
            toppingName == "cheese_burst" &&
            pizza.hasTopping("mushroom")
        ) {
            errorMessage =
                "Cheese burst and mushroom cannot be added together.";

            return false;
        }

        if (
            toppingName == "mushroom" &&
            pizza.hasTopping("cheese_burst")
        ) {
            errorMessage =
                "Cheese burst and mushroom cannot be added together.";

            return false;
        }

        return true;
    }
};

/*
    PricingRule:
        Used for discounts or changes in topping price.
*/
class PricingRule {
public:
    virtual double calculatePrice(
        Pizza &pizza,
        Topping &topping,
        int servings
    ) = 0;

    virtual ~PricingRule() = default;
};

/*
    Default pricing:
        price = topping price * servings
*/
class DefaultPricingRule : public PricingRule {
public:
    double calculatePrice(
        Pizza &pizza,
        Topping &topping,
        int servings
    ) {
        return topping.getPrice() * servings;
    }
};

/*
    Corn rule:
        First 2 servings have normal price.
        Servings after 2 get 30% discount.
*/
class CornDiscountRule : public PricingRule {
public:
    double calculatePrice(
        Pizza &pizza,
        Topping &topping,
        int servings
    ) {
        if (topping.getName() != "corn")
            return topping.getPrice() * servings;

        int normalServings = min(servings, 2);
        int discountedServings = max(0, servings - 2);

        double normalPrice =
            normalServings * topping.getPrice();

        double discountedPrice =
            discountedServings * topping.getPrice() * 0.70;

        return normalPrice + discountedPrice;
    }
};

/*
    Onion rule:
        Onion gets 20% discount on a large pizza.
*/
class OnionLargePizzaRule : public PricingRule {
public:
    double calculatePrice(
        Pizza &pizza,
        Topping &topping,
        int servings
    ) {
        double price = topping.getPrice() * servings;

        if (
            topping.getName() == "onion" &&
            pizza.getSize() == LARGE
        ) {
            price = price * 0.80;
        }

        return price;
    }
};

class PizzaShop {
    map<string, Topping*> toppingMenu;

    vector<ValidationRule*> validationRules;
    vector<PricingRule*> pricingRules;

public:
    void addToppingToMenu(Topping *topping) {
        toppingMenu[topping->getName()] = topping;
    }

    void addValidationRule(ValidationRule *rule) {
        validationRules.push_back(rule);
    }

    void addPricingRule(PricingRule *rule) {
        pricingRules.push_back(rule);
    }

    bool addTopping(
        Pizza &pizza,
        string toppingName,
        int servings = 1
    ) {
        if (toppingMenu.find(toppingName) == toppingMenu.end()) {
            cout << "Topping not available." << endl;
            return false;
        }

        if (servings <= 0) {
            cout << "Servings must be greater than zero." << endl;
            return false;
        }

        for (ValidationRule *rule : validationRules) {
            string errorMessage;

            if (!rule->validate(
                    pizza,
                    toppingName,
                    errorMessage
                )) {
                cout << errorMessage << endl;
                return false;
            }
        }

        pizza.addTopping(toppingName, servings);

        return true;
    }

    double calculatePrice(Pizza &pizza) {
        double totalPrice = pizza.getBasePrice();

        for (auto entry : pizza.getToppings()) {
            string toppingName = entry.first;
            int servings = entry.second;

            Topping *topping = toppingMenu[toppingName];

            double toppingPrice =
                topping->getPrice() * servings;

            /*
                Apply the pricing rule matching this topping.

                For simplicity, we stop after finding the first
                special rule for the topping.
            */
            for (PricingRule *rule : pricingRules) {
                double calculatedPrice =
                    rule->calculatePrice(
                        pizza,
                        *topping,
                        servings
                    );

                if (calculatedPrice != toppingPrice) {
                    toppingPrice = calculatedPrice;
                    break;
                }
            }

            totalPrice += toppingPrice;
        }

        return totalPrice;
    }

    void printPizza(Pizza &pizza) {
        cout << "\nPizza Details" << endl;

        if (pizza.getSize() == SMALL)
            cout << "Size: Small" << endl;
        else if (pizza.getSize() == MEDIUM)
            cout << "Size: Medium" << endl;
        else
            cout << "Size: Large" << endl;

        cout << "Base price: $"
             << pizza.getBasePrice()
             << endl;

        cout << "Toppings:" << endl;

        for (auto entry : pizza.getToppings()) {
            cout << "- "
                 << entry.first
                 << " x "
                 << entry.second
                 << endl;
        }

        cout << "Final price: $"
             << fixed
             << setprecision(2)
             << calculatePrice(pizza)
             << endl;
    }
};

int main() {
    PizzaShop shop;

    Topping corn("corn", 1.0);
    Topping onion("onion", 1.5);
    Topping mushroom("mushroom", 2.0);
    Topping cheeseBurst("cheese_burst", 3.0);

    shop.addToppingToMenu(&corn);
    shop.addToppingToMenu(&onion);
    shop.addToppingToMenu(&mushroom);
    shop.addToppingToMenu(&cheeseBurst);

    CheeseBurstSizeRule sizeRule;
    CheeseBurstMushroomRule exclusionRule;

    shop.addValidationRule(&sizeRule);
    shop.addValidationRule(&exclusionRule);

    CornDiscountRule cornRule;
    OnionLargePizzaRule onionRule;

    shop.addPricingRule(&cornRule);
    shop.addPricingRule(&onionRule);

    Pizza pizza(LARGE);

    shop.addTopping(pizza, "corn", 4);
    shop.addTopping(pizza, "onion", 2);
    shop.addTopping(pizza, "mushroom", 1);

    // This will fail because mushroom is already present.
    shop.addTopping(pizza, "cheese_burst", 1);

    shop.printPizza(pizza);

    return 0;
}