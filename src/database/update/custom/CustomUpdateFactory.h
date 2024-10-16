#ifndef SPHINXD_CUSTOMUPDATEFACTORY_H
#define SPHINXD_CUSTOMUPDATEFACTORY_H

#include    "CustomUpdate.h"
#include    <functional>
#include    <map>
#include    <string>

/**
 * @class CustomUpdateFactory
 *
 * @brief Factory class for creating instances of custom updates based on their names.
 *
 * This class implements the Factory Design Pattern to manage the creation of custom database updates. It allows
 * for the registration of custom update classes with their associated names, and then later creating instances of
 * these updates by name. This approach centralizes the creation logic and decouples the instantiation of custom updates
 * from their usage, facilitating easier management and scalability of database update mechanisms.
 *
 * Usage:
 * Custom updates are registered using the REGISTER_CUSTOM_UPDATE macro, which associates a class with a name.
 * Instances of registered updates are then created by calling createUpdate with the name of the desired update.
 */
class CustomUpdateFactory {
public:

    /**
     * Creates an instance of a custom update associated with the given name.
     *
     * @param updateName The name of the custom update to create.
     * @return A pointer to the newly created CustomUpdate instance, or nullptr if no update is associated with the name.
     */
    static CustomUpdate* createUpdate(const std::string& updateName);

    /**
     * Registers a constructor for a custom update class with a given name.
     *
     * This method associates a custom update's name with a function that creates an instance of that update,
     * allowing for later instantiation by name using createUpdate.
     *
     * @param updateName The name to associate with the custom update class.
     * @param constructor A function returning a pointer to a new instance of the associated custom update class.
     * @return True if the registration was successful.
     */
    static bool registerUpdate(const std::string& updateName, std::function<CustomUpdate*()> constructor);

private:

    /**
     * Retrieves the static registry mapping update names to their constructors.
     *
     * @return A reference to the map storing the association between custom update names and their constructors.
     */
    static std::map<std::string, std::function<CustomUpdate*()>>& getRegistry();
};

#define REGISTER_CUSTOM_UPDATE(NAME, CLASS) \
    static bool _ ## CLASS ## _registered = \
        CustomUpdateFactory::registerUpdate(NAME, []() -> CustomUpdate* { return new CLASS(); });


#endif //SPHINXD_CUSTOMUPDATEFACTORY_H
