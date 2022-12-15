///\/\/\\\\////\/\/\//\\//\\\/\/\/\/\/\\\\////\/\/\//\\//\\\/\/\
//
//  Assignment       COMP4300 - Assignment 4
//  Professor:       David Churchill
//  Year / Term:     2022-09
//  File Name:       Scene_EA.cpp
//
//  Student Name:    Jason Lomond
//  Student User:    jblomond
//  Student Email:   jblomond@mun.ca
//  Student ID:      201900966
//  Group Member(s): [enter student name(s)]
//
///\/\/\\\\////\/\/\//\\//\\\/\/\/\/\/\\\\////\/\/\//\\//\\\/\/\

#include "Scene_EA.h"
#include "Common.h"
#include "Physics.h"
#include "Assets.h"
#include "GameEngine.h"
#include "Components.h"
#include <tgmath.h>

Scene_EA::Scene_EA(GameEngine *game, const std::string &levelPath)
    : Scene(game), m_levelPath(levelPath)
{
    init(m_levelPath);
}

void Scene_EA::init(const std::string &levelPath)
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

void Scene_EA::loadLevel(const std::string &filename)
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
            fin >> tName >> tGridX >> tGridY >> bMove >> bVision;
            auto tile = m_entityManager.addEntity("tile");

            // add tiles to entity manager list
            tile->addComponent<CAnimation>(m_game->assets().getAnimation(tName), true);
            tile->addComponent<CTransform>(Vec2(tGridX * 64, tGridY * 64));
            tile->addComponent<CBoundingBox>(m_game->assets().getAnimation(tName).getSize(), bMove, bVision);
        }

        if (label == "NPC")
        {
            fin >> nName >> nGridX >> nGridY >> nMove >> nVision >> nHealth >> nDamage >> nSpeed;

            // Checks type of AI
            auto npc = m_entityManager.addEntity("npc");
            
            npc->addComponent<CFollowPlayer>(Vec2(nGridX, nGridY), nSpeed);

            // Gives npc all remaining components
            npc->addComponent<CAnimation>(m_game->assets().getAnimation(nName), true);
            npc->addComponent<CTransform>(Vec2(nGridX * 64, nGridY * 64));
            npc->addComponent<CBoundingBox>(m_game->assets().getAnimation(nName).getSize(), nMove, nVision);
            npc->addComponent<CHealth>(nHealth, nHealth);
            npc->addComponent<CDamage>(nDamage);
            npc->addComponent<CFollowPlayer>(Vec2(nGridX * 64, nGridY * 64), nSpeed);
        }
        /*if (label == "Boss")
        {
            fin >> nName >> nGridX >> nGridY >> nMove >> nVision >> nHealth >> nDamage >> nSpeed;

            auto boss = m_entityManager.addEntity("boss");

            boss->addComponent<CAnimation>(m_game->assets().getAnimation("Boss"), true);
            boss->addComponent<CTransform>(Vec2(nGridX * 64, nGridY * 64));
            boss->addComponent<CBoundingBox>(m_game->assets().getAnimation("Boss").getSize(), true, false);
            boss->addComponent<CHealth>(nHealth, nHealth);
            boss->addComponent<CDamage>(nDamage);
            boss->addComponent<CFollowPlayer>(Vec2(nGridX * 64, nGridY * 64), nSpeed);

        }*/

        // load player config
        if (label == "Player")
        {
            fin >> m_playerConfig.X >> m_playerConfig.Y >> m_playerConfig.CX >> m_playerConfig.CY >> m_playerConfig.SPEED >> m_playerConfig.HEALTH;
        }
    }
    spawnPlayer();
    m_game->assets().getSound("MusicPlay").play();
}

Vec2 Scene_EA::getPosition(int rx, int ry, int tx, int ty) const
{
    return Vec2((rx * 20 + tx) * 64 + 32, (ry * 12 + ty) * 64 + 32);
}

void Scene_EA::spawnPlayer()
{
    m_player = m_entityManager.addEntity("player");
    m_player->addComponent<CTransform>(Vec2(m_playerConfig.X, m_playerConfig.Y));
    m_player->addComponent<CAnimation>(m_game->assets().getAnimation("Running"), true);
    m_player->addComponent<CBoundingBox>(Vec2(m_playerConfig.CX, m_playerConfig.CY), true, false);
    m_player->addComponent<CHealth>(m_playerConfig.HEALTH, m_playerConfig.HEALTH);
    m_player->addComponent<CInput>();
}

void Scene_EA::spawnMissle(Vec2 position)
{
    auto missle = m_entityManager.addEntity("missle");
    missle->addComponent<CTransform>(position);
    missle->addComponent<CAnimation>(m_game->assets().getAnimation("SwordUp"), true);
    missle->addComponent<CBoundingBox>(m_game->assets().getAnimation("SwordUp").getSize());
    missle->addComponent<CDamage>(1);
    missle->getComponent<CTransform>().velocity = Vec2(0, 10);
    missle->addComponent<CLifeSpan>(120, m_currentFrame);
}

void Scene_EA::spawnSword(std::shared_ptr<Entity> entity)
{

    m_game->assets().getSound("Slash").play();
    auto sword = m_entityManager.addEntity("sword");
    sword->addComponent<CLifeSpan>(10, m_currentFrame);
    sword->addComponent<CDamage>(entity->getComponent<CDamage>().damage);
    sword->addComponent<CBoundingBox>(m_game->assets().getAnimation("SwordRight").getSize());

    // Checks what direction to spawn the sword in
    CTransform entityTransform = entity->getComponent<CTransform>();
    if (entityTransform.facing.x == 1.0f)
    {
        sword->addComponent<CTransform>(Vec2(entityTransform.pos.x + 64, entityTransform.pos.y));
    }
    else if (entityTransform.facing.x == -1.0f)
    {
        sword->addComponent<CTransform>(Vec2(entityTransform.pos.x - 64, entityTransform.pos.y));
    }
    else if (entityTransform.facing.y == 1.0f)
    {
        sword->addComponent<CTransform>(Vec2(entityTransform.pos.x, entityTransform.pos.y - 64));
    }
    else
    {
        sword->addComponent<CTransform>(Vec2(entityTransform.pos.x, entityTransform.pos.y + 64));
    }
}

void Scene_EA::update()
{
    if (!m_paused)
    {
        m_entityManager.update();

        sAI();
        sMovement();
        sStatus();
        sCollision();
        sAnimation();
        sCamera();

        m_currentFrame++;
    }
}

void Scene_EA::sMovement()
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
    CTransform playerPos = m_player->getComponent<CTransform>();
    for (auto &e : m_entityManager.getEntities("missle"))
    {
        Vec2 direction = playerPos.pos - e->getComponent<CTransform>().pos;

        float velLength = sqrtf(playerPos.velocity.x * playerPos.velocity.x + playerPos.velocity.y * playerPos.velocity.y);
        
        Vec2 desired = Vec2((direction.x / playerPos.pos.dist(e->getComponent<CTransform>().pos)),
                            (direction.y / playerPos.pos.dist(e->getComponent<CTransform>().pos)));
        
        //how fast missles will travel
        int speed = 10;
        desired*= speed;
        Vec2 steering = desired - e->getComponent<CTransform>().velocity;

        //amount of steering, higher number stronger steering
        //0 < scale < 1
        float scale = 0.02;
        steering *= scale;
        std::cout << sqrtf(steering.x * steering.x + steering.y * steering.y) << "\n";
        
        e->getComponent<CTransform>().velocity += steering;
        
    }

    for (auto e : m_entityManager.getEntities())
    {
        e->getComponent<CTransform>().prevPos = e->getComponent<CTransform>().pos;
        e->getComponent<CTransform>().pos += e->getComponent<CTransform>().velocity;
    }
}

void Scene_EA::sDoAction(const Action &action)
{

    if (action.type() == "START")
    {
        if (action.name() == "PAUSE")
        {
            setPaused(!m_paused);
            if (m_paused)
            {
                m_game->assets().getSound("MusicPlay").stop();
            }
            else
            {
                m_game->assets().getSound("MusicPlay").play();
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
            // USED UP FOR ALL DIRECTIONS
            // FIX
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
            spawnSword(m_player);
            m_player->getComponent<CState>().state = "attack";
        }
        else if (action.name() == "MOUSE_MOVE")
        {
            m_player->getComponent<CInput>().mousePos = window2World(action.pos());
        }
        else if (action.name() == "RIGHT_CLICK")
        {
            spawnMissle(window2World(action.pos()));
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

void Scene_EA::sAI()
{
    Vec2 destination;
    int speed, xRoom, yRoom;

    for (auto &e : m_entityManager.getEntities("npc"))
    {
        
        speed = e->getComponent<CFollowPlayer>().speed;

        // Checks if npc has line of sight to player
        bool los = true;
        for (auto t : m_entityManager.getEntities("tile"))
        {
            if (t->getComponent<CBoundingBox>().blockVision && Physics::EntityIntersect(m_player->getComponent<CTransform>().pos, e->getComponent<CTransform>().pos, t))
            {
                los = false;
            }
        }

        if (los) // if enemy has line of sight to player go to player, else go home
        {
            destination = m_player->getComponent<CTransform>().pos;
        }
        else
        {
            destination = e->getComponent<CFollowPlayer>().home;
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

        if (e->tag() == "boss")
        {
            std::cout << "yuh";
            if (!e->hasComponent<CCooldown>())
            {
                spawnMissle(e->getComponent<CTransform>().pos);
                e->addComponent<CCooldown>(120);
            }
        }
    }
}

void Scene_EA::sStatus()
{

    for (auto e : m_entityManager.getEntities())
    {
        // Decrement LifeSpan, destroy entity once lifespan is over
        if (e->hasComponent<CLifeSpan>())
        {
            if ((e->getComponent<CLifeSpan>().lifespan) > 0 && e->isActive())
            {
                e->getComponent<CLifeSpan>().lifespan -= 1;
            }
            else
            {
                if (e->tag() == "sword")
                {
                    m_player->getComponent<CInput>().attack = false;
                }
                e->destroy();
            }
        }
        // Decrement Invincibility, removed after 30 frames
        if (e->hasComponent<CInvincibility>())
        {
            if (e->getComponent<CInvincibility>().iframes > 0)
            {
                e->getComponent<CInvincibility>().iframes -= 1;
            }
            else
            {
                e->removeComponent<CInvincibility>();
            }
        }

        if (e->hasComponent<CCooldown>())
        {
            if (e->getComponent<CCooldown>().length > 0)
            {
                e->getComponent<CCooldown>().length -= 1;
            }
            else
            {
                e->removeComponent<CCooldown>();
            }
        }
    }
}

void Scene_EA::sCollision()
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
            Vec2 prevOverlap2 = Physics::GetPreviousOverlap(m_player, tile);

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

            if (prevOverlap2.y > 0 && tile->getComponent<CBoundingBox>().blockMove) // Case for horizontal overlap
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
            else if (prevOverlap2.x > 0 && tile->getComponent<CBoundingBox>().blockMove) // Case for vertical overlap
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

Vec2 Scene_EA::window2World(const Vec2& windowPos) const
{
    auto view = m_game->window().getView();

    float wx = view.getCenter().x - (m_game->window().getSize().x / 2);
    float wy = view.getCenter().y - (m_game->window().getSize().y / 2);

    return Vec2(windowPos.x + wx, windowPos.y + wy);
}

void Scene_EA::sAnimation()
{

    // player animation/direction
    CTransform playerTransform = m_player->getComponent<CTransform>();
    CInput playerInput = m_player->getComponent<CInput>();
    std::string curAnimation = m_player->getComponent<CAnimation>().animation.getName();

    // Works but player needs to be flipped around
    // FIX
    bool input = m_player->getComponent<CInput>().up || m_player->getComponent<CInput>().down || m_player->getComponent<CInput>().left || m_player->getComponent<CInput>().right;

    if (m_player->getComponent<CAnimation>().animation.getName() != "Running" && input)
    {
        m_player->getComponent<CAnimation>().animation = m_game->assets().getAnimation("Running");
    }
    else if (m_player->getComponent<CAnimation>().animation.getName() != "Stand" && !input)
    {
        m_player->getComponent<CAnimation>().animation = m_game->assets().getAnimation("Stand");
    }

    Vec2 direction = m_player->getComponent<CInput>().mousePos.abs() - m_player->getComponent<CTransform>().pos.abs();

    if (m_player->getComponent<CInput>().mousePos.y < m_player->getComponent<CTransform>().pos.y)
    {
        float mouseAngle = -atan(direction.x / direction.y) * 180 / 3.14;
        m_player->getComponent<CTransform>().angle = mouseAngle +180;
    }
    else
    {
        float mouseAngle = -atan(direction.x / direction.y) * 180 / 3.14;
        m_player->getComponent<CTransform>().angle = mouseAngle;
    }

    // if (playerTransform.facing.x == 1.0f) // If the plaer is facing left
    //{
    //     m_player->getComponent<CTransform>().scale.x = 1.0;
    //     // Check input, give animation based off current input
    //     if (playerInput.attack)
    //     {
    //         if (curAnimation != "AtkRight")
    //         {
    //             m_player->getComponent<CAnimation>().animation = m_game->assets().getAnimation("AtkRight");
    //         }
    //     }
    //     else if (playerInput.right)
    //     {
    //         if (curAnimation != "RunRight")
    //         {
    //             m_player->getComponent<CAnimation>().animation = m_game->assets().getAnimation("RunRight");
    //         }
    //     }
    //     else
    //     {
    //         m_player->getComponent<CAnimation>().animation = m_game->assets().getAnimation("StandRight");
    //     }
    // }
    // else if (playerTransform.facing.x == -1.0f) // If the plaer is facing right
    //{
    //     m_player->getComponent<CTransform>().scale.x = -1.0;
    //     if (playerInput.attack)
    //     {
    //         if (curAnimation != "AtkRight")
    //         {
    //             m_player->getComponent<CAnimation>().animation = m_game->assets().getAnimation("AtkRight");
    //         }
    //     }
    //     else if (playerInput.left)
    //     {
    //         if (curAnimation != "RunRight")
    //         {
    //             m_player->getComponent<CAnimation>().animation = m_game->assets().getAnimation("RunRight");
    //         }
    //     }
    //     else
    //     {
    //         m_player->getComponent<CAnimation>().animation = m_game->assets().getAnimation("StandRight");
    //     }
    // }
    // else if (playerTransform.facing.y == 1.0f) // If the plaer is facing up
    //{

    //    if (playerInput.attack)
    //    {
    //        if (curAnimation != "AtkUp")
    //        {
    //            m_player->getComponent<CAnimation>().animation = m_game->assets().getAnimation("AtkUp");
    //        }
    //    }
    //    else if (playerInput.up)
    //    {
    //        if (curAnimation != "RunUp")
    //        {
    //            m_player->getComponent<CAnimation>().animation = m_game->assets().getAnimation("RunUp");
    //        }
    //    }
    //    else
    //    {
    //        m_player->getComponent<CAnimation>().animation = m_game->assets().getAnimation("StandUp");
    //    }
    //}
    // else // If the plaer is facing down
    //{

    //    if (playerInput.attack)
    //    {
    //        if (curAnimation != "AtkDown")
    //        {
    //            m_player->getComponent<CAnimation>().animation = m_game->assets().getAnimation("AtkDown");
    //        }
    //    }
    //    else if (playerInput.down)
    //    {
    //        if (curAnimation != "RunDown")
    //        {
    //            m_player->getComponent<CAnimation>().animation = m_game->assets().getAnimation("RunDown");
    //        }
    //    }
    //    else
    //    {
    //        m_player->getComponent<CAnimation>().animation = m_game->assets().getAnimation("StandDown");
    //    }
    //}

    //// sword animation/direction
    // for (auto sword : m_entityManager.getEntities("sword"))
    //{
    //     CTransform entityTransform = m_player->getComponent<CTransform>();
    //     // Swing sword in correct direction
    //     if (entityTransform.facing.x == 1.0f)
    //     {
    //         sword->addComponent<CAnimation>(m_game->assets().getAnimation("SwordRight"), true);
    //         sword->getComponent<CTransform>().scale.x = 1.0f;
    //         sword->getComponent<CTransform>().pos = Vec2(entityTransform.pos.x + 64, entityTransform.pos.y);
    //     }
    //     else if (entityTransform.facing.x == -1.0f)
    //     {
    //         sword->addComponent<CAnimation>(m_game->assets().getAnimation("SwordRight"), true);
    //         sword->getComponent<CTransform>().scale.x = -1.0f;
    //         sword->getComponent<CTransform>().pos = Vec2(entityTransform.pos.x - 64, entityTransform.pos.y);
    //     }
    //     else if (entityTransform.facing.y == 1.0f)
    //     {
    //         sword->addComponent<CAnimation>(m_game->assets().getAnimation("SwordUp"), true);
    //         sword->getComponent<CTransform>().scale.y = 1.0f;
    //         sword->getComponent<CTransform>().pos = Vec2(entityTransform.pos.x, entityTransform.pos.y - 64);
    //     }
    //     else
    //     {
    //         sword->addComponent<CAnimation>(m_game->assets().getAnimation("SwordUp"), true);
    //         sword->getComponent<CTransform>().scale.y = -1.0f;
    //         sword->getComponent<CTransform>().pos = Vec2(entityTransform.pos.x, entityTransform.pos.y + 64);
    //     }
    // }

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

void Scene_EA::sCamera()
{

    sf::View view = m_game->window().getView();
    sf::View newView;

    newView = sf::View(sf::Vector2f(m_player->getComponent<CTransform>().pos.x, m_player->getComponent<CTransform>().pos.y), sf::Vector2f(1280, 768));

    // if (m_follow) // Follow camera
    //{
    //     newView = sf::View(sf::Vector2f(m_player->getComponent<CTransform>().pos.x, m_player->getComponent<CTransform>().pos.y), sf::Vector2f(1280, 768));
    // }
    // else // Room camera
    //{

    //    Vec2 cPos = m_player->getComponent<CTransform>().pos;
    //    int xRoom, yRoom;

    //    // -100/1280 returns 0, so if x or y is negative -1 to calculate current room
    //    if (cPos.x < 0)
    //    {
    //        xRoom = int(cPos.x / 1280) - 1 * 1280;
    //    }
    //    else
    //    {
    //        xRoom = int(cPos.x / 1280) * 1280;
    //    }

    //    if (cPos.y < 0)
    //    {
    //        yRoom = int(cPos.y / 768) - 1 * 768;
    //    }
    //    else
    //    {
    //        yRoom = int(cPos.y / 768) * 768;
    //    }

    //    newView = sf::View(sf::FloatRect(xRoom, yRoom, 1280, 768));
    //}

    m_game->window().setView(newView);
}

void Scene_EA::onEnd()
{
    // changes music to menu
    m_game->assets().getSound("MusicPlay").stop();
    m_game->assets().getSound("MusicTitle").play();

    m_hasEnded = true;
    m_game->changeScene("MENU", nullptr, true);
}

void Scene_EA::sRender()
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
}

// Copyright (C) David Churchill - All Rights Reserved
// COMP4300 - 2022-09 - Assignment 4
// Written by David Churchill (dave.churchill@gmail.com)
// Unauthorized copying of these files are strictly prohibited
// Distributed only for course work at Memorial University
// If you see this file online please contact email above
