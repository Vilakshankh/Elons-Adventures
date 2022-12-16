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
    registerAction(sf::Keyboard::Space, "SPECIAL");
    registerAction(sf::Keyboard::Num1, "ONE");
    registerAction(sf::Keyboard::Num2, "TWO");
    registerAction(sf::Keyboard::Num3, "THREE");

    // Load the shader
    shaderFade.loadFromFile("shaders/shader_fade.frag", sf::Shader::Fragment);
    shaderShake.loadFromFile("shaders/shader_shake.frag", sf::Shader::Fragment);
    shaderRed.loadFromFile("shaders/shader_red.frag", sf::Shader::Fragment);

    // load both shaders
    if (!shaderShadow.loadFromFile("shaders/vertex_shader.vert", "shaders/fragment_shader.frag"))
    {
        std::cout << "Error loading shader" << std::endl;
    }

    background.loadFromFile("images/EA/background-large.png");
    backgroundSprite.setTexture(background);
    // backgroundSprite.setScale(2, 2);
    // // center the sprite
    backgroundSprite.setPosition(-640, -384);
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
            tile->addComponent<CShader>("shaderShadow");

            if (tName == "Metal")
            {
                metalNeeded += 1;
            }
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
            npc->addComponent<CShader>("shaderShadow");
        }

        // load player config
        if (label == "Player")
        {
            fin >> m_playerConfig.X >> m_playerConfig.Y >> m_playerConfig.CX >> m_playerConfig.CY >> m_playerConfig.SPEED >> m_playerConfig.HEALTH;
        }
    }
    spawnPlayer();
    m_game->assets().getSound("InGameMusic").play();
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
    m_player->addComponent<CShader>("default");
}

void Scene_EA::spawnMissle(Vec2 position)
{
    auto missle = m_entityManager.addEntity("missle");
    missle->addComponent<CTransform>(position);
    missle->addComponent<CAnimation>(m_game->assets().getAnimation("Barrel"), true);
    missle->addComponent<CBoundingBox>(m_game->assets().getAnimation("Barrel").getSize());
    missle->addComponent<CDamage>(1);
    missle->getComponent<CTransform>().velocity = Vec2(0, 0);
    missle->addComponent<CLifeSpan>(120, m_currentFrame);
}

void Scene_EA::spawnBullet(std::shared_ptr<Entity> entity)
{
    auto weapon = m_entityManager.addEntity("weapon");
    weapon->addComponent<CAnimation>(m_game->assets().getAnimation("Bullet"), true);

    m_game->assets().getSound("GetItem").play();
    // determins bullet direction from current direction imput

    float angle = m_player->getComponent<CTransform>().angle + 90;
    int bulletSpeed = 25;
    Vec2 velocity = Vec2(cosf((angle)*3.14 / 180) * bulletSpeed, sinf((angle)*3.14 / 180) * bulletSpeed);

    // gives the bullet its components
    weapon->addComponent<CTransform>(entity->getComponent<CTransform>().pos, velocity, Vec2(1.0f, 1.0f), angle);
    weapon->addComponent<CBoundingBox>(Vec2(15, 15));
    weapon->addComponent<CDamage>(1);
    weapon->addComponent<CLifeSpan>(60, m_currentFrame);

    // shooting is cooldown 1
    m_player->addComponent<CCooldown>(10);
}

void Scene_EA::spawnGravityBomb(std::shared_ptr<Entity> entity)
{
    // gravity is cooldown 2
    m_player->addComponent<CCooldown>(120);

    auto bomb = m_entityManager.addEntity("gravity");
    bomb->addComponent<CAnimation>(m_game->assets().getAnimation("Barrel"), true);
    bomb->addComponent<CTransform>(entity->getComponent<CTransform>().pos);
    bomb->addComponent<CLifeSpan>(120, m_currentFrame);
    bomb->addComponent<CShader>("shaderShadow");
}

void Scene_EA::spawnFlame(std::shared_ptr<Entity> entity)
{
    // ADD CODE AND CONDITION TO USE FULE
    // FIXd
    // auto weapon = m_entityManager.addEntity("weapon");
    // weapon->addComponent<CAnimation>(m_game->assets().getAnimation("Flame"), true);

    //
    //// determins bullet direction from current direction imput

    // float angle = m_player->getComponent<CTransform>().angle + 90;
    // int flameSpeed = 1;
    // Vec2 velocity = Vec2(cosf((angle) * 3.14 / 180) * flameSpeed, sinf((angle) * 3.14 / 180) * flameSpeed);

    //// gives the bullet its components
    // Vec2 placeing = (entity->getComponent<CTransform>().pos + Vec2(cosf((angle) * 3.14 / 180), (sinf((angle) * 3.14 / 180))));
    // weapon->addComponent<CTransform>(placeing, velocity, Vec2(1.0f, 1.0f), angle-90);
    // weapon->addComponent<CBoundingBox>(Vec2(32, 32));
    // weapon->addComponent<CDamage>(1);
    // weapon->addComponent<CLifeSpan>(10, m_currentFrame);
}

void Scene_EA::spawnGravity(std::shared_ptr<Entity> entity)
{
    auto gravity = m_entityManager.addEntity("pull");
    sf::CircleShape circle = sf::CircleShape(128, 30);
    circle.setFillColor(sf::Color(0, 0, 255, 20));
    circle.setPosition(entity->getComponent<CTransform>().pos.x - 128, entity->getComponent<CTransform>().pos.y - 128);
    gravity->addComponent<CGravity>(circle);
    // gravity->addComponent<CBoundingBox>();
    // gravity->addComponent<CAnimation>(m_game->assets().getAnimation("Gravity"), true);
    gravity->addComponent<CTransform>(entity->getComponent<CTransform>().pos);
    gravity->addComponent<CLifeSpan>(120, m_currentFrame);
}

void Scene_EA::spawnSword(std::shared_ptr<Entity> entity)
{

    m_game->assets().getSound("Slash").play();
    auto weapon = m_entityManager.addEntity("weapon");
    weapon->addComponent<CLifeSpan>(20, m_currentFrame);
    weapon->addComponent<CDamage>(entity->getComponent<CDamage>().damage);
    weapon->addComponent<CBoundingBox>(Vec2(63, 63));

    // Checks what direction to spawn the sword in
    CTransform entityTransform = entity->getComponent<CTransform>();
    float angle = m_player->getComponent<CTransform>().angle + 90;
    int range = 32;
    weapon->addComponent<CTransform>(Vec2(entityTransform.pos.x + (cosf((angle)*3.14 / 180) * range), entityTransform.pos.y + (sinf((angle)*3.14 / 180) * range)));
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
        sHUD();

        shaderShake.setUniform("time", m_game->time.getElapsedTime().asSeconds());
        shaderFade.setUniform("time", m_game->time.getElapsedTime().asSeconds());
        shaderRed.setUniform("time", m_game->time.getElapsedTime().asSeconds());
        shaderShadow.setUniform("hasTexture", true);
        sf::Vector2f lightPos = sf::Vector2f(m_player->getComponent<CTransform>().pos.x, m_player->getComponent<CTransform>().pos.y);
        shaderShadow.setUniform("lightPos", lightPos);

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

    // code for missle projectile steering
    CTransform playerPos = m_player->getComponent<CTransform>();
    for (auto &e : m_entityManager.getEntities("missle"))
    {
        Vec2 direction = playerPos.pos - e->getComponent<CTransform>().pos;

        float velLength = sqrtf(playerPos.velocity.x * playerPos.velocity.x + playerPos.velocity.y * playerPos.velocity.y);

        Vec2 desired = Vec2((direction.x / playerPos.pos.dist(e->getComponent<CTransform>().pos)),
                            (direction.y / playerPos.pos.dist(e->getComponent<CTransform>().pos)));

        // how fast missles will travel
        int speed = 10;
        desired *= speed;
        Vec2 steering = desired - e->getComponent<CTransform>().velocity;

        // amount of steering, higher number stronger steering
        // 0 < scale < 1
        float scale = 0.02;
        steering *= scale;

        e->getComponent<CTransform>().velocity += steering;
    }

    for (auto e : m_entityManager.getEntities("weapon"))
    {
        // Used specific bounding box size to identify melee attack
        if (e->getComponent<CBoundingBox>().size == Vec2(63, 63))
        {
            e->getComponent<CTransform>().pos += playerPos.pos - playerPos.prevPos;
        }
        else if (e->getComponent<CAnimation>().animation.getName() == "Flame")
        {
            e->getComponent<CTransform>().pos += playerPos.pos - playerPos.prevPos;
        }
    }

    for (auto grav : m_entityManager.getEntities("pull"))
    {
        for (auto e : m_entityManager.getEntities("npc"))
        {
            if (e->getComponent<CTransform>().pos.dist(grav->getComponent<CTransform>().pos) < grav->getComponent<CGravity>().circle.getRadius())
            {
                CTransform position = grav->getComponent<CTransform>();
                Vec2 direction = grav->getComponent<CTransform>().pos - e->getComponent<CTransform>().pos;
                Vec2 gravity = Vec2((direction.x / position.pos.dist(e->getComponent<CTransform>().pos)),
                                    (direction.y / position.pos.dist(e->getComponent<CTransform>().pos)));
                e->getComponent<CTransform>().velocity = gravity;
            }
        }
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
                m_game->assets().getSound("InGameMusic").stop();
            }
            else
            {
                m_game->assets().getSound("InGameMusic").play();
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
        }
        else if (action.name() == "DOWN")
        {
            m_player->getComponent<CInput>().down = true;
            m_player->getComponent<CTransform>().facing = Vec2(0.0, -1.0);
        }
        else if (action.name() == "LEFT")
        {
            m_player->getComponent<CInput>().left = true;
            m_player->getComponent<CTransform>().facing = Vec2(-1.0, 0.0);
        }
        else if (action.name() == "RIGHT")
        {
            m_player->getComponent<CInput>().right = true;
            m_player->getComponent<CTransform>().facing = Vec2(1.0, 0.0);
        }
        else if (action.name() == "SPECIAL" && !m_player->hasComponent<CCooldown>())
        {
            m_player->addComponent<CCooldown>(240);
            spawnGravityBomb(m_player);
        }
        else if (action.name() == "MOUSE_MOVE")
        {
            m_player->getComponent<CInput>().mousePos = window2World(action.pos());
        }
        else if (action.name() == "LEFT_CLICK")
        {

            if (m_player->getComponent<CState>().state == "ONE" && !m_player->getComponent<CInput>().melee)
            {
                spawnSword(m_player);
                m_player->getComponent<CInput>().melee = true;
            }
            else if (m_player->getComponent<CState>().state == "TWO")
            {
                spawnBullet(m_player);
            }
            else if (m_player->getComponent<CState>().state == "THREE")
            {
                spawnFlame(m_player);
            }
        }
        else if (action.name() == "ONE")
        {
            m_player->getComponent<CState>().state = "ONE";
        }
        else if (action.name() == "TWO")
        {
            m_player->getComponent<CState>().state = "TWO";
        }
        else if (action.name() == "THREE")
        {
            m_player->getComponent<CState>().state = "THREE";
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

        Vec2 lookDirection = m_player->getComponent<CTransform>().pos - e->getComponent<CTransform>().pos;
        if (m_player->getComponent<CTransform>().pos.y < e->getComponent<CTransform>().pos.y)
        {
            float facingAngle = -atan(lookDirection.x / lookDirection.y) * 180 / 3.14;
            e->getComponent<CTransform>().angle = facingAngle + 180;
        }
        else
        {
            float facingAngle = -atan(lookDirection.x / lookDirection.y) * 180 / 3.14;
            e->getComponent<CTransform>().angle = facingAngle;
        }

        if (e->getComponent<CAnimation>().animation.getName() == "Boss")
        {
            if (!e->hasComponent<CCooldown>())
            {
                spawnMissle(e->getComponent<CTransform>().pos);
                e->addComponent<CCooldown>(120);
            }
        }
    }
}

void Scene_EA::sHUD()
{
    // Health
    healthText.setFont(m_game->assets().getFont("Megaman"));
    healthText.setCharacterSize(16);
    healthText.setFillColor(sf::Color::White);
    healthText.setPosition(m_player->getComponent<CTransform>().pos.x - healthText.getGlobalBounds().width / 2, m_player->getComponent<CTransform>().pos.y - 350);

    healthText.setString("Health: " + std::to_string(m_player->getComponent<CHealth>().current));

    // Score
    scoreText.setFont(m_game->assets().getFont("Megaman"));
    scoreText.setCharacterSize(16);
    scoreText.setFillColor(sf::Color::White);
    scoreText.setPosition(m_player->getComponent<CTransform>().pos.x + scoreText.getGlobalBounds().width, m_player->getComponent<CTransform>().pos.y - 350);
    scoreText.setString("Score: " + std::to_string(m_player->getComponent<CScore>().currentScore));

    // Lives
    livesText.setFont(m_game->assets().getFont("Megaman"));
    livesText.setCharacterSize(16);
    livesText.setFillColor(sf::Color::White);
    livesText.setPosition(m_player->getComponent<CTransform>().pos.x - livesText.getGlobalBounds().width * 2, m_player->getComponent<CTransform>().pos.y - 350);
    livesText.setString("Lives: " + std::to_string(m_player->getComponent<CLives>().livesRemaining));

    // Level
    // levelText.setFont(m_game->assets().getFont("Megaman"));
    // levelText.setCharacterSize(16);
    // levelText.setFillColor(sf::Color::White);
    // levelText.setPosition(m_player->getComponent<CTransform>().pos.x - levelText.getGlobalBounds().width * 3, m_player->getComponent<CTransform>().pos.y - 350);
    // levelText.setString("Level: " + std::to_string(m_player->getComponent<CLevel>().level));

    weaponText.setFont(m_game->assets().getFont("Megaman"));
    weaponText.setCharacterSize(16);
    weaponText.setFillColor(sf::Color::White);
    weaponText.setPosition(m_player->getComponent<CTransform>().pos.x - weaponText.getGlobalBounds().width * 3, m_player->getComponent<CTransform>().pos.y - 350);
    weaponText.setString("Weapon: " + m_player->getComponent<CState>().state);
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
                if (e->tag() == "gravity")
                {
                    spawnGravity(e);
                }
                else
                {
                    m_player->getComponent<CInput>().melee = false;
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
        // FIX
        if (e->hasComponent<CCooldown>())
        {
            if (e->getComponent<CCooldown>().length > 0)
            {
                e->getComponent<CCooldown>().length -= 1;
            }
            else if (e->getComponent<CCooldown>().length > 0)
            {
                e->removeComponent<CCooldown>();
            }
        }
    }
}

void Scene_EA::sCollision()
{
    Vec2 boundCheck = m_player->getComponent<CTransform>().pos;
    if (boundCheck.x < 0)
    {
        m_player->getComponent<CTransform>().pos.x = 0;
    }
    else if (boundCheck.x > 2560)
    {
        m_player->getComponent<CTransform>().pos.x = 2560;
    }
    else if (boundCheck.y < 0)
    {
        m_player->getComponent<CTransform>().pos.y = 0;
    }
    else if (boundCheck.y > 1536)
    {
        m_player->getComponent<CTransform>().pos.y = 1536;
    }

    for (auto missle : m_entityManager.getEntities("missle"))
    {
        Vec2 overlap = Physics::GetOverlap(missle, m_player);
        if (overlap.x > 0 && overlap.y > 0)
        {
            m_player->getComponent<CHealth>().current -= missle->getComponent<CDamage>().damage;
            m_player->addComponent<CInvincibility>(30);
            missle->destroy();
            m_game->assets().getSound("LinkHurt").play();
            // play sound
        }
    }
    for (auto tile : m_entityManager.getEntities("tile"))
    {
        // All NPC colisions with tiles
        for (auto e : m_entityManager.getEntities("npc"))
        {
            Vec2 overlap = Physics::GetOverlap(e, tile);
            if (overlap.x > 0 && overlap.y > 0)
            {
                Vec2 prevOverlap = Physics::GetPreviousOverlap(e, tile);

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
            if (tile->getComponent<CAnimation>().animation.getName() == "Health")
            {
                tile->destroy();
                m_player->getComponent<CHealth>().current = m_player->getComponent<CHealth>().max;
                m_game->assets().getSound("GetItem").play();
            }
            if (tile->getComponent<CAnimation>().animation.getName() == "Armour")
            {
                tile->destroy();
                m_player->addComponent<CInvincibility>(60 * 6);
                m_game->assets().getSound("GetItem").play();
            }

            if (tile->getComponent<CAnimation>().animation.getName() == "DMGup")
            {
                tile->destroy();
                m_player->getComponent<CDamage>().damage += 1;
                m_game->assets().getSound("GetItem").play();
            }

            if (tile->getComponent<CAnimation>().animation.getName() == "Metal")
            {
                tile->destroy();
                metalCollected += 1;
                if (metalCollected == metalNeeded)
                {
                    // Game win condition
                    onEnd();
                }
                m_game->assets().getSound("GetItem").play();
            }

            // condition for collision with black tile
            // FIX
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
                m_player->getComponent<CLives>().livesRemaining -= 1;

                if (m_player->getComponent<CLives>().livesRemaining == 0)
                {
                    m_game->assets().getSound("GameOver").play();
                    onEnd();
                }
                else
                {
                    m_player->addComponent<CInvincibility>(30);
                    m_game->assets().getSound("LinkHurt").play();
                }
            }

            // sword collision with npc
            for (auto &weapon : m_entityManager.getEntities("weapon"))
            {
                if (weapon->hasComponent<CDamage>())
                {
                    Vec2 overlap = Physics::GetOverlap(npc, weapon);
                    if (overlap.x > 0 && overlap.y > 0)
                    {

                        npc->getComponent<CHealth>().current -= weapon->getComponent<CDamage>().damage;
                        if (weapon->getComponent<CAnimation>().animation.getName() == "Bullet")
                        {
                            weapon->destroy();
                        }
                        else
                        {
                            weapon->removeComponent<CDamage>();
                            weapon->removeComponent<CBoundingBox>();
                        }

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
}

Vec2 Scene_EA::window2World(const Vec2 &windowPos) const
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

    if (m_player->getComponent<CHealth>().current == 1)
    {
        // Change to red after
        m_player->addComponent<CShader>("shaderFade");
    }
    else
    {
        m_player->addComponent<CShader>("default");
    }

    bool input = m_player->getComponent<CInput>().up || m_player->getComponent<CInput>().down || m_player->getComponent<CInput>().left || m_player->getComponent<CInput>().right;

    if (m_player->getComponent<CState>().state == "ONE")
    {
        if (m_player->getComponent<CAnimation>().animation.getName() != "BatAttack" && m_player->getComponent<CInput>().melee)
        {
            m_player->getComponent<CAnimation>().animation = m_game->assets().getAnimation("BatAttack");
        }
        else if (m_player->getComponent<CAnimation>().animation.getName() == "BatAttack" && m_player->getComponent<CInput>().melee)
        {
            // do nothing
        }
        else if (m_player->getComponent<CAnimation>().animation.getName() != "BatWalk" && !m_player->getComponent<CInput>().melee && input)
        {
            m_player->getComponent<CAnimation>().animation = m_game->assets().getAnimation("BatWalk");
        }
        else if (m_player->getComponent<CAnimation>().animation.getName() != "BatIdle" && !m_player->getComponent<CInput>().melee && !input)
        {
            m_player->getComponent<CAnimation>().animation = m_game->assets().getAnimation("BatIdle");
        }
    }
    else if (m_player->getComponent<CState>().state == "TWO")
    {
        // if (m_player->getComponent<CAnimation>().animation.getName() != "GunAttack" && m_player->getComponent<CInput>().melee)
        //{
        //     m_player->getComponent<CAnimation>().animation = m_game->assets().getAnimation("GunAttack");
        // }
        // else if (m_player->getComponent<CAnimation>().animation.getName() == "GunAttack" && m_player->getComponent<CInput>().melee)
        //{
        //     // do nothing
        // }
        // else
        if (m_player->getComponent<CAnimation>().animation.getName() != "GunWalk" && input)
        {
            m_player->getComponent<CAnimation>().animation = m_game->assets().getAnimation("GunWalk");
        }
        else if (m_player->getComponent<CAnimation>().animation.getName() != "GunIdle" && !input)
        {
            m_player->getComponent<CAnimation>().animation = m_game->assets().getAnimation("GunIdle");
        }
    }
    else if (m_player->getComponent<CState>().state == "THREE")
    {
        // if (m_player->getComponent<CAnimation>().animation.getName() != "FlameAttack" && m_player->getComponent<CInput>().melee)
        //{
        //     m_player->getComponent<CAnimation>().animation = m_game->assets().getAnimation("FlameAttack");
        // }
        // else if (m_player->getComponent<CAnimation>().animation.getName() == "FlameAttack" && m_player->getComponent<CInput>().melee)
        //{
        //     // do nothing
        // }
        // else
        if (m_player->getComponent<CAnimation>().animation.getName() != "FlameWalk" && input)
        {
            m_player->getComponent<CAnimation>().animation = m_game->assets().getAnimation("FlameWalk");
        }
        else if (m_player->getComponent<CAnimation>().animation.getName() != "FlameIdle" && !input)
        {
            m_player->getComponent<CAnimation>().animation = m_game->assets().getAnimation("FlameIdle");
        }
    }

    Vec2 direction = m_player->getComponent<CInput>().mousePos.abs() - m_player->getComponent<CTransform>().pos.abs();

    if (m_player->getComponent<CInput>().mousePos.y < m_player->getComponent<CTransform>().pos.y)
    {
        float mouseAngle = -atan(direction.x / direction.y) * 180 / 3.14;
        m_player->getComponent<CTransform>().angle = mouseAngle + 180;
    }
    else
    {
        float mouseAngle = -atan(direction.x / direction.y) * 180 / 3.14;
        m_player->getComponent<CTransform>().angle = mouseAngle;
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

void Scene_EA::sCamera()
{

    sf::View view = m_game->window().getView();
    sf::View newView;

    newView = sf::View(sf::Vector2f(m_player->getComponent<CTransform>().pos.x, m_player->getComponent<CTransform>().pos.y), sf::Vector2f(1280, 768));

    m_game->window().setView(newView);
}

void Scene_EA::onEnd()
{
    // changes music to menu
    m_game->assets().getSound("InGameMusic").stop();
    m_game->assets().getSound("MenuMusic").play();

    m_hasEnded = true;
    m_game->changeScene("MENU", nullptr, true);
}

void Scene_EA::sRender()
{
    m_game->window().clear(sf::Color(189, 44, 11));
    sf::RectangleShape tick({1.0f, 6.0f});
    tick.setFillColor(sf::Color::Black);

    // draw background
    m_game->window().draw(backgroundSprite, &shaderShadow);

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

            if (e->hasComponent<CGravity>())
            {
                m_game->window().draw(e->getComponent<CGravity>().circle);
            }

            if (e->hasComponent<CAnimation>())
            {
                auto &animation = e->getComponent<CAnimation>().animation;
                animation.getSprite().setRotation(transform.angle);
                animation.getSprite().setPosition(transform.pos.x, transform.pos.y);
                animation.getSprite().setScale(transform.scale.x, transform.scale.y);
                animation.getSprite().setColor(c);

                // draws the elements with their respective shaders
                if (e->hasComponent<CShader>())
                {
                    if (e->getComponent<CShader>().shaderName == "shaderShadow")
                    {
                        m_game->window().draw(animation.getSprite(), &shaderShadow);
                    }
                    if (e->getComponent<CShader>().shaderName == "shaderFade")
                    {
                        m_game->window().draw(animation.getSprite(), &shaderFade);
                    }
                    if (e->getComponent<CShader>().shaderName == "shaderShake")
                    {
                        m_game->window().draw(animation.getSprite(), &shaderShake);
                    }
                    if (e->getComponent<CShader>().shaderName == "default")
                    {
                        m_game->window().draw(animation.getSprite());
                    }
                }

                else
                {
                    m_game->window().draw(animation.getSprite());
                }
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
    m_game->window().draw(healthText);
    m_game->window().draw(scoreText);
    m_game->window().draw(livesText);
    m_game->window().draw(weaponText);

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
