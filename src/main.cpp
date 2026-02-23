#include "../raylib/include/raylib.h"
#include "url.h"
#include <sys/socket.h>
#include "parser.h"
#include "layout.h"

#define WINDOW_HEIGHT 800
#define WINDOW_WIDTH 700


void renderLayoutTree(layoutNode* node){

    switch(node->type){
        case nodeType::text: {
            DrawText(node->text.c_str(),  node->x, node->y , node->fontSize, WHITE);

        }
        default: DrawRectangle(node->x, node->y, node->width, node->height, node->backgroundColor);
    }

    for(auto child : node->children){
        renderLayoutTree(child);
    }
}

int main(){

    SetConfigFlags(FLAG_WINDOW_HIGHDPI);
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "html viewer");

    // converting address to ip and getting html from server
    std::string test = "http://127.0.0.1/index.html";
    urlReader testReader;
    testReader.read(test);
    std::string header, body;
    testReader.request(header, body);

    addGlobalDefaults("display: inline; color: black; background-color: transparent; font-size: 16px; font-weight: normal; font-style: normal; text-decoration: none; cursor: auto; margin-top: 0; margin-right: 0; margin-bottom: 0; margin-left: 0; padding-top: 0; padding-right: 0; padding-bottom: 0; padding-left: 0;");

    addDefaults("body",   "display: block; margin-top: 8px; margin-bottom:8px; margin-right: 8px; margin-left: 8px;");
    addDefaults("p",      "display: block; margin-top: 1em; margin-bottom: 1em;");
    addDefaults("div",    "display: block;");
    addDefaults("h1",     "display: block; font-size: 2em; font-weight: bold; margin-top: 0.67em; margin-bottom: 0.67em; ");
    addDefaults("h2",     "display: block; font-size: 1.5em; font-weight: bold; margin-top: 0.75em; margin-bottom: 0.75em; ");
    addDefaults("h3",     "display: block; font-size: 1.17em; font-weight: bold; margin-top: 0.83em; margin-bottom: 0.83em; ");
    addDefaults("span",   "display: inline; ");
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

    // parser.traverse(parser.domTree, 0);

    /* making the layout tree */
    layoutTree layoutRenderTree;

    // make a temporary root node for making layout tree
    layoutNode* temp = layoutRenderTree.layoutTreeRoot = new layoutNode;
    layoutRenderTree.makeLayoutTree(bodyNode, layoutRenderTree.layoutTreeRoot);
    // discard the root node and make the new root the body node
    layoutRenderTree.layoutTreeRoot = layoutRenderTree.layoutTreeRoot->children[0];
    delete temp;

    layoutRenderTree.windowWidth = WINDOW_WIDTH;
    layoutRenderTree.windowHeight = WINDOW_HEIGHT;
    
    layoutRenderTree.calculateLayoutPass(layoutRenderTree.layoutTreeRoot, layoutRenderTree.windowWidth);

    layoutRenderTree.traverse(layoutRenderTree.layoutTreeRoot, 0);

    // rendering 

    while (!WindowShouldClose())
    {
        int boxPositionY = GetMouseWheelMove();
        BeginDrawing();

            ClearBackground(layoutRenderTree.layoutTreeRoot->backgroundColor);
            renderLayoutTree(layoutRenderTree.layoutTreeRoot);

        EndDrawing();
    }

    CloseWindow();

    return 0;
}

