#include "../raylib/include/raylib.h"
#include "url.h"
#include <sys/socket.h>
#include "parser.h"
#include "layout.h"

int WINDOW_HEIGHT = 900;
int WINDOW_WIDTH  = 1600;

int ywindow = 0;

bool inView(layoutNode* node, int yOffset){
    if(node->y+yOffset+node->height < ywindow) return false;
    if( node->y + yOffset> WINDOW_HEIGHT - ywindow) return false;
    return true;
}

void renderLayoutTree(layoutNode* node, int yOffset){

    if(!inView(node, yOffset)) return;

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

    if(!inView(node, yOffset)) return;

    switch(node->type){
        case nodeType::text: {
            DrawRectangleLines(node->x, node->y+yOffset , node->width, node->height, PINK);
            DrawRectangle(node->x, node->y+yOffset , node->width, node->height, GetColor(0xfa25f722));
            break;
        }
        case nodeType::inlineContainer:{
            DrawRectangleLines(node->x, node->y+yOffset , node->width, node->height, GREEN);
            DrawRectangle(node->x, node->y+yOffset , node->width, node->height, GetColor(0x77d47944));
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

layoutNode* hitDetect(layoutNode* node, int x, int y){
    bool condition = ( x>node->x and x<(node->x+node->width) ) and ( y>node->y and y<(node->y+node->height) );
    if(condition){
        for(auto child : node->children){
            layoutNode* temp = hitDetect(child, x, y);
            if(temp) return temp;
        }
        return node;
    }else{
        return nullptr;
    }
}

int main2(){

    std::string url = "localhost/tt.html";

    curlReader fetcher;

    std::string body;
    fetcher.fetch(url, body);

    // parsing the html to make a dom tree
    htmlParser parser;

    // treeNode* htmlNode;
    // treeNode* bodyNode;

    for(int i=0; i<100000; i++){
        // fetcher.fetch(url, body);
        if(parser.domTree) parser.deleteTree(parser.domTree);
        parser.parse(body); // passing in the html
        // std::cout << "Parsed html and made tree" << std::endl;
        // htmlNode = parser.findNodeByName("html", parser.domTree);
        // parser.parseAttributes(parser.domTree);
        // std::cout << "Parsed attributes" << std::endl;
        // bodyNode = parser.findNodeByName("body", parser.domTree);
        // parser.inheritCss(bodyNode);
        // std::cout << "Inherited css" << std::endl;
        parser.traverse(parser.domTree, 0);
    }

    while(true);

    return 1;
}

int main(int argc, char **argv){

    WINDOW_HEIGHT = 900;
    WINDOW_WIDTH  = 1600;

    SetConfigFlags(FLAG_WINDOW_HIGHDPI);
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "html viewer");
    SetTargetFPS(60);

    std::string url = argv[1];
    std::cout << url << std::endl;

    // converting address to ip and getting html from server
    // std::string test = "http://127.0.0.1/wow.html";
    // urlReader testReader;
    // testReader.read(test);
    // std::string header, body;
    // testReader.request(header, body);

    addGlobalDefaults("display: inline; color: black; background-color: transparent; font-size: 16px; font-weight: normal; font-style: normal; text-decoration: none; cursor: auto; margin-top: 0; margin-right: 0; margin-bottom: 0; margin-left: 0; padding-top: 0; padding-right: 0; padding-bottom: 0; padding-left: 0;");

    addDefaults("body",   "display: block; margin-top: 8px; margin-bottom:8px; margin-right: 8px; margin-left: 8px; background-color: WHITE;");
    addDefaults("p",      "display: block; margin-top: 1em; margin-bottom: 1em;");
    addDefaults("div",    "display: block;");
    addDefaults("h1",     "display: block; font-size: 2em; font-weight: bold; margin-top: 0.67em; margin-bottom: 0.67em;");
    addDefaults("h2",     "display: block; font-size: 1.5em; font-weight: bold; margin-top: 0.75em; margin-bottom: 0.75em;");
    addDefaults("h3",     "display: block; font-size: 1.17em; font-weight: bold; margin-top: 0.83em; margin-bottom: 0.83em;");
    addDefaults("span",   "display: inline;");

    // enabling the line below somehow crashes the browser
    addDefaults("b",      "color: RED;");

    addDefaults("strong", "display: inline; font-weight: bold;");
    addDefaults("em",     "display: inline; font-style: italic;");
    addDefaults("a",      "display: inline; color: blue; text-decoration: underline; cursor: pointer;");
    addDefaults("br",     "display: block;");
    addDefaults("hr",     "display: block; margin: 10px; padding:0.5px; background-color: GRAY;");

    curlReader fetcher;

    std::string body;
    fetcher.fetch(url, body);

    // parsing the html to make a dom tree
    htmlParser parser;
    if(parser.domTree) delete parser.domTree;
    parser.parse(body); // passing in the html

    // parser.traverse(parser.domTree, 0);
    std::cout << "Parsed html and made tree" << std::endl;

    // adding some basic attributes to the html node
    treeNode* htmlNode = parser.findNodeByName("html", parser.domTree);

    // inherit css properties only for the nodes in body 

    parser.parseAttributes(parser.domTree);

    std::cout << "Parsed attributes" << std::endl;

    treeNode* bodyNode = parser.findNodeByName("body", parser.domTree);

    parser.inheritCss(bodyNode);

    std::cout << "Inherited css" << std::endl;

    parser.traverse(parser.domTree, 0);

    /* making the layout tree object */
    layoutTree layoutRenderTree;

    layoutNode* underMouse = nullptr;

    // rendering 
    bool debugMode       = false;
    bool layoutTreeDirty = true;
    float zoomFactor     = 0.01f;
    float scrollFactor   = 4;
    float yOffset        = 0.0f;

    while (!WindowShouldClose())
    {
        yOffset += GetMouseWheelMove()*scrollFactor;

        if (IsKeyDown(KEY_J)) yOffset -= 10;
        if (IsKeyDown(KEY_K)) yOffset += 10;

        // limit the scroll offset
        if(yOffset>ywindow) yOffset = ywindow;
        // if(layoutRenderTree.layoutTreeRoot){
        //     if(yOffset< -layoutRenderTree.layoutTreeRoot->children[0]->height+WINDOW_HEIGHT ) yOffset = -layoutRenderTree.layoutTreeRoot->children[0]->height + WINDOW_HEIGHT;
        //     if(layoutRenderTree.layoutTreeRoot->children[0]->height < WINDOW_HEIGHT) yOffset = ywindow;
        // }
        
        if(IsKeyDown(KEY_R)){
            fetcher.fetch(url, body);
            delete parser.domTree;
            std::cout <<"body capacity" <<body.capacity() << std::endl;
            parser.parse(body); // passing in the html
            std::cout << "Parsed html and made tree" << std::endl;
            htmlNode = parser.findNodeByName("html", parser.domTree);
            parser.parseAttributes(parser.domTree);
            std::cout << "Parsed attributes" << std::endl;
            bodyNode = parser.findNodeByName("body", parser.domTree);
            parser.inheritCss(bodyNode);
            std::cout << "Inherited css" << std::endl;
            // parser.traverse(parser.domTree, 0);
            layoutTreeDirty = true;
        }

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
            // layoutRenderTree.traverse(layoutRenderTree.layoutTreeRoot, 0);
            layoutRenderTree.cursorX = 0;
            layoutRenderTree.cursorY = 0;
            layoutTreeDirty = false;
            // std::cout << layoutRenderTree.scale << std::endl;
            underMouse = nullptr;
        }

        underMouse = hitDetect(layoutRenderTree.layoutTreeRoot ,GetMousePosition().x, GetMousePosition().y-yOffset);

        BeginDrawing();

        if(!debugMode){
            ClearBackground(layoutRenderTree.layoutTreeRoot->children[0]->backgroundColor);
            renderLayoutTree(layoutRenderTree.layoutTreeRoot, yOffset);
        }else{
            ClearBackground(BLACK);
            renderLayoutTreeDebug(layoutRenderTree.layoutTreeRoot, yOffset);
        }


        DrawRectangle(0, 0 , WINDOW_WIDTH, ywindow, GetColor(0xfa25f744));
        DrawRectangle(0, WINDOW_HEIGHT-ywindow , WINDOW_WIDTH, ywindow, GetColor(0xfa25f744));

        if(underMouse != nullptr){
            DrawRectangleLines(underMouse->x, underMouse->y+yOffset, underMouse->width, underMouse->height, GetColor(0xff0000ff));
        }

        EndDrawing();
    }

    CloseWindow();

    return 0;
}

