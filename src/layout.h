#pragma once
#include <vector>
#include "parser.h"
#include "../raylib/include/raylib.h"
#include <utility>

enum class displayType{
    displayBlock, displayInline
};

enum class nodeType{
    text, html, image, inlineContainer, lineContainer
};

class layoutNode{
    public:
        std::vector<layoutNode*> children;
        layoutNode*              parent;
        treeNode*                originNode;
        std::string              text;

        float height;
        float width;
        float x;
        float y;

        float margin[4]  = {0};  // top, bottom, right, left
        float padding[4] = {0};  // top, bottom, right, left
      //float borders[4] = {0};  // top, bottom, right, left
      
        float fontSize;
        
        displayType display = displayType::displayBlock;
        nodeType type       = nodeType::html;
        
        Color color           = BLACK;
        Color backgroundColor = GetColor(0x00000000);
        Color borderColor     = BLACK;

        layoutNode* returnClone();
};

class layoutTree{
    public:
        void makeLayoutTree(treeNode* node, layoutNode* parentLayout);
        void traverse(layoutNode* node, int level);
        float calculateLayoutPass(layoutNode* node, float availableWidth);

        float convertStringToPx(std::string& input);
        displayType returnDisplayType(std::string& input);
        Color convertStringToColor(std::string& input);

        layoutNode* layoutTreeRoot;
        layoutNode* currentContainerNode = nullptr;

        // constants
        float emToPx = 16;
        float scale  = 3;

        float windowHeight;
        float windowWidth;
        float cursorX = 0;
        float cursorY = 0;

    private:
        // these functions are called depending on type of node encountered.
        // these functions then call the calculate layout pass function for their children
        // calculate layout pass just switchs to these functions
        float calculateLayoutBlock(layoutNode* node, float availableWidth);
        float calculateLayoutInlineContainer(layoutNode* node, float availableWidth);
        float calculateLayoutLineContainer(layoutNode* node, float availableWidth);
        float calculateLayoutText(layoutNode* node, float availableWidth);

        // these functions are used to calculate layout inside inline containers
        void  seperateLineText(layoutNode*  node, layoutNode* child, float availableWidth, std::vector<layoutNode*>& lineContainers);
        std::pair<float, float> getTextNodeHeight(layoutNode* node);
};
