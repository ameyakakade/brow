#include "../raylib/include/raylib.h"
#include "url.h"
#include <sys/socket.h>
#include "parser.h"
#include "layout.h"



void renderLayoutTree(layoutNode* node, int yOffset){

    switch(node->type){
        case nodeType::text: {
            DrawRectangle(node->x, node->y+yOffset, node->width, node->height, node->backgroundColor);
            DrawText(node->text.c_str(),  node->x, node->y+yOffset, node->fontSize, node->color);
            break;
        }
        default: DrawRectangle(node->x, node->y+yOffset, node->width, node->height, node->backgroundColor);
    }

    for(auto child : node->children){
        renderLayoutTree(child, yOffset);
    }
}

void renderLayoutTreeDebug(layoutNode* node, int yOffset){

    switch(node->type){
        case nodeType::text: {
            DrawRectangleLines(node->x, node->y+yOffset , node->width, node->height, PINK);
            DrawRectangle(node->x, node->y+yOffset , node->width, node->height, GetColor(0xfa25f722));
            break;
        }
        case nodeType::inlineContainer:{
            DrawRectangleLines(node->x, node->y+yOffset , node->width, node->height, GREEN);
            DrawRectangle(node->x, node->y+yOffset , node->width, node->height, GetColor(0x77d47933));
            break;
        }
        case nodeType::lineContainer:{
            DrawRectangleLines(node->x, node->y+yOffset , node->width, node->height, YELLOW);
            DrawRectangle(node->x, node->y+yOffset , node->width, node->height, GetColor(0xFFEA4F33));
            break;
        }
        default:{
            DrawRectangleLines(node->x, node->y+yOffset, node->width, node->height, RED);
            DrawRectangle(node->x, node->y+yOffset , node->width, node->height, GetColor(0xff000015));
        } 
    }

    for(auto child : node->children){
        renderLayoutTreeDebug(child, yOffset);
    }
}

int main(){

    int WINDOW_HEIGHT = 900;
    int WINDOW_WIDTH  = 1600;

    SetConfigFlags(FLAG_WINDOW_HIGHDPI);
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "html viewer");
    SetTargetFPS(60);



    // converting address to ip and getting html from server
    std::string test = "http://127.0.0.1/wow.html";
    urlReader testReader;
    testReader.read(test);
    std::string header, body;
    testReader.request(header, body);

    addGlobalDefaults("display: inline; color: black; background-color: transparent; font-size: 16px; font-weight: normal; font-style: normal; text-decoration: none; cursor: auto; margin-top: 0; margin-right: 0; margin-bottom: 0; margin-left: 0; padding-top: 0; padding-right: 0; padding-bottom: 0; padding-left: 0;");

    addDefaults("body",   "display: block; margin-top: 8px; margin-bottom:8px; margin-right: 8px; margin-left: 8px;");
    addDefaults("p",      "display: block; margin-top: 1em; margin-bottom: 1em;");
    addDefaults("div",    "display: block;");
    addDefaults("h1",     "display: block; font-size: 2em; font-weight: bold; margin-top: 0.67em; margin-bottom: 0.67em;");
    addDefaults("h2",     "display: block; font-size: 1.5em; font-weight: bold; margin-top: 0.75em; margin-bottom: 0.75em;");
    addDefaults("h3",     "display: block; font-size: 1.17em; font-weight: bold; margin-top: 0.83em; margin-bottom: 0.83em;");
    addDefaults("span",   "display: inline;");
    addDefaults("strong", "display: inline; font-weight: bold;");
    addDefaults("em",     "display: inline; font-style: italic;");
    addDefaults("a",      "display: inline; color: blue; text-decoration: underline; cursor: pointer;");

    // parsing the html to make a dom tree
    htmlParser parser;
    parser.parse(body); // passing in the html

    // adding some basic attributes to the html node
    treeNode* htmlNode;
    for(auto node : parser.domTree->children){
        if(node->name == "html") htmlNode = node;
    }

    // inherit css properties only for the nodes in body 

    parser.parseAttributes(parser.domTree);

    treeNode* bodyNode;
    for(auto node : htmlNode->children){
        if(node->name == "body") bodyNode = node;
    }
    parser.inheritCss(bodyNode);

    parser.traverse(parser.domTree, 0);

    /* making the layout tree object */
    layoutTree layoutRenderTree;

    // make a root node for making layout tree
    // layoutNode* temp = layoutRenderTree.layoutTreeRoot = new layoutNode;
    // layoutRenderTree.makeLayoutTree(bodyNode, layoutRenderTree.layoutTreeRoot);
    // we are not doing this for now ->
        // discard the root node and make the new root the body node
        // layoutRenderTree.layoutTreeRoot = layoutRenderTree.layoutTreeRoot->children[0];
        // delete temp;


    // rendering 
    bool debugMode       = false;
    bool layoutTreeDirty = true;
    float zoomFactor     = 0.01f;
    float scrollFactor   = 4;
    float yOffset        = 0.0f;

    while (!WindowShouldClose())
    {
        yOffset += GetMouseWheelMove()*scrollFactor;
        if (IsKeyDown(KEY_RIGHT)) debugMode = true;
        if (IsKeyDown(KEY_LEFT)) debugMode = false;
        if (IsKeyDown(KEY_UP)){
            layoutTreeDirty = true;
            layoutRenderTree.scale += zoomFactor;
        }
        if (IsKeyDown(KEY_DOWN)){
            layoutTreeDirty = true;
            layoutRenderTree.scale -= zoomFactor;
        }

        if(GetScreenWidth() != WINDOW_WIDTH){
            WINDOW_WIDTH = GetScreenWidth();
            layoutTreeDirty = true;
        }

        if(GetScreenHeight() != WINDOW_HEIGHT){
            WINDOW_HEIGHT = GetScreenHeight();
            layoutTreeDirty = true;
        }

        if(layoutTreeDirty){
            // remake the layout tree
            if(layoutRenderTree.layoutTreeRoot) delete layoutRenderTree.layoutTreeRoot;
            layoutNode* temp = layoutRenderTree.layoutTreeRoot = new layoutNode;
            layoutRenderTree.makeLayoutTree(bodyNode, layoutRenderTree.layoutTreeRoot);
            layoutRenderTree.windowWidth = WINDOW_WIDTH;
            layoutRenderTree.windowHeight = WINDOW_HEIGHT;
            layoutRenderTree.calculateLayoutPass(layoutRenderTree.layoutTreeRoot, layoutRenderTree.windowWidth);
            layoutRenderTree.traverse(layoutRenderTree.layoutTreeRoot, 0);
            layoutRenderTree.cursorX = 0;
            layoutRenderTree.cursorY = 0;
            layoutTreeDirty = false;
            // std::cout << layoutRenderTree.scale << std::endl;
        }

        BeginDrawing();

        if(!debugMode){
            ClearBackground(layoutRenderTree.layoutTreeRoot->children[0]->backgroundColor);
            renderLayoutTree(layoutRenderTree.layoutTreeRoot, yOffset);
        }else{
            ClearBackground(BLACK);
            renderLayoutTreeDebug(layoutRenderTree.layoutTreeRoot, yOffset);
        }

        EndDrawing();
    }

    CloseWindow();

    return 0;
}

