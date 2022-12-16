///\/\/\\\\////\/\/\//\\//\\\/\/\/\/\/\\\\////\/\/\//\\//\\\/\/\
//
//  Assignment       COMP4300 - Assignment 4
//  Professor:       David Churchill
//  Year / Term:     2022-09
//  File Name:       Scene_Map.cpp
//
//  Student Name:    Jason Lomond
//  Student User:    jblomond
//  Student Email:   jblomond@mun.ca
//  Student ID:      201900966
//  Group Member(s): [enter student name(s)]
//
///\/\/\\\\////\/\/\//\\//\\\/\/\/\/\/\\\\////\/\/\//\\//\\\/\/\

#include "Scene_Map.h"
#include "Scene_LevelEditor.h"
#include "Scene_EA.h"
#include "Common.h"
#include "Assets.h"
#include "GameEngine.h"
#include "Components.h"
#include "Action.h"
#include <tgmath.h>

Scene_Map::Scene_Map(GameEngine *gameEngine)
    : Scene(gameEngine)
{
    init();
}



void Scene_Map::init()
{
    registerAction(sf::Keyboard::W, "UP");
    registerAction(sf::Keyboard::S, "DOWN");
    registerAction(sf::Keyboard::D, "SELECT");
    registerAction(sf::Keyboard::Escape, "QUIT");

    m_title = "Map";

    m_menuStrings.push_back("Level 1");
    m_menuStrings.push_back("Level 2");
    m_menuStrings.push_back("Level 3: Boss Fight");
    

    m_levelPaths.push_back("level1.txt");
    m_levelPaths.push_back("level2.txt");
    m_levelPaths.push_back("level3.txt");
    

    m_menuText.setFont(m_game->assets().getFont("Megaman"));
    m_menuText.setCharacterSize(64);

    m_game->playSound("MusicTitle");

    planetMars = m_entityManager.addEntity("planetMars");
    planetMars->addComponent<CAnimation>(m_game->assets().getAnimation("Planet1"), true);
    galaxy = m_entityManager.addEntity("galaxy");
    galaxy->addComponent<CAnimation>(m_game->assets().getAnimation("Galaxy"), true);
}





void Scene_Map::update()
{
    m_entityManager.update();
}





void Scene_Map::sDoAction(const Action &action)
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

            if (m_selectedMenuIndex == 0)
            {
                planetMars->addComponent<CAnimation>(m_game->assets().getAnimation("Planet1"), true);
                //std::cout << m_selectedMenuIndex;
            }
            else if (m_selectedMenuIndex == 1)
            {

                planetMars->addComponent<CAnimation>(m_game->assets().getAnimation("Planet2"), true);
            }
            else if (m_selectedMenuIndex == 2)
            {
                planetMars->addComponent<CAnimation>(m_game->assets().getAnimation("Planet3"), true);
            }
        }
        else if (action.name() == "DOWN")
        {
            m_selectedMenuIndex = (m_selectedMenuIndex + 1) % m_menuStrings.size();
            //std::cout << m_selectedMenuIndex;
            if (m_selectedMenuIndex == 0)
            {
                planetMars->addComponent<CAnimation>(m_game->assets().getAnimation("Planet1"), true);
                //std::cout << m_selectedMenuIndex;
            }
            else if (m_selectedMenuIndex == 1)
            {

                planetMars->addComponent<CAnimation>(m_game->assets().getAnimation("Planet2"), true);
            }
            else if (m_selectedMenuIndex == 2)
            {
                planetMars->addComponent<CAnimation>(m_game->assets().getAnimation("Planet3"), true);
            }
            
        }
        else if (action.name() == "SELECT")
        {
            std::cout << "selected" << m_levelPaths[m_selectedMenuIndex] << "\n";

            if (m_levelPaths[m_selectedMenuIndex] == "level1.txt")
            {
                m_game->changeScene("SELECT", std::make_shared<Scene_EA>(m_game, m_levelPaths[m_selectedMenuIndex]));
                m_game->assets().getSound("MusicTitle").stop();
            }
            if (m_levelPaths[m_selectedMenuIndex] == "level2.txt")
            {
                m_game->changeScene("SELECT", std::make_shared<Scene_EA>(m_game, m_levelPaths[m_selectedMenuIndex]));
                m_game->assets().getSound("MusicTitle").stop();
            }
            if (m_levelPaths[m_selectedMenuIndex] == "level3.txt")
            {
                m_game->changeScene("SELECT", std::make_shared<Scene_EA>(m_game, m_levelPaths[m_selectedMenuIndex]));
                m_game->assets().getSound("MusicTitle").stop();
            }
            else if (m_levelPaths[m_selectedMenuIndex] == "back")
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





void Scene_Map::sRender()
{

    // clear the window to a blue
    m_game->window().setView(m_game->window().getDefaultView());
    m_game->window().clear(sf::Color(0, 0, 0));


    //draw galaxy
    for (auto e : m_entityManager.getEntities())
    {
        if (e->hasComponent<CAnimation>())
        {
            auto& animation = e->getComponent<CAnimation>().animation;
            if (e->tag() == "galaxy")
            {
                animation.getSprite().setPosition(800, 300);
                animation.getSprite().setScale(1.7, 1.7);
                animation.update();
                m_game->window().draw(animation.getSprite());
            }
        }
    }



    //draw the planetMars
    for (auto e : m_entityManager.getEntities())
    {
        if (e->hasComponent<CAnimation>())
        {

            auto& animation = e->getComponent<CAnimation>().animation;
            if (e->tag() == "planetMars")
            {
                animation.getSprite().setPosition(1000, 400);
                //animation.getSprite().setScale(1.5, 1.5);
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

void Scene_Map::onEnd()
{
    m_hasEnded = true;
    m_game->changeScene("MENU", nullptr, true);
}

// Copyright (C) David Churchill - All Rights Reserved
// COMP4300 - 2022-09 - Assignment 4
// Written by David Churchill (dave.churchill@gmail.com)
// Unauthorized copying of these files are strictly prohibited
// Distributed only for course work at Memorial University
// If you see this file online please contact email above
