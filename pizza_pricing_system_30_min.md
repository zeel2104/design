# Pizza Pricing System — 30-Minute Interview Version

## 1. Basic intuition


> “The system should create a pizza, add toppings, validate invalid combinations, and calculate the final price.
>
> I’ll keep `Pizza` and `Topping` as simple entities.
>
> Validation rules and pricing rules can change independently, so I’ll use two small abstract classes:
>
> - `ValidationRule` for whether a topping can be added
> - `PricingRule` for calculating topping price
>
> `PizzaShop` will coordinate menu lookup, validation, and final price calculation.”

---

## 2. Main classes

> “I need:
>
> - `PizzaSize` enum
> - `Topping`
> - `Pizza`
> - `ValidationRule` abstract class
> - concrete validation rules
> - `PricingRule` abstract class
> - concrete pricing rules
> - `PizzaShop` as the main service”

---

# 3. Build the solution step by step

## Step 1: Pizza size and topping

What I would say:

> “Pizza size affects the base price and some discount rules. A topping only needs a name and price.”

```cpp
enum PizzaSize {
    SMALL,
    MEDIUM,
    LARGE
};

class Topping {
    string name;
    double price;

public:
    Topping(string name, double price) {
        this->name = name;
        this->price = price;
    }

    string getName() {
        return name;
    }

    double getPrice() {
        return price;
    }
};
```

---

## Step 2: Pizza entity


> “The pizza stores its size, base price, and the number of servings for each topping.”

```cpp
class Pizza {
    PizzaSize size;
    double basePrice;

    // topping name -> servings
    unordered_map<string, int> toppings;

public:
    Pizza(PizzaSize size) {
        this->size = size;

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

    unordered_map<string, int>& getToppings() {
        return toppings;
    }

    bool hasTopping(string name) {
        return toppings.count(name);
    }

    void addTopping(string name, int servings) {
        toppings[name] += servings;
    }
};
```

### Design decision

> “I use a map from topping name to servings so multiple additions of the same topping are accumulated.”

---

## Step 3: Abstract validation rule

What I would say:

> “Different restrictions have different logic, but all of them answer whether the topping can be added.”

```cpp
class ValidationRule {
public:
    virtual bool validate(
        Pizza& pizza,
        string toppingName
    ) = 0;

    virtual ~ValidationRule() = default;
};
```

---

## Step 4: Validation rules

What I would say:

> “I’ll implement the two required restrictions as separate rule classes.”

```cpp
class CheeseBurstSizeRule : public ValidationRule {
public:
    bool validate(
        Pizza& pizza,
        string toppingName
    ) override {
        return !(toppingName == "cheese_burst" &&
                 pizza.getSize() == SMALL);
    }
};

class CheeseBurstMushroomRule : public ValidationRule {
public:
    bool validate(
        Pizza& pizza,
        string toppingName
    ) override {
        if (toppingName == "cheese_burst" &&
            pizza.hasTopping("mushroom"))
            return false;

        if (toppingName == "mushroom" &&
            pizza.hasTopping("cheese_burst"))
            return false;

        return true;
    }
};
```

### Design decision

> “Adding another validation later only requires another class implementing `ValidationRule`.”

---

## Step 5: Abstract pricing rule

What I would say:

> “Pricing rules return the final price for one topping and its servings.”

```cpp
class PricingRule {
public:
    virtual bool applies(
        Pizza& pizza,
        Topping& topping
    ) = 0;

    virtual double calculate(
        Pizza& pizza,
        Topping& topping,
        int servings
    ) = 0;

    virtual ~PricingRule() = default;
};
```

### Why add `applies()`?

> “It makes rule selection clear. The shop first checks whether the rule applies and then uses it to calculate the price.”

---

## Step 6: Pricing rules

What I would say:

> “Corn servings after two get a 30% discount. Onion gets a 20% discount on large pizzas.”

```cpp
class CornDiscountRule : public PricingRule {
public:
    bool applies(
        Pizza& pizza,
        Topping& topping
    ) override {
        return topping.getName() == "corn";
    }

    double calculate(
        Pizza& pizza,
        Topping& topping,
        int servings
    ) override {
        int normal = min(servings, 2);
        int discounted = max(0, servings - 2);

        return normal * topping.getPrice() +
               discounted * topping.getPrice() * 0.70;
    }
};

class OnionLargePizzaRule : public PricingRule {
public:
    bool applies(
        Pizza& pizza,
        Topping& topping
    ) override {
        return topping.getName() == "onion" &&
               pizza.getSize() == LARGE;
    }

    double calculate(
        Pizza& pizza,
        Topping& topping,
        int servings
    ) override {
        return topping.getPrice() * servings * 0.80;
    }
};
```

---

## Step 7: Pizza shop


> “`PizzaShop` stores the available toppings and registered rules.
>
> While adding a topping, it checks all validation rules.
>
> While calculating price, it applies the first matching special pricing rule. If no rule matches, it uses the normal topping price.”

```cpp
class PizzaShop {
    unordered_map<string, Topping*> menu;
    vector<ValidationRule*> validationRules;
    vector<PricingRule*> pricingRules;
};
```

---

# 4. Complete code

```cpp
#include <bits/stdc++.h>
using namespace std;

enum PizzaSize {
    SMALL,
    MEDIUM,
    LARGE
};


/*
    Represents one topping in the menu.
*/
class Topping {
    string name;
    double price;

public:
    Topping(string name, double price) {
        this->name = name;
        this->price = price;
    }

    string getName() {
        return name;
    }

    double getPrice() {
        return price;
    }
};


/*
    Represents one pizza.
*/
class Pizza {
    PizzaSize size;
    double basePrice;

    // topping name -> number of servings
    unordered_map<string, int> toppings;

public:
    Pizza(PizzaSize size) {
        this->size = size;

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

    unordered_map<string, int>& getToppings() {
        return toppings;
    }

    bool hasTopping(string name) {
        return toppings.count(name);
    }

    void addTopping(string name, int servings) {
        toppings[name] += servings;
    }
};


/*
    Abstract class for topping validation.
*/
class ValidationRule {
public:
    virtual bool validate(
        Pizza& pizza,
        string toppingName
    ) = 0;

    virtual ~ValidationRule() = default;
};


/*
    Cheese burst is not allowed on a small pizza.
*/
class CheeseBurstSizeRule : public ValidationRule {
public:
    bool validate(
        Pizza& pizza,
        string toppingName
    ) override {
        return !(toppingName == "cheese_burst" &&
                 pizza.getSize() == SMALL);
    }
};


/*
    Cheese burst and mushroom cannot exist together.
*/
class CheeseBurstMushroomRule : public ValidationRule {
public:
    bool validate(
        Pizza& pizza,
        string toppingName
    ) override {
        if (toppingName == "cheese_burst" &&
            pizza.hasTopping("mushroom"))
            return false;

        if (toppingName == "mushroom" &&
            pizza.hasTopping("cheese_burst"))
            return false;

        return true;
    }
};


/*
    Abstract class for topping pricing rules.
*/
class PricingRule {
public:
    virtual bool applies(
        Pizza& pizza,
        Topping& topping
    ) = 0;

    virtual double calculate(
        Pizza& pizza,
        Topping& topping,
        int servings
    ) = 0;

    virtual ~PricingRule() = default;
};


/*
    Corn servings after two get a 30% discount.
*/
class CornDiscountRule : public PricingRule {
public:
    bool applies(
        Pizza& pizza,
        Topping& topping
    ) override {
        return topping.getName() == "corn";
    }

    double calculate(
        Pizza& pizza,
        Topping& topping,
        int servings
    ) override {
        int normalServings = min(servings, 2);
        int discountedServings = max(0, servings - 2);

        double normalPrice =
            normalServings * topping.getPrice();

        double discountedPrice =
            discountedServings *
            topping.getPrice() *
            0.70;

        return normalPrice + discountedPrice;
    }
};


/*
    Onion gets a 20% discount on large pizzas.
*/
class OnionLargePizzaRule : public PricingRule {
public:
    bool applies(
        Pizza& pizza,
        Topping& topping
    ) override {
        return topping.getName() == "onion" &&
               pizza.getSize() == LARGE;
    }

    double calculate(
        Pizza& pizza,
        Topping& topping,
        int servings
    ) override {
        return topping.getPrice() *
               servings *
               0.80;
    }
};


/*
    Main service that validates toppings
    and calculates the final price.
*/
class PizzaShop {
    unordered_map<string, Topping*> menu;
    vector<ValidationRule*> validationRules;
    vector<PricingRule*> pricingRules;

public:
    void addToppingToMenu(Topping* topping) {
        menu[topping->getName()] = topping;
    }

    void addValidationRule(ValidationRule* rule) {
        validationRules.push_back(rule);
    }

    void addPricingRule(PricingRule* rule) {
        pricingRules.push_back(rule);
    }

    /*
        Adds a topping only if it exists
        and passes all validation rules.
    */
    bool addTopping(
        Pizza& pizza,
        string toppingName,
        int servings = 1
    ) {
        if (!menu.count(toppingName) || servings <= 0)
            return false;

        for (ValidationRule* rule : validationRules) {
            if (!rule->validate(pizza, toppingName))
                return false;
        }

        pizza.addTopping(toppingName, servings);
        return true;
    }

    /*
        Calculates base price plus all topping prices.
    */
    double calculatePrice(Pizza& pizza) {
        double total = pizza.getBasePrice();

        for (auto& entry : pizza.getToppings()) {
            string toppingName = entry.first;
            int servings = entry.second;

            Topping* topping = menu[toppingName];

            // Default topping price.
            double toppingPrice =
                topping->getPrice() * servings;

            // Apply the first matching special rule.
            for (PricingRule* rule : pricingRules) {
                if (rule->applies(pizza, *topping)) {
                    toppingPrice =
                        rule->calculate(
                            pizza,
                            *topping,
                            servings
                        );
                    break;
                }
            }

            total += toppingPrice;
        }

        return total;
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
    CheeseBurstMushroomRule combinationRule;

    shop.addValidationRule(&sizeRule);
    shop.addValidationRule(&combinationRule);

    CornDiscountRule cornRule;
    OnionLargePizzaRule onionRule;

    shop.addPricingRule(&cornRule);
    shop.addPricingRule(&onionRule);

    Pizza pizza(LARGE);

    shop.addTopping(pizza, "corn", 4);
    shop.addTopping(pizza, "onion", 2);
    shop.addTopping(pizza, "mushroom", 1);

    // Fails because mushroom already exists.
    shop.addTopping(pizza, "cheese_burst", 1);

    cout << fixed << setprecision(2);
    cout << "Final price: $"
         << shop.calculatePrice(pizza)
         << endl;

    return 0;
}
```

---

# 5. Complexity

Let:

- `V` be the number of validation rules
- `P` be the number of pricing rules
- `T` be the number of different toppings on the pizza

What I would say:

> “Adding a topping takes O(V) because every validation rule may be checked.
>
> Calculating the final price takes O(T × P), because each topping may check every pricing rule.
>
> The pizza stores O(T) topping entries.”

---

# 6. Important design decisions

## Why use abstract rule classes?

> “Business rules change frequently. Keeping them in separate classes prevents `PizzaShop` from becoming one large block of `if-else` statements.”

## Why have separate validation and pricing rules?

> “Validation decides whether an operation is allowed. Pricing decides how much it costs.
>
> These are different responsibilities and can change independently.”

## Why not use the Decorator pattern?

> “Decorator works well when every topping wraps a pizza object, but servings, cross-topping validation, and discount rules make that approach more complicated.
>
> Rule classes are simpler for this 30-minute interview.”

## Why use `applies()` in pricing rules?

> “It avoids guessing whether a rule changed the price. The rule clearly tells the shop whether it handles that topping.”

## Why apply only the first pricing rule?

> “For the current requirements, each topping has at most one special pricing rule.
>
> If multiple discounts must be combined, I would apply all matching rules in sequence.”

---

# 7. Follow-up requirements

## Add a new validation rule

What I would say:

> “I create another class implementing `ValidationRule` and register it with `PizzaShop`.”

## Add a new discount

What I would say:

> “I create another class implementing `PricingRule`. The existing pricing flow does not change.”

## Remove a topping

What I would say:

> “I would add a method in `Pizza` that decreases the serving count and erases the topping when the count reaches zero.”

## Allow multiple discounts on one topping

What I would say:

> “Instead of stopping after the first matching pricing rule, I would pass the current price through every matching rule.”

---

# 8. Final interview summary

What I would say:

> “`Pizza` stores size, base price, and topping servings.
>
> `ValidationRule` handles whether a topping can be added.
>
> `PricingRule` handles special pricing and discounts.
>
> `PizzaShop` coordinates the menu, rules, and final price calculation.
>
