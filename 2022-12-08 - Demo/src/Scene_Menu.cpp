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

    m_title = "Elon's Adventure";

    m_menuStrings.push_back("Play/Continue");
    m_menuStrings.push_back("Rebind Controls");
    m_menuStrings.push_back("Game Volume");
    m_menuStrings.push_back("Level Editor");
    m_menuStrings.push_back("Reset Progress");
    m_menuStrings.push_back("Quit");

    m_levelPaths.push_back("level1.txt");
    m_levelPaths.push_back("rebind-keys");
    m_levelPaths.push_back("game-volume");
    m_levelPaths.push_back("level-editor");
    m_levelPaths.push_back("reset-progress");
    m_levelPaths.push_back("quit");

    m_menuText.setFont(m_game->assets().getFont("Megaman"));
    m_menuText.setCharacterSize(64);

    m_game->playSound("MusicTitle");
}

void Scene_Menu::update()
{
    m_entityManager.update();
}

void Scene_Menu::sDoAction(const Action &action)
{
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
                m_game->assets().getSound("MusicTitle").stop();
            }
            else if (m_levelPaths[m_selectedMenuIndex] == "rebind-keys")
            {
                std::cout << "Rebind Keys not deployed yet"
                          << "\n";
            }
            else if (m_levelPaths[m_selectedMenuIndex] == "game-volume")
            {
                std::cout << "Game Volume not deployed yet"
                          << "\n";
            }
            else if (m_levelPaths[m_selectedMenuIndex] == "level-editor")
            {
                m_game->changeScene("LEVELEDITOR", std::make_shared<Scene_LevelEditor>(m_game, m_levelPaths[0]));
                m_game->assets().getSound("MusicTitle").stop();
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

    // draw the game title in the top-left of the screen
    m_menuText.setCharacterSize(48);
    m_menuText.setString(m_title);
    m_menuText.setFillColor(sf::Color(204, 0, 0));
    m_menuText.setPosition(sf::Vector2f(200, 100));
    m_game->window().draw(m_menuText);

    // draw all of the menu options
    for (size_t i = 0; i < m_menuStrings.size(); i++)
    {
        m_menuText.setString(m_menuStrings[i]);
        m_menuText.setFillColor(i == m_selectedMenuIndex ? sf::Color::White : sf::Color(100, 100, 100));
        m_menuText.setPosition(sf::Vector2f(200, 250 + i * 72));
        m_game->window().draw(m_menuText);
    }

    // draw the controls in the bottom-left
    m_menuText.setCharacterSize(20);
    m_menuText.setFillColor(sf::Color(100, 0, 0));
    m_menuText.setString("Up: W     Down: S    Select: D      Back: ESC");
    m_menuText.setPosition(sf::Vector2f(200, 750));
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
