#include "Common.h"
#include "Scene.h"
#include <map>
#include <memory>
#include <deque>

#include "EntityManager.h"

class Inventory : public Scene
{

protected:
    Inventory();
    void init();
    void update();
    void inventoryItems();
    void addItemToInventory(std::vector<std::vector<Inventory>>& inventory, 
                            int x, int y, Inventory item);
public:
    


    
    Inventory(GameEngine* gameEngine = nullptr);

};
