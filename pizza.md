## 1. Start with the core requirements

“I’ll first support creating a pizza with a size, adding multiple servings of toppings, and calculating the final price.

Then I’ll add two kinds of business rules:

* validation rules that decide whether a topping can be added;
* pricing rules that calculate discounts.

I’ll keep these rules outside the Pizza class so that Pizza does not become a large collection of `if-else` conditions.”

```cpp
enum PizzaSize {
    SMALL,
    MEDIUM,
    LARGE
};
```

---

## 2. Create the basic Topping entity

“I’ll start with a simple `Topping` class. A topping only needs a name and a price per serving.

I’m keeping it as an entity instead of storing prices directly inside Pizza because the shop owns the topping menu and pricing information.”

```cpp
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
```

---

## 3. Create the Pizza entity

“The `Pizza` class represents one customer’s pizza.

It stores its size, base price, and selected toppings.

For toppings, I’ll use a map from topping name to serving count. This lets me quickly update the number of servings and avoids storing duplicate topping objects.”

```cpp
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
};
```

---

## 4. Add basic topping operations to Pizza

“I’ll now add small helper methods to Pizza.

`addTopping` updates the serving count.

`hasTopping` is useful for combination rules.

`getToppingCount` can support future rules based on quantity.”

```cpp
// These functions belong inside the Pizza class.

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
```

---

## 5. Introduce validation rules

“Now I’ll handle topping restrictions.

Instead of placing every restriction inside `PizzaShop::addTopping`, I’ll define a common `ValidationRule` interface.

Each validation rule receives the current pizza and the topping being added. It returns false and provides an error message when the operation is invalid.”

```cpp
class ValidationRule {
public:
    virtual bool validate(
        Pizza &pizza,
        string toppingName,
        string &errorMessage
    ) = 0;

    virtual ~ValidationRule() = default;
};
```

“I’m using an abstract class because the shop should be able to execute all validation rules in the same way without knowing their concrete types.”

---

## 6. Add the cheese-burst size rule

“The first rule says that cheese burst cannot be added to a small pizza.

This class contains only that rule, so it is easy to modify or remove independently.”

```cpp
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
```

---

## 7. Add the incompatible-toppings rule

“The next rule prevents cheese burst and mushroom from existing together.

I need to check both directions because either topping may be added first.”

```cpp
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
```

---

## 8. Introduce pricing rules

“Validation and pricing are different responsibilities, so I’ll define a separate `PricingRule` abstraction.

Every pricing rule receives the pizza, topping, and number of servings, and returns the calculated price for that topping.”

```cpp
class PricingRule {
public:
    virtual double calculatePrice(
        Pizza &pizza,
        Topping &topping,
        int servings
    ) = 0;

    virtual ~PricingRule() = default;
};
```

“This allows new discounts to be added without changing the Pizza entity.”

---

## 9. Add default topping pricing

“The default calculation is simply price per serving multiplied by the serving count.”

```cpp
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
```

“In this implementation, the shop already performs this default calculation directly. This class represents the default rule concept, although it is not registered in `main`.”

---

## 10. Add the corn discount

“For corn, the first two servings remain full price. Any servings after two receive a 30% discount.

I separate the normal and discounted servings and calculate both portions.”

```cpp
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
```

“For four corn servings at $1 each, the price would be two dollars for the first two servings and one dollar forty cents for the next two, giving $3.40.”

---

## 11. Add the large-pizza onion discount

“The onion rule depends on both the topping and the pizza size.

On a large pizza, onion receives a 20% discount. Otherwise, its regular price is returned.”

```cpp
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
```

---

## 12. Create the PizzaShop orchestration class

“Now I need a class that owns the menu and coordinates the operations.

`PizzaShop` stores:

* the available toppings;
* all validation rules;
* all pricing rules.

The Pizza object stores customer selections, while PizzaShop stores business configuration.”

```cpp
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
};
```

“I’m using a map for the menu because toppings are retrieved by name. The rule collections are vectors because I only need to execute them sequentially.”

---

## 13. Implement adding a topping

“When adding a topping, I’ll perform the checks in a clear order:

First, verify that the topping exists in the menu.

Second, validate that the serving count is positive.

Third, run all registered validation rules.

Only after every rule passes do I modify the Pizza object.”

```cpp
// This function belongs inside the PizzaShop class.

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
```

“The Pizza class does not validate business rules itself because those rules may change independently of the pizza data model.”

---

## 14. Implement price calculation

“To calculate the final price, I’ll begin with the pizza’s base price.

Then I’ll iterate through each topping and its serving count.

The regular topping price is my default. I then run the registered pricing rules and use the first rule that changes the price.”

```cpp
// This function belongs inside the PizzaShop class.

double calculatePrice(Pizza &pizza) {
    double totalPrice = pizza.getBasePrice();

    for (auto entry : pizza.getToppings()) {
        string toppingName = entry.first;
        int servings = entry.second;

        Topping *topping = toppingMenu[toppingName];

        double toppingPrice =
            topping->getPrice() * servings;

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
```

“For the current requirements, I’m assuming at most one special pricing rule applies to a topping, so I stop after finding the first matching rule.”

“If the interviewer wants multiple discounts to be combined, I would change the rule contract so that each rule receives the current price and transforms it.”

---

## 15. Add a display function

“I’ll add a small display method to demonstrate the result. It prints the pizza size, base price, toppings, and calculated final price.”

```cpp
// This function belongs inside the PizzaShop class.

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
```

---

## 16. Wire everything together

“Finally, I’ll create the shop configuration.

I add the available toppings, register the validation and pricing rules, and then create a large pizza.”

```cpp
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
```

“Now I’ll add four servings of corn, two servings of onion, and one serving of mushroom.”

```cpp
    shop.addTopping(pizza, "corn", 4);
    shop.addTopping(pizza, "onion", 2);
    shop.addTopping(pizza, "mushroom", 1);
```

“When I try to add cheese burst, the combination rule rejects it because mushroom is already present.”

```cpp
    shop.addTopping(pizza, "cheese_burst", 1);

    shop.printPizza(pizza);

    return 0;
}
```

---

## 17. Walk through the final price

“The large pizza starts at $12.

Four corn servings cost $3.40 because the last two servings receive a 30% discount.

Two onion servings normally cost $3, but on a large pizza they receive a 20% discount, so they cost $2.40.

One mushroom serving costs $2.

Cheese burst is rejected, so it contributes nothing.

The final price is:

$12 + $3.40 + $2.40 + $2 = $19.80.”

---

## 18. Complexity

“Adding a topping runs every validation rule, so its complexity is O(V), where V is the number of validation rules.

Price calculation processes every selected topping and may check every pricing rule, so it is O(T × P), where T is the number of topping types on the pizza and P is the number of pricing rules.

The topping menu lookup is O(log M) because this implementation uses `map`, where M is the number of menu toppings.”

---

## 19. Design summary

“The main entities are `Pizza` and `Topping`.

`PizzaShop` coordinates menu lookup, validation, and price calculation.

Validation rules prevent invalid states before the pizza is modified.

Pricing rules isolate discount logic from the Pizza class.

This design lets me add another restriction or discount by creating another rule class and registering it with the shop, instead of modifying one large calculation function.”

---

## 20. Follow-up: Why not use the Decorator pattern?

“I would not use Decorator for the initial version because toppings have quantities, topping combinations must be validated, and discounts may depend on the overall pizza.

Representing every topping serving as another nested decorator would make quantity lookup and cross-topping validation more difficult.

A map of topping counts plus separate rule objects is more direct for these requirements.”

---

## 21. Follow-up: What would I improve in production?

“For an interview-sized implementation, raw pointers keep the setup short.

In production, I would use `unique_ptr` or `shared_ptr` depending on ownership, replace topping-name strings with enums or stable IDs, and avoid comparing floating-point prices to detect whether a rule matched.

I would likely add an explicit `supports` method to `PricingRule`.”

```cpp
class PricingRule {
public:
    virtual bool supports(
        Pizza &pizza,
        Topping &topping
    ) = 0;

    virtual double calculatePrice(
        Pizza &pizza,
        Topping &topping,
        int servings
    ) = 0;

    virtual ~PricingRule() = default;
};
```

“Then the calculation would apply the first rule whose `supports` method returns true, which is clearer than detecting a price change.”
