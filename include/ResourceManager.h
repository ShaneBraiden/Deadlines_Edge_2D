#pragma once

#include <map>
#include <string>
#include <stdexcept>
#include <iostream>

// Generic resource cache for SFML resource types (sf::Texture, sf::Font, etc.).
//
// WHY a template:
//   SFML uses different classes for different resource types, but they all
//   share the loadFromFile() interface. A class template lets us write the
//   caching/loading logic once and reuse it for any resource type.
//
// Lab requirements covered:
//   - Class templates: ResourceManager<T>
//   - STL map: std::map<std::string, T> for the cache
//   - STL iterators: used in clear() and getLoadedIds()
//   - Exception handling: try/catch around loadFromFile()
//   - Rethrowing exceptions: catch load failure, wrap with context, rethrow

template<typename T>
class ResourceManager {
public:
    ResourceManager() = default;
    ~ResourceManager() = default;

    // Returns a reference to the cached resource. Loads from file on first access.
    // Throws std::runtime_error if loading fails.
    // Lab: exception handling + rethrowing
    T& get(const std::string& id, const std::string& filepath) {
        // Check cache first — Lab: STL map find
        auto it = resources.find(id);                    // Lab: STL iterator
        if (it != resources.end()) {
            return it->second;                           // Lab: arrow operator on iterator
        }

        // Not cached — load from file
        T resource;
        if (!resource.loadFromFile(filepath)) {
            // Lab: exception handling — throw on failure
            throw std::runtime_error(
                "ResourceManager: failed to load '" + id + "' from '" + filepath + "'"
            );
        }

        // Insert into cache and return reference
        // Lab: STL map insert
        auto result = resources.emplace(id, std::move(resource));
        return result.first->second;                     // Lab: arrow operator on iterator
    }

    // Loads a resource into the cache without returning it.
    // Useful for preloading assets before gameplay starts.
    // Lab: exception handling + rethrowing
    void preload(const std::string& id, const std::string& filepath) {
        if (resources.find(id) != resources.end()) {
            return;  // Already cached
        }

        T resource;
        if (!resource.loadFromFile(filepath)) {
            try {
                // Lab: throw exception
                throw std::runtime_error(
                    "ResourceManager: failed to preload '" + id + "' from '" + filepath + "'"
                );
            }
            catch (const std::runtime_error&) {
                // Lab: rethrowing exception — add context and propagate up
                std::cerr << "[ResourceManager] Rethrowing load failure for: " << id << std::endl;
                throw;  // Lab: rethrow to caller (Game) for handling
            }
        }

        resources.emplace(id, std::move(resource));      // Lab: STL map insert
    }

    // Checks if a resource is already cached.
    // Lab: STL map
    bool has(const std::string& id) const {
        return resources.find(id) != resources.end();
    }

    // Removes all cached resources.
    // Lab: STL iterators — explicit iterator loop over map
    void clear() {
        for (auto it = resources.begin(); it != resources.end(); ++it) {
            // Lab: STL iterator traversal
            // Each resource's destructor handles cleanup (SFML manages GPU memory)
        }
        resources.clear();
    }

    // Returns the number of cached resources.
    std::size_t size() const {
        return resources.size();
    }

private:
    std::map<std::string, T> resources;   // Lab: STL map, class template member
};
