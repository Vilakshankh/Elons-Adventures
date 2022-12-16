///\/\/\\\\////\/\/\//\\//\\\/\/\/\/\/\\\\////\/\/\//\\//\\\/\/\
//
//  Assignment       COMP4300 - Assignment 4
//  Professor:       David Churchill
//  Year / Term:     2022-09
//  File Name:       Scene_Menu.cpp
//
//  Student Name:    Jason Lomond
//  Student User:    jblomond
//  Student Email:   jblomond@mun.ca
//  Student ID:      201900966
//  Group Member(s): [enter student name(s)]
//
///\/\/\\\\////\/\/\//\\//\\\/\/\/\/\/\\\\////\/\/\//\\//\\\/\/\

#include "Scene_Menu.h"
#include "Scene_LevelEditor.h"
#include "Scene_Map.h"
#include "Scene_Volume.h"
#include "Scene_EA.h"
#include "Common.h"
#include "Assets.h"
#include "GameEngine.h"
#include "Components.h"
#include "Action.h"
#include <tgmath.h>

Scene_Menu::Scene_Menu(GameEngine *gameEngine)
    : Scene(gameEngine)
{
    init();
}

void Scene_Menu::init()
{
    registerAction(sf::Keyboard::W, "UP");
    registerAction(sf::Keyboard::S, "DOWN");
    registerAction(sf::Keyboard::D, "SELECT");
    registerAction(sf::Keyboard::Escape, "QUIT");

    // menu items
    m_title = "Elon's Adventure";

    m_menuStrings.push_back("Play/Continue");
    m_menuStrings.push_back("Map");
    m_menuStrings.push_back("Game Volume");
    m_menuStrings.push_back("Level Editor");
    m_menuStrings.push_back("Reset Progress");
    m_menuStrings.push_back("Quit");

    m_levelPaths.push_back("level1.txt");
    m_levelPaths.push_back("map");
    m_levelPaths.push_back("game-volume");
    m_levelPaths.push_back("level-editor");
    m_levelPaths.push_back("reset-progress");
    m_levelPaths.push_back("quit");

    m_menuText.setFont(m_game->assets().getFont("Megaman"));
    m_menuText.setCharacterSize(64);

    m_game->playSound("MenuMusic");

    // planets and galaxies
    planetEarth = m_entityManager.addEntity("planetEarth");
    planetEarth->addComponent<CAnimation>(m_game->assets().getAnimation("PlanetEarth"), true);
    planetMars = m_entityManager.addEntity("planetMars");
    planetMars->addComponent<CAnimation>(m_game->assets().getAnimation("Planet1"), true);

    // parallax layers

    galaxyL5 = m_entityManager.addEntity("galaxyL5");
    galaxyL5->addComponent<CAnimation>(m_game->assets().getAnimation("GalaxyL5"), true);
    galaxyL4 = m_entityManager.addEntity("galaxyL4");
    galaxyL4->addComponent<CAnimation>(m_game->assets().getAnimation("GalaxyL4"), true);
    galaxyL3 = m_entityManager.addEntity("galaxyL3");
    galaxyL3->addComponent<CAnimation>(m_game->assets().getAnimation("GalaxyL3"), true);
    galaxyL2 = m_entityManager.addEntity("galaxyL2");
    galaxyL2->addComponent<CAnimation>(m_game->assets().getAnimation("GalaxyL2"), true);
    galaxyL1 = m_entityManager.addEntity("galaxyL1");
    galaxyL1->addComponent<CAnimation>(m_game->assets().getAnimation("GalaxyL1"), true);
    galaxyOverlay = m_entityManager.addEntity("galaxyOverlay");
    galaxyOverlay->addComponent<CAnimation>(m_game->assets().getAnimation("GalaxyOverlay"), true);

    float mousePosX = 0;
    float mousePosY = 0;
}

void Scene_Menu::update()
{
    m_entityManager.update();
}

void Scene_Menu::sDoAction(const Action &action)
{

    if (action.name() == "MOUSE_MOVE")
    {
        mousePosX = action.pos().x - 640;
        mousePosY = action.pos().y - 385;
    }

    if (action.type() == "START")
    {
        if (action.name() == "UP")
        {
            if (m_selectedMenuIndex > 0)
            {
                m_selectedMenuIndex--;
            }
            else
            {
                m_selectedMenuIndex = m_menuStrings.size() - 1;
            }
        }
        else if (action.name() == "DOWN")
        {
            m_selectedMenuIndex = (m_selectedMenuIndex + 1) % m_menuStrings.size();
        }
        else if (action.name() == "SELECT")
        {
            std::cout << "selected" << m_levelPaths[m_selectedMenuIndex] << "\n";

            if (m_levelPaths[m_selectedMenuIndex] == "level1.txt")
            {
                m_game->changeScene("SELECT", std::make_shared<Scene_EA>(m_game, m_levelPaths[m_selectedMenuIndex]));
                m_game->assets().getSound("MenuMusic").stop();
            }
            else if (m_levelPaths[m_selectedMenuIndex] == "map")
            {
                m_game->changeScene("MAP", std::make_shared<Scene_Map>(m_game));
                m_game->assets().getSound("MenuMusic").stop();
            }
            else if (m_levelPaths[m_selectedMenuIndex] == "game-volume")
            {
                m_game->changeScene("VOLUME", std::make_shared<Scene_Volume>(m_game));
                m_game->assets().getSound("MenuMusic").stop();
            }
            else if (m_levelPaths[m_selectedMenuIndex] == "level-editor")
            {
                m_game->changeScene("LEVELEDITOR", std::make_shared<Scene_LevelEditor>(m_game, m_levelPaths[0]));
                m_game->assets().getSound("MenuMusic").stop();
            }
            else if (m_levelPaths[m_selectedMenuIndex] == "reset-progress")
            {
                std::cout << "Reset Progress not deployed yet"
                          << "\n";
            }
            else if (m_levelPaths[m_selectedMenuIndex] == "quit")
            {
                onEnd();
            }
        }
        else if (action.name() == "QUIT")
        {
            onEnd();
        }
    }
}

void Scene_Menu::sRender()
{

    // clear the window to a blue
    m_game->window().setView(m_game->window().getDefaultView());
    m_game->window().clear(sf::Color(0, 0, 0));

    // draw planets and galaxy
    for (auto e : m_entityManager.getEntities())
    {
        if (e->hasComponent<CAnimation>())
        {
            auto &animation = e->getComponent<CAnimation>().animation;
            if (e->tag() == "galaxyL5")
            {
                animation.getSprite().setPosition(640 - (mousePosX / 600), 385);
                animation.getSprite().setScale(1.41, 1.41);
                animation.update();
                m_game->window().draw(animation.getSprite());
            }
            if (e->tag() == "galaxyL4")
            {
                animation.getSprite().setPosition(640 + (mousePosX / 600), 385 - (mousePosY / 900));
                animation.getSprite().setScale(1.41, 1.41);
                animation.update();
                m_game->window().draw(animation.getSprite());
            }
            if (e->tag() == "galaxyL3")
            {
                animation.getSprite().setPosition(640 - (mousePosX / 600), 385 - (mousePosY / 600));
                animation.getSprite().setScale(1.41, 1.41);
                animation.update();
                m_game->window().draw(animation.getSprite());
            }

            if (e->tag() == "galaxyL2")
            {
                animation.getSprite().setPosition(640 + (mousePosX / 350), 385 + (mousePosY / 350));
                animation.getSprite().setScale(1.41, 1.41);
                animation.update();
                m_game->window().draw(animation.getSprite());
            }
            
            if (e->tag() == "galaxyL1")
            {
                animation.getSprite().setPosition(640 - (mousePosX / 200), 385 - (mousePosY / 200));
                animation.getSprite().setScale(1.41, 1.41);
                animation.update();
                m_game->window().draw(animation.getSprite());
            }
            if (e->tag() == "galaxyOverlay")
            {
                animation.getSprite().setPosition(640, 385);
                animation.getSprite().setScale(1.41, 1.41);
                animation.update();
                m_game->window().draw(animation.getSprite());
            }
        }
    }

    for (auto e : m_entityManager.getEntities())
    {

        if (e->hasComponent<CAnimation>())
        {
            auto &animation = e->getComponent<CAnimation>().animation;
            if (e->tag() == "planetEarth")
            {
                animation.getSprite().setPosition(1000, 400);
                // animation.getSprite().setScale(1, 1);
                animation.update();
                m_game->window().draw(animation.getSprite());
            }
            if (e->tag() == "planetMars")
            {
                animation.getSprite().setPosition(800, 300);
                animation.getSprite().setScale(0.2, 0.2);
                animation.update();
                m_game->window().draw(animation.getSprite());
            }
        }
    }

    // draw the game title in the top-left of the screen
    m_menuText.setCharacterSize(48);
    m_menuText.setString(m_title);
    m_menuText.setFillColor(sf::Color(204, 0, 0));
    m_menuText.setPosition(sf::Vector2f(200, 100));
    m_game->window().draw(m_menuText);

    m_menuText.setCharacterSize(35);

    // draw all of the menu options
    for (size_t i = 0; i < m_menuStrings.size(); i++)
    {
        m_menuText.setString(m_menuStrings[i]);
        m_menuText.setFillColor(i == m_selectedMenuIndex ? sf::Color::White : sf::Color(100, 100, 100));
        m_menuText.setPosition(sf::Vector2f(200, 200 + i * 72));
        m_game->window().draw(m_menuText);
    }

    // draw the controls in the bottom-left
    m_menuText.setCharacterSize(20);
    m_menuText.setFillColor(sf::Color(100, 0, 0));
    m_menuText.setString("Up: W     Down: S    Select: D      Back: ESC");
    m_menuText.setPosition(sf::Vector2f(200, 700));
    m_game->window().draw(m_menuText);
}

void Scene_Menu::onEnd()
{
    m_hasEnded = true;
    m_game->quit();
}

// Copyright (C) David Churchill - All Rights Reserved
// COMP4300 - 2022-09 - Assignment 4
// Written by David Churchill (dave.churchill@gmail.com)
// Unauthorized copying of these files are strictly prohibited
// Distributed only for course work at Memorial University
// If you see this file online please contact email above
