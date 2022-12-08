///\/\/\\\\////\/\/\//\\//\\\/\/\/\/\/\\\\////\/\/\//\\//\\\/\/\
//
//  Assignment       COMP4300 - Assignment 4
//  Professor:       David Churchill
//  Year / Term:     2022-09
//  File Name:       Physics.cpp
//
//  Student Name:    Jason Lomond
//  Student User:    jblomond
//  Student Email:   jblomond@mun.ca
//  Student ID:      201900966
//  Group Member(s): [enter student name(s)]
//
///\/\/\\\\////\/\/\//\\//\\\/\/\/\/\/\\\\////\/\/\//\\//\\\/\/\

#include "Physics.h"
#include "Components.h"

Vec2 Physics::GetOverlap(std::shared_ptr<Entity> a, std::shared_ptr<Entity> b)
{
    Vec2 delta = (a->getComponent<CTransform>().pos - b->getComponent<CTransform>().pos).abs();

    float overlapX = (a->getComponent<CBoundingBox>().halfSize.x) + (b->getComponent<CBoundingBox>().halfSize.x) - delta.x;
    float overlapY = (a->getComponent<CBoundingBox>().halfSize.y) + (b->getComponent<CBoundingBox>().halfSize.y) - delta.y;

    return Vec2(overlapX, overlapY);

    return Vec2(0, 0);
}

Vec2 Physics::GetPreviousOverlap(std::shared_ptr<Entity> a, std::shared_ptr<Entity> b)
{
    Vec2 delta = (a->getComponent<CTransform>().prevPos - b->getComponent<CTransform>().prevPos).abs();

    float prevOverlapX = (a->getComponent<CBoundingBox>().halfSize.x) + (b->getComponent<CBoundingBox>().halfSize.x) - delta.x;
    float prevOverlapY = (a->getComponent<CBoundingBox>().halfSize.y) + (b->getComponent<CBoundingBox>().halfSize.y) - delta.y;

    return Vec2(prevOverlapX, prevOverlapY);

    return Vec2(0, 0);
}

bool Physics::IsInside(const Vec2 &pos, std::shared_ptr<Entity> e)
{
    // STUDENT TODO:
    // Implement this function

    return false;
}

Intersect Physics::LineIntersect(const Vec2 &a, const Vec2 &b, const Vec2 &c, const Vec2 &d)
{
    
    Vec2 r = b - a;
    Vec2 s = d - c;
    float rxs = r * s;
    Vec2 cma = c - a;

    //Checks to avoid division by zero
    if (rxs != 0)
    {
        //get point of line intersection
        float t = (cma * s) / rxs;
        float u = (cma * r) / rxs;
        if (t >= 0 && t <= 1 && u >= 0 && u <= 1)
        {
            return {true, Vec2(a.x + t * r.x, a.y + t * r.y)};
        }
        else
        {
            return {false, Vec2(0, 0)};
        }
    }
    else
    {
        return { false, Vec2(0, 0) };
    }
}

bool Physics::EntityIntersect(const Vec2 &a, const Vec2 &b, std::shared_ptr<Entity> e)
{
    //Get position of entity corners
    Vec2 ePos = e->getComponent<CTransform>().pos;
    Vec2 e1 = ePos + Vec2(-32, -32);
    Vec2 e2 = ePos + Vec2(32, -32);
    Vec2 e3 = ePos + Vec2(32, 32);
    Vec2 e4 = ePos + Vec2(-32, 32);

    //Check all line segments of entity
    if (LineIntersect(a, b, e1, e2).result || LineIntersect(a, b, e2, e3).result || LineIntersect(a, b, e3, e4).result || LineIntersect(a, b, e4, e1).result)
    {
        return true;
    } 
    else
    {
        return false;
    }
}

// Copyright (C) David Churchill - All Rights Reserved
// COMP4300 - 2022-09 - Assignment 4
// Written by David Churchill (dave.churchill@gmail.com)
// Unauthorized copying of these files are strictly prohibited
// Distributed only for course work at Memorial University
// If you see this file online please contact email above
