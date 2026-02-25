#include "layout.h"
#include <iostream>
#include <string>
#include <unordered_map>

std::unordered_map<std::string, Color> stringToColorMap = {

        {"WHITE",        GetColor(0xFFFFFFFF)},
        {"SILVER",       GetColor(0xC0C0C0FF)},
        {"GRAY",         GetColor(0x808080FF)},
        {"BLACK",        GetColor(0x000000FF)},
        
        //               Reds    & Pinks
        {"RED",          GetColor(0xFF0000FF)},
        {"CRIMSON",      GetColor(0xDC143CFF)},
        {"PINK",         GetColor(0xFFC0CBFF)},
        {"HOTPINK",      GetColor(0xFF69B4FF)},
        {"MAGENTA",      GetColor(0xFF00FFFF)},

        //               Oranges & Yellows
        {"ORANGE",       GetColor(0xFFA500FF)},
        {"CORAL",        GetColor(0xFF7F50FF)},
        {"GOLD",         GetColor(0xFFD700FF)},
        {"YELLOW",       GetColor(0xFFFF00FF)},
        {"BEIGE",        GetColor(0xF5F5DCFF)},

        //               Greens
        {"GREEN",        GetColor(0x008000FF)},
        {"LIME",         GetColor(0x00FF00FF)},
        {"OLIVE",        GetColor(0x808000FF)},
        {"DARKGREEN",    GetColor(0x006400FF)},
        {"TEAL",         GetColor(0x008080FF)},

        //               Blues
        {"AQUA",         GetColor(0x00FFFFFF)},
        {"SKYBLUE",      GetColor(0x87CEEBFF)},
        {"BLUE",         GetColor(0x0000FFFF)},
        {"NAVY",         GetColor(0x000080FF)},
        {"ROYALBLUE",    GetColor(0x4169E1FF)},
        {"MIDNIGHTBLUE", GetColor(0x191970FF)},

        //               Purples & Browns
        {"PURPLE",       GetColor(0x800080FF)},
        {"VIOLET",       GetColor(0xEE82EEFF)},
        {"INDIGO",       GetColor(0x4B0082FF)},
        {"BROWN",        GetColor(0xA52A2AFF)},
        {"MAROON",       GetColor(0x800000FF)},

        {"TRANSPARENT",  GetColor(0x00000000)}
};

layoutNode* layoutNode::returnClone(){
    layoutNode* value = new layoutNode();
    
    value->height           = height;
    value->width            = width;
    value->x                = x;
    value->y                = y;

    for(int i = 0; i<4; i++){
        value->margin[i] = margin[i];
        value->padding[i] = padding[i];
    }

    value->fontSize         = fontSize;

    value->display          = display;
    value->type             = type;

    value->color            = color;
    value->backgroundColor  = backgroundColor;

    return value;
}

// body and root node
void layoutTree::makeLayoutTree(treeNode* node, layoutNode* parentLayout){

    // create a layout node
    layoutNode* currentLayoutNode = new layoutNode;

    // add its parent layout node
    currentLayoutNode->parent = parentLayout;

    // link it to its origin dom node
    currentLayoutNode->originNode = node;

    /* filling data in layout nodes */

    // lambda function for filling margin and padding data
    auto fillArray = [&](float* array, const std::string& identifier){
        std::vector<std::string> suffix = {"-top", "-bottom", "-right", "-left"};
        for(int i=0; i<4; i++) array[i] = convertStringToPx(node->style[node->cssPropertyIndexCache[identifier+(suffix[i])]].value)*scale;
    };

    // filling margin and padding
    fillArray(currentLayoutNode->margin, "margin");
    fillArray(currentLayoutNode->padding, "padding");

    // filling the display type 
    currentLayoutNode->display = returnDisplayType(node->style[node->cssPropertyIndexCache["display"]].value);

    // filling the node type 
    if(node->type == text){
        currentLayoutNode->type = nodeType::text;
        for(auto a : node->nodeAttributes){
            if(a.name == "text"){
                currentLayoutNode->text = a.value;
                break;
            }
        }
    }else if(node->type == html){
        currentLayoutNode->type = nodeType::html;
    }

    // filling font size
    currentLayoutNode->fontSize = convertStringToPx(node->style[node->cssPropertyIndexCache["font-size"]].value)*scale;
    std::cout << "font size of " << node->name << " is " << currentLayoutNode->fontSize;

    // filling the background color 
    currentLayoutNode->backgroundColor = convertStringToColor(node->style[node->cssPropertyIndexCache["background-color"]].value);

    // filling the background color 
    currentLayoutNode->color = convertStringToColor(node->style[node->cssPropertyIndexCache["color"]].value);
    std::cout << "filling the color of" << node->name << std::endl;

    // store the current container node and make it null so that children are forced to make their own container nodes
    layoutNode* temp = currentContainerNode;
    currentContainerNode = nullptr;

    // now we have to recurse and add layout nodes for all children of the treenode which will be linked to current layout node
    for(auto child : node->children){
        makeLayoutTree(child, currentLayoutNode);
    }

    // set the current container node to temp
    // this way children do not get added to the wrong container node and tree structure for inline elements is preserved
    currentContainerNode = temp;

    // add it to children of parent layout node if block
    // or add to the last container node if inline. if last node doesnt exist make a new one
    if(currentLayoutNode->display == displayType::displayBlock){
        parentLayout->children.push_back(currentLayoutNode);
        currentContainerNode = nullptr;
    }else if(currentLayoutNode->display == displayType::displayInline){
        if(currentContainerNode != nullptr){
            currentContainerNode->children.push_back(currentLayoutNode);
        }else{
            currentContainerNode = new layoutNode();
            parentLayout->children.push_back(currentContainerNode);
            currentContainerNode->type = nodeType::inlineContainer;
            currentContainerNode->children.push_back(currentLayoutNode);
        }
    }



}

void layoutTree::traverse(layoutNode* node, int level){
    std::string indent;
    for(int i=0; i<level; i++){
        indent += "   ";
    }
    if(node->originNode){ std::cout << indent << node->originNode->name << " ";}
    else if(node->type == nodeType::inlineContainer){ std::cout << indent << "container node ";}
    else if(node->type == nodeType::lineContainer){ std::cout << indent << "line ";}
    std::cout << (int)node->color.r << (int)node->color.g << (int)node->color.b << (int)node->color.a ;
    std::cout << " font size " << node->fontSize;
    // std::cout << " " << " height:" << node->height;
    // std::cout << " " << "  width:" << node->width;
    // std::cout << " " << "      x:" << node->x;
    // std::cout << " " << "      y:" << node->y;
    std::cout << " " << node->text;

    std::cout << "\n";
    for(auto child : node->children){
        if(child) traverse(child, level+1);
    }
}

float layoutTree::convertStringToPx(std::string& input){
    std::string data;
    std::string type;

    int state = 0;
    for(char c : input){
        if(state == 0){
            if( c>47 && c<58 || c==46){
                data += c;
            }else if(c == ' '){
                // do nothing
            }else{
                state = 1;
                type += c;
            }
        }else if(state == 1){
            if(c == ' ') break;
            type += c;
        }
    }

    float value;

    try{
        value = stof(data);
    }catch(std::invalid_argument){
        std::cout << "Invalid CSS in \"" << input << "\"" << std::endl;
        value = 16;
    }

    float multiplier = 1;

    if(type == "em") multiplier = emToPx;
    if(type == "px") multiplier = 1;

    return value*multiplier;
}

displayType layoutTree::returnDisplayType(std::string& input){
    std::string temp;
    int state = 0;
    for(char c : input){
        if(c>65 && c<91) {
            temp += (c + 32);
            state = 1;
        }else if(c>96 && c<123){
            temp += c;
            state = 1;
        }else{
            if(state == 1) break;
        }
    }

    displayType value;

    if(temp == "block"){
        value = displayType::displayBlock;
    }else if(temp == "inline"){
        value = displayType::displayInline;
    }else{
        std::cout << "Invalid CSS in \"" << input << "\"" << std::endl;
        value = displayType::displayBlock;
    }
    return value;

}

// top, bottom, right, left;

float layoutTree::calculateLayoutPass(layoutNode* node, float availableWidth){
    float nodeWidth;
    float nodeHeight = 0;

    switch(node->type) {
        case nodeType::html: {
            return calculateLayoutBlock(node, availableWidth);
        }

        case nodeType::inlineContainer: {
            return calculateLayoutInlineContainer(node, availableWidth);
        }

        case nodeType::lineContainer: {
            return calculateLayoutLineContainer(node, availableWidth);
        }

        case nodeType::text: {
            return calculateLayoutText(node, availableWidth);
        }

        case nodeType::image: {
            // std::cout << "image node detected" << std::endl;
            return nodeHeight;
        }

    }
}

float layoutTree::calculateLayoutBlock(layoutNode* node, float availableWidth){
    float nodeWidth;
    float nodeHeight = 0;

    // node width is available width minus left and right margin
    nodeWidth = availableWidth - (node->margin[2] + node->margin[3]);
    
    node->x = cursorX + node->margin[3];
    node->y = cursorY + node->margin[0];

    cursorX = node->x + node->padding[3];
    cursorY = node->y + node->padding[0];

    float newAvailableWidth = nodeWidth - (node->padding[2] + node->padding[3]);

    for(auto child : node->children){
        nodeHeight += calculateLayoutPass(child, newAvailableWidth);
    }

    nodeHeight += (node->padding[0] + node->padding[1]);

    cursorX = node->x - node->margin[3];
    cursorY = node->y + nodeHeight + node->margin[1];

    node->height = nodeHeight;
    node->width  = nodeWidth;

    return nodeHeight + (node->margin[0] + node->margin[1]);
}

float layoutTree::calculateLayoutInlineContainer(layoutNode* node, float availableWidth){

    // work on all the children elements and recurse only for elements that may have children
    // text cannot have children so divide it up into more text nodes and put them in line containers
    // inline container node will have no margin or padding because that will be handled by its parent node
    
    // make sure to store the width of these line containers

    float nodeHeight = 0;
    std::vector<layoutNode*> lineContainers;

    node->x = cursorX;
    node->y = cursorY;

    for(auto child : node->children){

        switch(child->type){

            case nodeType::text: {
                seperateLineText(node, child, availableWidth, lineContainers);
                break;
            }

            default: /* std::cout << "Default node detected in container node" << std::endl*/ ; 

        }

    }

    node->children = lineContainers;

    // calculate layout for all the created lines
    for(auto line : node->children){
        nodeHeight += calculateLayoutPass(line, availableWidth);
    }

    node->height = nodeHeight;
    node->width  = availableWidth;

    cursorY += nodeHeight;

    return nodeHeight;

}

float layoutTree::calculateLayoutLineContainer(layoutNode* node, float availableWidth){
    float nodeHeight = 0;
    float descender = 0;
    node->x = cursorX;
    node->y = cursorY;

    // get the largest element of all
    for(auto child : node->children){
        float tempAscender = 0;
        float tempDescender = 0;

        switch(child->type){
            case nodeType::text:{
                auto temp = getTextNodeHeight(child);
                tempAscender  = temp.first;
                tempDescender = temp.second;
                break;
            }
            default:;
        }

        if(tempAscender  > nodeHeight) nodeHeight = tempAscender;
        if(tempDescender > nodeHeight) descender  = tempDescender;
    }
    
    for(auto child : node->children){
        calculateLayoutPass(child, availableWidth);
    }

    cursorY += nodeHeight;
    node->height = nodeHeight;
    return nodeHeight;
}

float layoutTree::calculateLayoutText(layoutNode* node, float availableWidth){
    float nodeHeight = 0;

    // get height of the line container and calculate based on it
    node->x = cursorX;
    node->y = cursorY;

    return nodeHeight;
}

void layoutTree::seperateLineText(layoutNode* node, layoutNode* child, float availableWidth, std::vector<layoutNode*>& lineContainers){


    std::cout << "Text node detected in container node" << std::endl;

    // use this to check if somethings can be fit in the previous line
    // after checking it set the bool to false and never check again for this child
    bool checkLastLine = false;

    std::string tempString;
    std::string word;

    // lambda to create a new line container
    auto createLineContainer = [&](){
        layoutNode* temp = new layoutNode();
        temp->parent = node;
        lineContainers.push_back(temp);
        temp->type = nodeType::lineContainer;

        layoutNode* tempchild = node->returnClone();
        tempchild->type = nodeType::text;
        tempchild->originNode = child->originNode;
        tempchild->parent = temp;
        tempchild->text = tempString;
        tempchild->width = MeasureText(tempString.c_str(), child->fontSize);
        tempchild->fontSize = child->fontSize;
        // make sure to copy attributes of the parent text node

        temp->width = tempchild->width;
        temp->children.push_back(tempchild);
    };

    for(auto c : child->text + " "){
        if(c == ' '){
            // do the checks and if we hit the max word length make a new line container
            word += c;
            
            float width = MeasureText((tempString+word).c_str(), child->fontSize);
            if(width >= availableWidth){

                if(!checkLastLine){
                    createLineContainer();
                }else{
                    // check if you can fit things in the last line container
                }

                tempString.clear();
            }

            tempString += word;
            word.clear();
        }else{
            word += c;
        }
    }

    // add the last line as a line container too
    createLineContainer();

    delete child;

}

Color layoutTree::convertStringToColor(std::string& input){
    std::string temp;
    int state = 0;
    for(char c : input){
        if(c>64 && c<91) {
            temp += c;
            state = 1;
        }else if(c>96 && c<123){
            temp += (c - 32);
            state = 1;
        }else{
            if(state == 1) break;
        }
    }

    Color value = GetColor(0x00000000);
    
    if(stringToColorMap.count(temp) == 1) value = stringToColorMap[temp];
    else{ std::cout << "Color not found " << std::endl;}

    return value;
}

std::pair<float, float> layoutTree::getTextNodeHeight(layoutNode* node){
    float ascender  = 0;
    float descender = 0;
    
    ascender = node->fontSize;
    node->height = ascender;

    return std::make_pair(ascender, descender);
}
