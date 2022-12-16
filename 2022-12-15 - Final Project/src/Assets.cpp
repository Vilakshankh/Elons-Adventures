///\/\/\\\\////\/\/\//\\//\\\/\/\/\/\/\\\\////\/\/\//\\//\\\/\/\
//
//  Assignment       COMP4300 - Assignment 4
//  Professor:       David Churchill
//  Year / Term:     2022-09
//  File Name:       Assets.cpp
//
//  Student Name:    Jason Lomond
//  Student User:    jblomond
//  Student Email:   jblomond@mun.ca
//  Student ID:      201900966
//  Group Member(s): [enter student name(s)]
//
///\/\/\\\\////\/\/\//\\//\\\/\/\/\/\/\\\\////\/\/\//\\//\\\/\/\

#include "Assets.h"
#include <cassert>

Assets::Assets()
{
}

void Assets::loadFromFile(const std::string &path)
{
    std::ifstream file(path);
    std::string str;
    while (file.good())
    {
        file >> str;

        if (str == "Texture")
        {
            std::string name, path;
            file >> name >> path;
            addTexture(name, path);
        }
        else if (str == "Animation")
        {
            std::string name, texture;
            size_t frames, speed;
            file >> name >> texture >> frames >> speed;
            addAnimation(name, texture, frames, speed);
        }
        else if (str == "Sound")
        {
            std::string name, path;
            file >> name >> path;
            addSound(name, path);
        }
        else if (str == "Font")
        {
            std::string name, path;
            file >> name >> path;
            addFont(name, path);
        }
        else
        {
            std::cerr << "Unknown Asset Type: " << str << std::endl;
        }
    }
}

void Assets::addTexture(const std::string &textureName, const std::string &path, bool smooth)
{
    m_textureMap[textureName] = sf::Texture();

    if (!m_textureMap[textureName].loadFromFile(path))
    {
        std::cerr << "Could not load texture file: " << path << std::endl;
        m_textureMap.erase(textureName);
    }
    else
    {
        m_textureMap[textureName].setSmooth(smooth);
        std::cout << "Loaded Texture: " << path << std::endl;
    }
}

const sf::Texture &Assets::getTexture(const std::string &textureName) const
{
    //assert(m_textureMap.find(textureName) != m_textureMap.end());
    if (m_textureMap.find(textureName) != m_textureMap.end()) 
    {
        return m_textureMap.at(textureName);
    }
    else
    {
        return m_textureMap.at("TexGalaxyL1");
        std::cout << "returned texGalaxy" << "\n";
    }

    
    
}

void Assets::addAnimation(const std::string &animationName, const std::string &textureName, size_t frameCount, size_t speed)
{
    m_animationMap[animationName] = Animation(animationName, getTexture(textureName), frameCount, speed);
}

const Animation &Assets::getAnimation(const std::string &animationName) const
{
    assert(m_animationMap.find(animationName) != m_animationMap.end());
    return m_animationMap.at(animationName);
}

void Assets::addFont(const std::string &fontName, const std::string &path)
{
    m_fontMap[fontName] = sf::Font();
    if (!m_fontMap[fontName].loadFromFile(path))
    {
        std::cerr << "Could not load font file: " << path << std::endl;
        m_fontMap.erase(fontName);
    }
    else
    {
        std::cout << "Loaded Font:    " << path << std::endl;
    }
}

const sf::Font &Assets::getFont(const std::string &fontName) const
{
    assert(m_fontMap.find(fontName) != m_fontMap.end());
    return m_fontMap.at(fontName);
}

void Assets::addSound(const std::string &soundName, const std::string &path)
{
    m_soundBufferMap[soundName] = sf::SoundBuffer();
    if (!m_soundBufferMap[soundName].loadFromFile(path))
    {
        std::cerr << "Could not load sound file: " << path << std::endl;
        m_soundBufferMap.erase(soundName);
    }
    else
    {
        std::cout << "Loaded Sound:    " << path << std::endl;
        m_soundMap[soundName] = sf::Sound(m_soundBufferMap[soundName]);
        m_soundMap[soundName].setVolume(25);
    }
}

sf::Sound &Assets::getSound(const std::string &soundName)
{
    assert(m_soundMap.find(soundName) != m_soundMap.end());
    return m_soundMap.at(soundName);
}

// Copyright (C) David Churchill - All Rights Reserved
// COMP4300 - 2022-09 - Assignment 4
// Written by David Churchill (dave.churchill@gmail.com)
// Unauthorized copying of these files are strictly prohibited
// Distributed only for course work at Memorial University
// If you see this file online please contact email above
