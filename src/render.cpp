#include "render.h"
#include <iostream>

extern Font calibri;

bool inView(layoutNode *node, int yOffset)
{
    if (node->y + yOffset + node->height < ywindow)
        return false;
    if (node->y + yOffset > WINDOW_HEIGHT - ywindow)
        return false;
    return true;
}

void renderLayoutTree(layoutNode *node, int yOffset)
{

    if (!inView(node, yOffset))
        return;

    switch (node->type)
    {
    case nodeType::text:
    {
        DrawRectangle(node->x, node->y + yOffset, node->width, node->height,
                      node->backgroundColor);
        // DrawText(node->text.c_str(), node->x, node->y + yOffset, node->fontSize,
        //          node->color);
        DrawTextEx(calibri, node->text.c_str(), {node->x, node->y + yOffset}, node->fontSize, node->fontSize*0.05, node->color); // Draw text using font and additional parameters

        break;
    }
    default:
        DrawRectangle(node->x, node->y + yOffset, node->width, node->height,
                      node->backgroundColor);
    }

    for (auto child : node->children)
    {
        renderLayoutTree(child, yOffset);
    }
}

void renderLayoutTreeDebug(layoutNode *node, int yOffset)
{

    if (!inView(node, yOffset))
        return;

    switch (node->type)
    {
    case nodeType::text:
    {
        DrawRectangleLines(node->x, node->y + yOffset, node->width,
                           node->height, PINK);
        DrawRectangle(node->x, node->y + yOffset, node->width, node->height,
                      GetColor(0xfa25f722));
        break;
    }
    case nodeType::inlineContainer:
    {
        DrawRectangleLines(node->x, node->y + yOffset, node->width,
                           node->height, GREEN);
        DrawRectangle(node->x, node->y + yOffset, node->width, node->height,
                      GetColor(0x77d47922));
        break;
    }
    case nodeType::lineContainer:
    {
        DrawRectangleLines(node->x, node->y + yOffset, node->width,
                           node->height, YELLOW);
        DrawRectangle(node->x, node->y + yOffset, node->width, node->height,
                      GetColor(0xFFEA4F22));
        break;
    }
    default:
    {
        DrawRectangleLines(node->x, node->y + yOffset, node->width,
                           node->height, RED);
        DrawRectangle(node->x, node->y + yOffset, node->width, node->height,
                      GetColor(0xff000015));
    }
    }

    for (auto child : node->children)
    {
        renderLayoutTreeDebug(child, yOffset);
    }
}

layoutNode *hitDetect(layoutNode *node, int x, int y)
{
    bool condition = (x > node->x and x < (node->x + node->width)) and
                     (y > node->y and y < (node->y + node->height));
    if (condition)
    {
        for (auto child : node->children)
        {
            layoutNode *temp = hitDetect(child, x, y);
            if (temp)
                return temp;
        }
        return node;
    }
    else
    {
        return nullptr;
    }
}

bool findUrl(std::string& url, treeNode* hit)
{
    while (hit->parentNode)
    {
        for (auto attribute : hit->nodeAttributes)
        {
            if (attribute.name == "href")
            {
                if(attribute.value[0] == '/')
                {
                    url += attribute.value;
                }
                else 
                {
                    url = attribute.value;
                }
                std::cout << url << std::endl;
                return true;
            }
        }
        hit = hit->parentNode;
    }
    return false;
}
