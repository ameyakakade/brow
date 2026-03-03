#include "../raylib/include/raylib.h"
#include "url.h"
#include <sys/socket.h>
#include "parser.h"
#include "layout.h"
#include "render.h"

int WINDOW_HEIGHT = 900;
int WINDOW_WIDTH  = 1600;
int ywindow = 0;

void remakeLayoutTree(layoutTree& lTree, treeNode* body){
    delete lTree.layoutTreeRoot;
    lTree.layoutTreeRoot = new layoutNode;
    lTree.makeLayoutTree(body, lTree.layoutTreeRoot);
    lTree.windowWidth = WINDOW_WIDTH;
    lTree.windowHeight = WINDOW_HEIGHT;
    lTree.calculateLayoutPass(lTree.layoutTreeRoot, lTree.windowWidth);
    lTree.cursorX = 0;
    lTree.cursorY = 0;
}

void downloadAndMakeDomTree(curlReader& fetcher, std::string& url, std::string& body, treeNode*& domTree, treeNode*& htmlNode, treeNode*& bodyNode){
    fetcher.fetch(url, body);
    htmlParser parser;
    delete parser.domTree;
    parser.parse(body); // passing in the html
    std::cout << "Parsed html and made tree" << std::endl;
    htmlNode = parser.findNodeByName("html", parser.domTree);
    parser.parseAttributes(parser.domTree);
    std::cout << "Parsed attributes" << std::endl;
    bodyNode = parser.findNodeByName("body", parser.domTree);
    parser.inheritCss(bodyNode);
    std::cout << "Inherited css" << std::endl;

    // switching trees
    treeNode* temp = domTree;
    domTree = parser.domTree;
    delete temp;
}

void initDefaults();

int main(int argc, char **argv){

    WINDOW_HEIGHT = 900;
    WINDOW_WIDTH  = 1600;

    SetConfigFlags(FLAG_WINDOW_HIGHDPI);
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "html viewer");
    SetTargetFPS(60);

    std::string url = argv[1];
    std::cout << url << std::endl;

    initDefaults();

    curlReader fetcher;

    std::string body;
    fetcher.fetch(url, body);

    // parsing the html to make a dom tree
    treeNode* domTree  = nullptr;
    treeNode* htmlNode = nullptr;
    treeNode* bodyNode = nullptr;

    /* making the layout tree object */
    layoutTree layoutRenderTree;

    layoutNode* underMouse = nullptr;

    // rendering 
    bool debugMode       = false;
    bool layoutTreeDirty = true;
    float zoomFactor     = 0.01f;
    float scrollFactor   = 4;
    float yOffset        = 0.0f;

    downloadAndMakeDomTree(fetcher, url, body, domTree, htmlNode, bodyNode);

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
            downloadAndMakeDomTree(fetcher, url, body, domTree, htmlNode, bodyNode);
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
            remakeLayoutTree(layoutRenderTree, bodyNode);
            layoutTreeDirty = false;
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

void initDefaults(){
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
}
