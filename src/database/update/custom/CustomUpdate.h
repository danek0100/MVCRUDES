#ifndef SPHINXD_CUSTOMUPDATE_H
#define SPHINXD_CUSTOMUPDATE_H

/**
 * @class CustomUpdate
 *
 * @brief Abstract base class for implementing custom database updates.
 *
 * Provides a framework for defining custom update logic to be applied to the database schema. Each custom update is
 * encapsulated in its own class derived from CustomUpdate, with the specific update logic implemented in the update method.
 * This approach allows for flexible and manageable database schema evolution over time, accommodating changes that cannot
 * be handled through automatic update mechanisms.
 */
class CustomUpdate {
public:
    virtual int update() = 0;
    virtual ~CustomUpdate() = default;
};

#endif //SPHINXD_CUSTOMUPDATE_H
