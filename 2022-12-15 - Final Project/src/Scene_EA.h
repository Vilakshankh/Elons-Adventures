///\/\/\\\\////\/\/\//\\//\\\/\/\/\/\/\\\\////\/\/\//\\//\\\/\/\
//
//  Assignment       COMP4300 - Assignment 4
//  Professor:       David Churchill
//  Year / Term:     2022-09
//  File Name:       Scene_EA.h
//
//  Student Name:    Jason Lomond
//  Student User:    jblomond
//  Student Email:   jblomond@mun.ca
//  Student ID:      201900966
//  Group Member(s): [enter student name(s)]
//
///\/\/\\\\////\/\/\//\\//\\\/\/\/\/\/\\\\////\/\/\//\\//\\\/\/\

#pragma once

#include "Common.h"
#include "Scene.h"
#include <map>
#include <memory>

#include "EntityManager.h"

class Scene_EA : public Scene
{

    struct PlayerConfig
    {
        float X, Y, CX, CY, SPEED, HEALTH;
        std::string WEAPON;
    };

protected:
    std::shared_ptr<Entity> m_player;
    std::string m_levelPath;
    PlayerConfig m_playerConfig;
    sf::Shader shader;
    bool m_drawTextures = true;
    bool m_drawCollision = false;
    bool m_follow = false;

    void init(const std::string &levelPath);

    void loadLevel(const std::string &filename);

    void onEnd();
    void update();
    void spawnPlayer();
    void spawnSword(std::shared_ptr<Entity> entity);
    Vec2 getPosition(int sx, int sy, int tx, int ty) const;

    void sMovement();
    void sAI();
    void sStatus();
    void sAnimation();
    void sCollision();
    void sCamera();

public:
    Scene_EA(GameEngine *gameEngine, const std::string &levelPath);

    void sRender();
    void sDoAction(const Action &action);
};

// Copyright (C) David Churchill - All Rights Reserved
// COMP4300 - 2022-09 - Assignment 4
// Written by David Churchill (dave.churchill@gmail.com)
// Unauthorized copying of these files are strictly prohibited
// Distributed only for course work at Memorial University
// If you see this file online please contact email above
