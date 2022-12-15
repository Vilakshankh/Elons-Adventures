///\/\/\\\\////\/\/\//\\//\\\/\/\/\/\/\\\\////\/\/\//\\//\\\/\/\
//
//  Assignment       COMP4300 - Assignment 4
//  Professor:       David Churchill
//  Year / Term:     2022-09
//  File Name:       Action.cpp
// 
//  Student Name:    Jason Lomond
//  Student User:    jblomond
//  Student Email:   jblomond@mun.ca
//  Student ID:      201900966
//  Group Member(s): [enter student name(s)]
//
///\/\/\\\\////\/\/\//\\//\\\/\/\/\/\/\\\\////\/\/\//\\//\\\/\/\
                                
#include "Action.h"

Action::Action()
{

}

Action::Action(const std::string& name, const std::string& type)
    : m_name(name)
    , m_type(type)
{

}

Action::Action(const std::string & name, const std::string & type, const Vec2& pos)
    : m_name(name)
    , m_type(type)
    , m_pos(pos)
{
                                
}

const std::string & Action::name() const
{
    return m_name;
}

const std::string & Action::type() const
{
    return m_type;
}

const Vec2& Action::pos() const
{
    return m_pos;
}

std::string Action::toString() const
{
    std::stringstream ss;
    ss << name() << " " << type();
    return ss.str();
}

// Copyright (C) David Churchill - All Rights Reserved
// COMP4300 - 2022-09 - Assignment 4
// Written by David Churchill (dave.churchill@gmail.com)
// Unauthorized copying of these files are strictly prohibited
// Distributed only for course work at Memorial University
// If you see this file online please contact email above
