///\/\/\\\\////\/\/\//\\//\\\/\/\/\/\/\\\\////\/\/\//\\//\\\/\/\
//
//  Assignment       COMP4300 - Assignment 4
//  Professor:       David Churchill
//  Year / Term:     2022-09
//  File Name:       Scene_LevelEditor.cpp
//
//  Student Name:    Jason Lomond
//  Student User:    jblomond
//  Student Email:   jblomond@mun.ca
//  Student ID:      201900966
//  Group Member(s): [enter student name(s)]
//
///\/\/\\\\////\/\/\//\\//\\\/\/\/\/\/\\\\////\/\/\//\\//\\\/\/\

#include "Scene_LevelEditor.h"
#include "Common.h"
#include "Physics.h"
#include "Assets.h"
#include "GameEngine.h"
#include "Components.h"
#include <tgmath.h>

Scene_LevelEditor::Scene_LevelEditor(GameEngine *game, const std::string &levelPath)
    : Scene(game), m_levelPath(levelPath)
{
    std::string levelFile;
    init(m_levelPath);
}

void Scene_LevelEditor::init(const std::string &levelPath)
{
    loadLevel(levelPath);

    // Registers the actions required to play the game

    registerAction(sf::Keyboard::Escape, "QUIT");
    registerAction(sf::Keyboard::P, "PAUSE");
    registerAction(sf::Keyboard::Y, "TOGGLE_FOLLOW");  // Toggle drawing (T)extures
    registerAction(sf::Keyboard::T, "TOGGLE_TEXTURE"); // Toggle drawing (T)extures
    registerAction(sf::Keyboard::C, "TOGGLE_COLLISION");
    registerAction(sf::Keyboard::W, "UP");
    registerAction(sf::Keyboard::A, "LEFT");
    registerAction(sf::Keyboard::D, "RIGHT");
    registerAction(sf::Keyboard::S, "DOWN");
    registerAction(sf::Keyboard::Space, "ATTACK");
}

void Scene_LevelEditor::loadLevel(const std::string &filename)
{
    std::string label;

    // tile variables
    std::string tName;
    float roomX, roomY, tGridX, tGridY;
    int bMove, bVision;

    // npc variables
    std::string nName, AI;
    float nRoomX, nRoomY, nGridX, nGridY;
    int nMove, nVision, nHealth, nDamage, nSpeed, N, Xi, Yi;
    std::vector<Vec2> npcPos;

    // reset the entity manager every time we load a level
    m_entityManager = EntityManager();

    // reads the level file, adding all entities to entityManager and load playerConfig
    std::ifstream fin(filename);
    while (fin >> label)
    {
        if (label == "Tile")
        {
            // removed room x and Y temporarily
            fin >> tName >> tGridX >> tGridY >> bMove >> bVision;
            auto tile = m_entityManager.addEntity("tile");

            // add tiles to entity manager list
            tile->addComponent<CAnimation>(m_game->assets().getAnimation(tName), true);
            tile->addComponent<CTransform>(Vec2(((tGridX * 64) - 32), ((tGridY * 64) - 32)));
            tile->addComponent<CBoundingBox>(m_game->assets().getAnimation(tName).getSize(), bMove, bVision);
            tile->addComponent<CDraggable>();
        }

        if (label == "NPC")
        {
            // removed room x and Y temporarily
            fin >> nName >> nGridX >> nGridY >> nMove >> nVision >> nHealth >> nDamage >> nSpeed;

            // Checks type of AI
            auto npc = m_entityManager.addEntity("npc");
            if (AI == "Patrol")
            {
                fin >> N;

                for (int i = 0; i < N; i++)
                {
                    fin >> Xi >> Yi;

                    npcPos.push_back(Vec2(Xi, Yi));
                }
                npc->addComponent<CPatrol>(npcPos, nSpeed);
                npcPos = {};
            }
            else
            {
                npc->addComponent<CFollowPlayer>(Vec2(nGridX, nGridY), nSpeed);
            }

            // Gives npc all remaining components
            npc->addComponent<CAnimation>(m_game->assets().getAnimation(nName), true);
            npc->addComponent<CTransform>(Vec2(((tGridX * 64) - 32), ((tGridY * 64) - 32)));
            npc->addComponent<CBoundingBox>(m_game->assets().getAnimation(nName).getSize(), nMove, nVision);
            npc->addComponent<CHealth>(nHealth, nHealth);
            npc->addComponent<CDamage>(nDamage);
            npc->addComponent<CDraggable>(true);
        }

        // load player config
        if (label == "Player")
        {
            fin >> m_playerConfig.X >> m_playerConfig.Y >> m_playerConfig.CX >> m_playerConfig.CY >> m_playerConfig.SPEED >> m_playerConfig.HEALTH;
            m_playerConfig.SPEED *= 3;

            auto player = m_entityManager.addEntity("playerEditor");
            player->addComponent<CTransform>(Vec2(m_playerConfig.X, m_playerConfig.Y));
            player->addComponent<CAnimation>(m_game->assets().getAnimation("IdleKnife"), true);
            player->addComponent<CBoundingBox>(Vec2(m_playerConfig.CX, m_playerConfig.CY), false, false);
            player->addComponent<CHealth>(m_playerConfig.HEALTH, m_playerConfig.HEALTH);
            player->addComponent<CDraggable>();
        }
    }
    spawnPlayer();

    spawnEditorItems();

    // m_game->assets().getSound("MusicPlay").play();
}

Vec2 Scene_LevelEditor::getPosition(int rx, int ry, int tx, int ty) const
{
    return Vec2((rx * 20 + tx) * 64 + 32, (ry * 12 + ty) * 64 + 32);
}

Vec2 Scene_LevelEditor::window2World(const Vec2 &windowPos) const
{
    auto view = m_game->window().getView();

    float wx = view.getCenter().x - (m_game->window().getSize().x / 2);
    float wy = view.getCenter().y - (m_game->window().getSize().y / 2);

    return Vec2(windowPos.x + wx, windowPos.y + wy);
}

bool isInside(Vec2 pos, std::shared_ptr<Entity> e)
{
    auto ePos = e->getComponent<CTransform>().pos;
    auto size = e->getComponent<CAnimation>().animation.getSize();

    float dx = fabs(pos.x - ePos.x);
    float dy = fabs(pos.y - ePos.y);

    return ((dx <= size.x / 2) && (dy <= size.y / 2));
}

void Scene_LevelEditor::snap2Grid(std::shared_ptr<Entity> &e)
{

    // Checking if tile overlaps with another
    Vec2 place = Vec2((int(e->getComponent<CTransform>().pos.x / 64) * 64 + 32), (int(e->getComponent<CTransform>().pos.y / 64) * 64 + 32));
    for (auto &a : m_entityManager.getEntities())
    {
        if (a->id() != e->id())
        {
            Vec2 drop = Vec2((int(a->getComponent<CTransform>().pos.x / 64) * 64 + 32), (int(a->getComponent<CTransform>().pos.y / 64) * 64 + 32));
            if (drop == place)
            {
                e->getComponent<CTransform>().pos.x += 64;
                snap2Grid(e);
            }
        }
    }

    // Snaps tile to grid
    e->getComponent<CTransform>().pos.x = int(e->getComponent<CTransform>().pos.x / 64) * 64 + 32;
    e->getComponent<CTransform>().pos.y = int(e->getComponent<CTransform>().pos.y / 64) * 64 + 32;
}

void Scene_LevelEditor::sDragAndDrop()
{
    for (auto e : m_entityManager.getEntities())
    {
        if (e->hasComponent<CDraggable>() && e->getComponent<CDraggable>().dragging)
        {
            e->getComponent<CTransform>().pos = window2World(m_player->getComponent<CInput>().mousePos);
        }
    }
}

void Scene_LevelEditor::spawnEditorItems()
{
    std::string label;
    std::string tName;

    std::ifstream fin("editorItems.txt");

    int xItem = 0;
    int yItem = 0;

    while (fin >> label)
    {
        if (yItem >= 12)
        {
            yItem = 0;
            xItem += 1;
        }

        if (label == "Tile")
        {
            fin >> tName;
            auto tile = m_entityManager.addEntity("tile");
            tile->addComponent<CAnimation>(m_game->assets().getAnimation(tName), true);
            tile->addComponent<CTransform>(Vec2((xItem * 64) + 32, (yItem * 64) + 32));
            yItem += 1;
            tile->addComponent<CBoundingBox>(m_game->assets().getAnimation(tName).getSize(), true, true); // Last false, false represents bMove and bVision
            tile->addComponent<CDraggable>(true);
        }
        if (label == "NPC")
        {
            fin >> tName;
            auto npc = m_entityManager.addEntity("npc");
            npc->addComponent<CAnimation>(m_game->assets().getAnimation(tName), true);
            npc->addComponent<CTransform>(Vec2((xItem * 64) + 32, (yItem * 64) + 32));
            yItem += 1;
            npc->addComponent<CBoundingBox>(m_game->assets().getAnimation(tName).getSize(), true, true);
            npc->addComponent<CHealth>(1, 1); // HARDCODED HEALTH //FIX
            npc->addComponent<CDamage>(1);    // FIX
            npc->addComponent<CDraggable>(true);
        }
    }
}

void Scene_LevelEditor::sLevelMenu()
{
    for (auto e : m_entityManager.getEntities())
    {
        if (e->getComponent<CDraggable>().edit)
        {
            e->getComponent<CTransform>().pos.x += m_player->getComponent<CTransform>().pos.x - m_player->getComponent<CTransform>().prevPos.x;
            e->getComponent<CTransform>().pos.y += m_player->getComponent<CTransform>().pos.y - m_player->getComponent<CTransform>().prevPos.y;
        }
    }
}

void Scene_LevelEditor::spawnPlayer()
{
    m_player = m_entityManager.addEntity("player");
    m_player->addComponent<CTransform>(Vec2(m_playerConfig.X, m_playerConfig.Y));
    // m_player->addComponent<CAnimation>(m_game->assets().getAnimation("StandDown"), true);
    // m_player->addComponent<CBoundingBox>(Vec2(m_playerConfig.CX, m_playerConfig.CY), false, false);
    // m_player->addComponent<CHealth>(m_playerConfig.HEALTH, m_playerConfig.HEALTH);
    m_player->addComponent<CInput>();
}

void Scene_LevelEditor::update()
{
    if (!m_paused)
    {
        m_entityManager.update();

        // sAI();
        sMovement();
        sStatus();
        // sCollision();
        // sAnimation();
        sCamera();
        sLevelMenu();
        sDragAndDrop();

        m_currentFrame++;
    }
}

void Scene_LevelEditor::sMovement() // Used to move camera around the level editor
{

    m_player->getComponent<CTransform>().velocity = Vec2(0, 0);
    if (m_player->getComponent<CInput>().up)
    {
        m_player->getComponent<CTransform>().velocity += Vec2(0, -m_playerConfig.SPEED);
    }
    if (m_player->getComponent<CInput>().down)
    {
        m_player->getComponent<CTransform>().velocity += Vec2(0, m_playerConfig.SPEED);
    }
    if (m_player->getComponent<CInput>().left)
    {
        m_player->getComponent<CTransform>().velocity += Vec2(-m_playerConfig.SPEED, 0);
    }
    if (m_player->getComponent<CInput>().right)
    {
        m_player->getComponent<CTransform>().velocity += Vec2(m_playerConfig.SPEED, 0);
    }

    for (auto e : m_entityManager.getEntities())
    {
        e->getComponent<CTransform>().prevPos = e->getComponent<CTransform>().pos;
        e->getComponent<CTransform>().pos += e->getComponent<CTransform>().velocity;
    }

    Vec2 bounds = m_player->getComponent<CTransform>().pos - Vec2((1280 / 2) - 64, (768 / 2) - 64);
    if (bounds.x < 0)
    {
        m_player->getComponent<CTransform>().pos.x = (1280 / 2) - 64;
    }
    if (bounds.y < 0)
    {
        m_player->getComponent<CTransform>().pos.y = (768 / 2) - 64;
    }
}

void Scene_LevelEditor::sDoAction(const Action &action)
{

    if (action.type() == "START")
    {
        if (action.name() == "PAUSE")
        {
            setPaused(!m_paused);
            if (m_paused)
            {
                // m_game->assets().getSound("MusicPlay").stop();
            }
            else
            {
                // m_game->assets().getSound("MusicPlay").play();
            }
        }
        else if (action.name() == "QUIT")
        {
            onEnd();
        }
        else if (action.name() == "TOGGLE_FOLLOW")
        {
            m_follow = !m_follow;
        }
        else if (action.name() == "TOGGLE_TEXTURE")
        {
            m_drawTextures = !m_drawTextures;
        }
        else if (action.name() == "TOGGLE_COLLISION")
        {
            m_drawCollision = !m_drawCollision;
        }
        else if (action.name() == "UP")
        {
            m_player->getComponent<CInput>().up = true;
            m_player->getComponent<CTransform>().facing = Vec2(0.0, 1.0);
            m_player->getComponent<CState>().state = "run";
        }
        else if (action.name() == "DOWN")
        {
            m_player->getComponent<CInput>().down = true;
            m_player->getComponent<CTransform>().facing = Vec2(0.0, -1.0);
            m_player->getComponent<CState>().state = "run";
        }
        else if (action.name() == "LEFT")
        {
            m_player->getComponent<CInput>().left = true;
            m_player->getComponent<CTransform>().facing = Vec2(-1.0, 0.0);
            m_player->getComponent<CState>().state = "run";
        }
        else if (action.name() == "RIGHT")
        {
            m_player->getComponent<CInput>().right = true;
            m_player->getComponent<CTransform>().facing = Vec2(1.0, 0.0);
            m_player->getComponent<CState>().state = "run";
        }
        else if (action.name() == "ATTACK" && !m_player->getComponent<CInput>().attack)
        {
            m_player->getComponent<CInput>().attack = true;
            // spawnSword(m_player);
            m_player->getComponent<CState>().state = "attack";
        }
        else if (action.name() == "MOUSE_MOVE")
        {
            m_player->getComponent<CInput>().mousePos = action.pos();

            // m_mpos = ;
            Vec2 worldPos = window2World(action.pos());
            m_mouseShape.setPosition(worldPos.x, worldPos.y);
        }
        else if (action.name() == "LEFT_CLICK")
        {

            // Sorry for bad code
            // FIX format of code
            Vec2 worldPos = window2World(action.pos());
            std::shared_ptr<Entity> e = m_entityManager.addEntity("Bob");
            EntityVec tiles, npcs;

            // Separates npcs and tiles for priority of drag&drop
            bool skip = false;
            for (auto &a : m_entityManager.getEntities())
            {
                if (isInside(worldPos, a) && a->hasComponent<CDraggable>())
                {
                    if (a->getComponent<CDraggable>().dragging == true)
                    {
                        e = a;
                        skip = true;
                        break;
                    }
                    if (a->tag() == "tile")
                    {
                        tiles.push_back(a);
                    }
                    else
                    {
                        npcs.push_back(a);
                    }
                }
            }
            // Priority goes to npcs, then tiles
            if (!skip)
            {
                if (npcs.size() != 0)
                {
                    e = npcs[0];
                }
                else if (tiles.size() != 0)
                {
                    e = tiles[0];
                }
            }

            if (e->getComponent<CDraggable>().dragging)
            {
                if (e->getComponent<CDraggable>().edit)
                {
                    e->getComponent<CDraggable>().edit = false;
                }
                snap2Grid(e);
            }
            else
            {
                if (e->getComponent<CDraggable>().edit)
                {
                    std::string tag = e->tag();
                    if (tag == "tile")
                    {
                        auto tile = m_entityManager.addEntity("tile");
                        tile->addComponent<CAnimation>(e->getComponent<CAnimation>().animation, true);
                        tile->addComponent<CTransform>(e->getComponent<CTransform>().pos);
                        tile->addComponent<CBoundingBox>(e->getComponent<CBoundingBox>().size, e->getComponent<CBoundingBox>().blockMove, e->getComponent<CBoundingBox>().blockVision);
                        tile->addComponent<CDraggable>(true);
                    }
                    else if (tag == "npc")
                    {
                        auto npc = m_entityManager.addEntity("npc");
                        npc->addComponent<CAnimation>(e->getComponent<CAnimation>().animation, true);
                        npc->addComponent<CTransform>(e->getComponent<CTransform>().pos);
                        npc->addComponent<CBoundingBox>(e->getComponent<CBoundingBox>().size, e->getComponent<CBoundingBox>().blockMove, e->getComponent<CBoundingBox>().blockVision);
                        npc->addComponent<CHealth>(e->getComponent<CHealth>().max, e->getComponent<CHealth>().max);
                        npc->addComponent<CDamage>(e->getComponent<CDamage>().damage);
                        npc->addComponent<CDraggable>(true);
                    }
                }
            }

            e->getComponent<CDraggable>().dragging = !e->getComponent<CDraggable>().dragging;
        }

        else if (action.name() == "RIGHT_CLICK")
        {
            for (auto &e : m_entityManager.getEntities())
            {
                if (e->hasComponent<CSelected>())
                {
                    e->removeComponent<CSelected>();

                    m_selected.setRadius(1);
                    m_selected.setOrigin(1, 1);
                }

                if (isInside(window2World(action.pos()), e))
                {
                    e->addComponent<CSelected>();
                }
            }
        }
    }
    else if (action.type() == "END")
    {
        if (action.name() == "UP")
        {
            m_player->getComponent<CInput>().up = false;
        }
        else if (action.name() == "DOWN")
        {
            m_player->getComponent<CInput>().down = false;
        }
        else if (action.name() == "LEFT")
        {
            m_player->getComponent<CInput>().left = false;
        }
        else if (action.name() == "RIGHT")
        {
            m_player->getComponent<CInput>().right = false;
        }
    }
}

void Scene_LevelEditor::sAI()
{
    Vec2 destination;
    int speed, xRoom, yRoom;

    for (auto &e : m_entityManager.getEntities("npc"))
    {
        if (e->hasComponent<CPatrol>())
        {

            speed = e->getComponent<CPatrol>().speed;
            // Checks what room npc is in
            Vec2 rm = e->getComponent<CTransform>().pos;
            if (rm.x < 0)
            {
                xRoom = int(rm.x / 1280) - 1 * 1280;
            }
            else
            {
                xRoom = int(rm.x / 1280) * 1280;
            }

            if (rm.y < 0)
            {
                yRoom = int(rm.y / 768) - 1 * 768;
            }
            else
            {
                yRoom = int(rm.y / 768) * 768;
            }
            destination = getPosition(xRoom, yRoom, e->getComponent<CPatrol>().positions[e->getComponent<CPatrol>().currentPosition].x, e->getComponent<CPatrol>().positions[e->getComponent<CPatrol>().currentPosition].y);
        }

        // If not within 10pixels of destination, move towards it
        if (destination.dist(e->getComponent<CTransform>().pos) > 10)
        {
            Vec2 direction = destination - e->getComponent<CTransform>().pos;
            float xVel = speed * direction.x / destination.dist(e->getComponent<CTransform>().pos);
            float yVel = speed * direction.y / destination.dist(e->getComponent<CTransform>().pos);

            // go to destination
            e->getComponent<CTransform>().velocity = Vec2(xVel, yVel);
        }
        // If patrol, cycle through positions
        else if (e->hasComponent<CPatrol>())
        {
            if (e->getComponent<CPatrol>().positions.size() - 1 == e->getComponent<CPatrol>().currentPosition)
            {
                e->getComponent<CPatrol>().currentPosition = 0;
            }
            else
            {
                e->getComponent<CPatrol>().currentPosition += 1;
            }
        }
        else
        {
            e->getComponent<CTransform>().velocity = Vec2(0, 0);
        }
    }
}

void Scene_LevelEditor::sStatus()
{

    for (auto e : m_entityManager.getEntities())
    {
        if (e->hasComponent<CSelected>())
        {
            Vec2 pos = e->getComponent<CTransform>().pos;
            m_selected.setPosition(pos.x, pos.y);

            m_selected.setRadius(32);
            m_selected.setOrigin(32, 32);
        }
    }
}

void Scene_LevelEditor::sCollision()
{
    for (auto tile : m_entityManager.getEntities("tile"))
    {
        // All NPC colisions with tiles
        for (auto e : m_entityManager.getEntities("npc"))
        {
            Vec2 overlap = Physics::GetOverlap(e, tile);
            if (overlap.x > 0 && overlap.y > 0)
            {
                Vec2 prevOverlap = Physics::GetPreviousOverlap(e, tile);

                // condition for collision with heart
                if (tile->getComponent<CAnimation>().animation.getName() == "Heart")
                {
                    tile->destroy();
                    e->getComponent<CHealth>().current = e->getComponent<CHealth>().max;
                }

                if (prevOverlap.y > 0) // Case for horizontal overlap
                {
                    if (e->getComponent<CTransform>().pos.x < tile->getComponent<CTransform>().pos.x)
                    {
                        e->getComponent<CTransform>().pos.x -= overlap.x;
                    }
                    else
                    {
                        e->getComponent<CTransform>().pos.x += overlap.x;
                    }
                }
                else if (prevOverlap.x > 0) // Case for vertical overlap
                {

                    if (e->getComponent<CTransform>().pos.y < tile->getComponent<CTransform>().pos.y)
                    {
                        // If the player collides with top of tile
                        e->getComponent<CTransform>().pos.y -= overlap.y;
                    }
                    else
                    {
                        // If the player collides with bottom of tile
                        e->getComponent<CTransform>().pos.y += overlap.y;
                    }
                }
            }
        }

        // All player colisions with tiles
        Vec2 overlap2 = Physics::GetOverlap(m_player, tile);
        if (overlap2.x > 0 && overlap2.y > 0)
        {
            Vec2 prevOverlap = Physics::GetPreviousOverlap(m_player, tile);

            // condition for collision with heart
            if (tile->getComponent<CAnimation>().animation.getName() == "Heart")
            {
                tile->destroy();
                m_player->getComponent<CHealth>().current = m_player->getComponent<CHealth>().max;
                m_game->assets().getSound("GetItem").play();
            }

            // condition for collision with black tile
            if (tile->getComponent<CAnimation>().animation.getName() == "Black")
            {
                std::vector<std::shared_ptr<Entity>> blackTiles;

                for (auto &b : m_entityManager.getEntities("tile"))
                {
                    if (b->getComponent<CAnimation>().animation.getName() == "Black" && (b->id() != tile->id()))
                    {
                        blackTiles.push_back(b);
                    }
                }

                m_player->getComponent<CTransform>().pos = blackTiles[rand() % blackTiles.size()]->getComponent<CTransform>().pos + Vec2(0, 80);
            }

            if (prevOverlap.y > 0 && tile->getComponent<CBoundingBox>().blockMove) // Case for horizontal overlap
            {
                if (m_player->getComponent<CTransform>().pos.x < tile->getComponent<CTransform>().pos.x)
                {
                    m_player->getComponent<CTransform>().pos.x -= overlap2.x;
                }
                else
                {
                    m_player->getComponent<CTransform>().pos.x += overlap2.x;
                }
            }
            else if (prevOverlap.x > 0 && tile->getComponent<CBoundingBox>().blockMove) // Case for vertical overlap
            {

                if (m_player->getComponent<CTransform>().pos.y < tile->getComponent<CTransform>().pos.y)
                {
                    // If the player collides with top of tile
                    m_player->getComponent<CTransform>().pos.y -= overlap2.y;
                }
                else
                {
                    // If the player collides with bottom of tile
                    m_player->getComponent<CTransform>().pos.y += overlap2.y;
                }
            }
        }
    }

    for (auto &npc : m_entityManager.getEntities("npc"))
    {
        // player collision with npc
        Vec2 overlap = Physics::GetOverlap(m_player, npc);
        if (overlap.x > 0 && overlap.y > 0 && !m_player->hasComponent<CInvincibility>())
        {
            m_player->getComponent<CHealth>().current -= npc->getComponent<CDamage>().damage;
            if (m_player->getComponent<CHealth>().current <= 0)
            {
                m_player->getComponent<CTransform>().pos = Vec2(m_playerConfig.X, m_playerConfig.Y);
                m_player->getComponent<CHealth>().current = m_playerConfig.HEALTH;
                m_game->assets().getSound("LinkDie").play();
            }
            else
            {
                m_player->addComponent<CInvincibility>(30);
                m_game->assets().getSound("LinkHurt").play();
            }
        }

        // sword collision with npc
        for (auto &sword : m_entityManager.getEntities("sword"))
        {
            if (sword->hasComponent<CDamage>())
            {
                Vec2 overlap = Physics::GetOverlap(npc, sword);
                if (overlap.x > 0 && overlap.y > 0)
                {

                    npc->getComponent<CHealth>().current -= sword->getComponent<CDamage>().damage;
                    sword->removeComponent<CDamage>();
                    sword->removeComponent<CBoundingBox>();
                    m_game->assets().getSound("EnemyHit").play();

                    if (npc->getComponent<CHealth>().current <= 0)
                    {
                        m_game->assets().getSound("EnemyDie").play();
                        npc->destroy();
                    }
                }
            }
        }
    }
}

void Scene_LevelEditor::sAnimation()
{

    // player animation/direction
    CTransform playerTransform = m_player->getComponent<CTransform>();
    CInput playerInput = m_player->getComponent<CInput>();
    std::string curAnimation = m_player->getComponent<CAnimation>().animation.getName();

    m_player->getComponent<CAnimation>().animation = m_game->assets().getAnimation("RunRight");

    Vec2 direction = m_player->getComponent<CInput>().mousePos.abs() - m_player->getComponent<CTransform>().pos.abs();

    if (m_player->getComponent<CInput>().mousePos.y < m_player->getComponent<CTransform>().pos.y)
    {
        float mouseAngle = -atan(direction.x / direction.y) * 180 / 3.14;
        m_player->getComponent<CTransform>().angle = mouseAngle;
    }
    else
    {
        float mouseAngle = -atan(direction.x / direction.y) * 180 / 3.14;
        m_player->getComponent<CTransform>().angle = mouseAngle + 180;
    }

    // Loops through all entities updating their animation to the next frame
    for (auto e : m_entityManager.getEntities())
    {
        if (e->hasComponent<CAnimation>())
        {
            e->getComponent<CAnimation>().animation.update();

            if (!e->getComponent<CAnimation>().repeat && e->getComponent<CAnimation>().animation.hasEnded())
            {
                e->destroy();
            }
        }
    }
}

void Scene_LevelEditor::sCamera()
{

    sf::View view = sf::View(sf::Vector2f(m_player->getComponent<CTransform>().pos.x, m_player->getComponent<CTransform>().pos.y), sf::Vector2f(1280, 768));

    m_game->window().setView(view);
}

void Scene_LevelEditor::onEnd()
{
    // changes music to menu
    // m_game->assets().getSound("MusicPlay").stop();
    // m_game->assets().getSound("MusicTitle").play();
    saveLevel();

    m_hasEnded = true;
    m_game->changeScene("MENU", nullptr, true);
}

void Scene_LevelEditor::sRender()
{
    // RENDERING DONE FOR YOU

    m_game->window().clear(sf::Color(189, 44, 11));
    sf::RectangleShape tick({1.0f, 6.0f});
    tick.setFillColor(sf::Color::Black);

    // draw all Entity textures / animations
    if (m_drawTextures)
    {
        for (auto e : m_entityManager.getEntities())
        {
            auto &transform = e->getComponent<CTransform>();
            sf::Color c = sf::Color::White;
            if (e->hasComponent<CInvincibility>())
            {
                c = sf::Color(255, 255, 255, 128);
            }

            if (e->hasComponent<CAnimation>())
            {
                auto &animation = e->getComponent<CAnimation>().animation;
                animation.getSprite().setRotation(transform.angle);
                animation.getSprite().setPosition(transform.pos.x, transform.pos.y);
                animation.getSprite().setScale(transform.scale.x, transform.scale.y);
                animation.getSprite().setColor(c);
                m_game->window().draw(animation.getSprite());
            }
        }

        for (auto e : m_entityManager.getEntities())
        {
            auto &transform = e->getComponent<CTransform>();
            if (e->hasComponent<CHealth>())
            {
                auto &h = e->getComponent<CHealth>();
                Vec2 size(64, 6);
                sf::RectangleShape rect({size.x, size.y});
                rect.setPosition(transform.pos.x - 32, transform.pos.y - 48);
                rect.setFillColor(sf::Color(96, 96, 96));
                rect.setOutlineColor(sf::Color::Black);
                rect.setOutlineThickness(2);
                m_game->window().draw(rect);

                float ratio = (float)(h.current) / h.max;
                size.x *= ratio;
                rect.setSize({size.x, size.y});
                rect.setFillColor(sf::Color(255, 0, 0));
                rect.setOutlineThickness(0);
                m_game->window().draw(rect);

                for (int i = 0; i < h.max; i++)
                {
                    tick.setPosition(rect.getPosition() + sf::Vector2f(i * 64 * 1 / h.max, 0));
                    m_game->window().draw(tick);
                }
            }
        }
    }

    m_mouseShape.setFillColor(sf::Color(255, 0, 0));
    m_mouseShape.setRadius(4);
    m_mouseShape.setOrigin(2, 2);
    m_game->window().draw(m_mouseShape);

    m_selected.setFillColor(sf::Color(255, 0, 0));
    m_game->window().draw(m_selected);

    // draw all Entity collision bounding boxes with a rectangleshape
    if (m_drawCollision)
    {
        sf::CircleShape dot(4);
        dot.setFillColor(sf::Color::Black);
        for (auto e : m_entityManager.getEntities())
        {
            if (e->hasComponent<CBoundingBox>())
            {
                auto &box = e->getComponent<CBoundingBox>();
                auto &transform = e->getComponent<CTransform>();
                sf::RectangleShape rect;
                rect.setSize(sf::Vector2f(box.size.x - 1, box.size.y - 1));
                rect.setOrigin(sf::Vector2f(box.halfSize.x, box.halfSize.y));
                rect.setPosition(transform.pos.x, transform.pos.y);
                rect.setFillColor(sf::Color(0, 0, 0, 0));

                if (box.blockMove && box.blockVision)
                {
                    rect.setOutlineColor(sf::Color::Black);
                }
                if (box.blockMove && !box.blockVision)
                {
                    rect.setOutlineColor(sf::Color::Blue);
                }
                if (!box.blockMove && box.blockVision)
                {
                    rect.setOutlineColor(sf::Color::Red);
                }
                if (!box.blockMove && !box.blockVision)
                {
                    rect.setOutlineColor(sf::Color::White);
                }
                rect.setOutlineThickness(1);
                m_game->window().draw(rect);
            }

            if (e->hasComponent<CPatrol>())
            {
                auto &patrol = e->getComponent<CPatrol>().positions;
                for (size_t p = 0; p < patrol.size(); p++)
                {
                    dot.setPosition(patrol[p].x, patrol[p].y);
                    m_game->window().draw(dot);
                }
            }

            if (e->hasComponent<CFollowPlayer>())
            {
                sf::VertexArray lines(sf::LinesStrip, 2);
                lines[0].position.x = e->getComponent<CTransform>().pos.x;
                lines[0].position.y = e->getComponent<CTransform>().pos.y;
                lines[0].color = sf::Color::Black;
                lines[1].position.x = m_player->getComponent<CTransform>().pos.x;
                lines[1].position.y = m_player->getComponent<CTransform>().pos.y;
                lines[1].color = sf::Color::Black;
                m_game->window().draw(lines);
                dot.setPosition(e->getComponent<CFollowPlayer>().home.x, e->getComponent<CFollowPlayer>().home.y);
                m_game->window().draw(dot);
            }
        }
    }

    // Draw UI
}

// Saving file
void Scene_LevelEditor::saveLevel() // const std::string& filename
{
    std::ofstream fout;
    fout.open("EditorOutput.txt");

    for (auto e : m_entityManager.getEntities("tile"))
    {
        fout << "Tile "
             << e->getComponent<CAnimation>().animation.getName()
             << "   0  0 "
             << int(e->getComponent<CTransform>().pos.x / 64)
             << " "
             << int(e->getComponent<CTransform>().pos.y / 64)
             << " "
             << e->getComponent<CBoundingBox>().blockMove
             << " "
             << e->getComponent<CBoundingBox>().blockVision
             << "\n";
    }

    for (auto e : m_entityManager.getEntities("npc"))
    {
        fout << "NPC "
             << e->getComponent<CAnimation>().animation.getName()
             << "   0  0 "
             << int(e->getComponent<CTransform>().pos.x / 64)
             << " "
             << int(e->getComponent<CTransform>().pos.y / 64)
             << " "
             << e->getComponent<CBoundingBox>().blockMove
             << " "
             << e->getComponent<CBoundingBox>().blockVision
             << " "
             << e->getComponent<CHealth>().max
             << " Follow "
             << e->getComponent<CPatrol>().speed
             << "\n";
    }

    for (auto e : m_entityManager.getEntities("playerEditor"))
    {
        fout << "Player "
             << e->getComponent<CTransform>().pos.x << " " << e->getComponent<CTransform>().pos.y << " "
             << e->getComponent<CBoundingBox>().size.x << " " << e->getComponent<CBoundingBox>().size.y << " "
             << m_playerConfig.SPEED / 3
             << e->getComponent<CHealth>().max;
    }
}

// Copyright (C) David Churchill - All Rights Reserved
// COMP4300 - 2022-09 - Assignment 4
// Written by David Churchill (dave.churchill@gmail.com)
// Unauthorized copying of these files are strictly prohibited
// Distributed only for course work at Memorial University
// If you see this file online please contact email above
