#include "../raylib/include/raylib.h"
#include "layout.h"
#include "parser.h"
#include "render.h"
#include "url.h"
#include <sys/socket.h>
#include <thread>

int WINDOW_HEIGHT = 900;
int WINDOW_WIDTH = 1600;
int ywindow = 000;

std::mutex m;
std::atomic<bool> reload;

Font calibri;

void remakeLayoutTree(layoutTree &lTree, treeNode *body)
{
    delete lTree.layoutTreeRoot;
    lTree.layoutTreeRoot = new layoutNode;
    lTree.makeLayoutTree(body, lTree.layoutTreeRoot);
    lTree.windowWidth = WINDOW_WIDTH;
    lTree.windowHeight = WINDOW_HEIGHT;
    lTree.cursorX = 0;
    lTree.cursorY = 0;
    lTree.traverse(lTree.layoutTreeRoot, 0);
    lTree.calculateLayoutPass(lTree.layoutTreeRoot, lTree.windowWidth);
    lTree.traverse(lTree.layoutTreeRoot, 0);
}

void downloadAndMakeDomTree(curlReader &fetcher, std::string &url,
                            std::string &body, treeNode *&domTree,
                            treeNode *&htmlNode, treeNode *&bodyNode,
                            std::atomic<bool> &done)
{
    while (true)
    {
        bool check = reload.load();
        if (check)
        {
            if (!fetcher.fetch(url, body))
            {
                reload.store(false);
                done.store(true);
                return;
            }
            htmlParser parser;
            delete parser.domTree;
            parser.parse(body); // passing in the html
            std::cout << "Parsed html and made tree" << std::endl;
            treeNode *htmlNodeTemp =
                parser.findNodeByName("html", parser.domTree);
            parser.parseAttributes(parser.domTree);
            std::cout << "Parsed attributes" << std::endl;
            treeNode *bodyNodeTemp =
                parser.findNodeByName("body", parser.domTree);
            parser.inheritCss(bodyNodeTemp);
            std::cout << "Inherited css" << std::endl;
            parser.traverse(parser.domTree, 0);

            treeNode *temp;
            if (!htmlNodeTemp or !bodyNodeTemp)
            {
                reload.store(false);
                done.store(true);
                return;
            }
            {
                std::unique_lock<std::mutex> m;
                // switching trees
                temp = domTree;
                domTree = parser.domTree;

                htmlNode = htmlNodeTemp;
                bodyNode = bodyNodeTemp;
            }
            // delete temp after unlocking mutex so ui doesnt stop for long
            delete temp;
            reload.store(false);
            done.store(true);
        }
    }
}

void initDefaults();

int main(int argc, char **argv)
{

    WINDOW_HEIGHT = 900;
    WINDOW_WIDTH = 1600;

    SetConfigFlags(FLAG_WINDOW_HIGHDPI);
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "html viewer");
    SetTargetFPS(60);

    calibri = LoadFontEx("resources/calibri.ttf", 300, 0, 250);

    std::string url = argv[1];
    std::cout << url << std::endl;
    initDefaults();

    curlReader fetcher;

    std::string body;
    fetcher.fetch(url, body);

    // pointers to important nodes
    treeNode *domTree = nullptr;
    treeNode *htmlNode = nullptr;
    treeNode *bodyNode = nullptr;

    reload.store(true);

    /* making the layout tree object */
    layoutTree layoutRenderTree;

    layoutNode *underMouse = nullptr;

    // rendering
    bool debugMode = false;
    std::atomic<bool> layoutTreeDirty = true;
    float zoomFactor = 0.1;
    float scrollFactor = 4;
    float yOffset = 0.0f;
    float ratio = 0;
    float counters[5] = {0};
    float bodyHeight = INT_MAX;

    std::cout << "Starting thread" << std::endl;
    std::thread t1{downloadAndMakeDomTree, std::ref(fetcher),
                   std::ref(url),          std::ref(body),
                   std::ref(domTree),      std::ref(htmlNode),
                   std::ref(bodyNode),     std::ref(layoutTreeDirty)};

    while (!WindowShouldClose())
    {
        yOffset += GetMouseWheelMove() * scrollFactor;

        if (IsKeyDown(KEY_J))
            yOffset -= 10;
        if (IsKeyDown(KEY_K))
            yOffset += 10;

        if (IsKeyDown(KEY_B))
            yOffset = -bodyHeight + WINDOW_HEIGHT;
        if (IsKeyDown(KEY_T))
            yOffset = 0;

        ratio = yOffset / (bodyHeight - WINDOW_HEIGHT);

        // limit the scroll offset
        if (yOffset > ywindow)
            yOffset = ywindow;
        if (layoutRenderTree.layoutTreeRoot)
        {
            if (yOffset < -bodyHeight + WINDOW_HEIGHT * 0.9)
                yOffset = -bodyHeight + WINDOW_HEIGHT * 0.9;
            if (bodyHeight < WINDOW_HEIGHT)
                yOffset = ywindow;
        }

        if (IsKeyDown(KEY_R))
        {
            counters[0] += 1;
        }
        else
        {
            if (counters[0] > 0)
            {
                reload.store(true);
                layoutTreeDirty = true;
                counters[0] = 0;
            }
        }

        if (IsKeyDown(KEY_L))
            debugMode = true;
        if (IsKeyDown(KEY_H))
            debugMode = false;

        if (IsKeyDown(KEY_UP))
        {
            counters[1] += 1;
            if (counters[1] > 20)
            {
                counters[1] = 0;
                layoutTreeDirty = true;
                layoutRenderTree.scale += zoomFactor;
                ratio = yOffset / bodyHeight;
            }
        }
        else
        {
            if (counters[1] > 0)
            {
                counters[1] = 0;
                layoutTreeDirty = true;
                layoutRenderTree.scale += zoomFactor;
                ratio = yOffset / bodyHeight;
            }
        }

        if (IsKeyDown(KEY_DOWN))
        {
            counters[2] += 1;
            if (counters[2] > 20)
            {
                counters[2] = 0;
                layoutTreeDirty = true;
                layoutRenderTree.scale -= zoomFactor;
                ratio = yOffset / bodyHeight;
            }
        }
        else
        {
            if (counters[2] > 0)
            {
                counters[2] = 0;
                layoutTreeDirty = true;
                layoutRenderTree.scale -= zoomFactor;
                ratio = yOffset / bodyHeight;
            }
        }

        if (GetScreenWidth() != WINDOW_WIDTH)
        {
            WINDOW_WIDTH = GetScreenWidth();
            layoutTreeDirty = true;
        }

        if (GetScreenHeight() != WINDOW_HEIGHT)
        {
            WINDOW_HEIGHT = GetScreenHeight();
            layoutTreeDirty = true;
        }

        if (layoutTreeDirty)
        {
            // remake the layout tree
            if (bodyNode)
                remakeLayoutTree(layoutRenderTree, bodyNode);
            if (layoutRenderTree.layoutTreeRoot)
            {
                bodyHeight =
                    layoutRenderTree.layoutTreeRoot->children[0]->height;
            }
            layoutTreeDirty = false;
            underMouse = nullptr;
        }

        if (layoutRenderTree.layoutTreeRoot)
            underMouse =
                hitDetect(layoutRenderTree.layoutTreeRoot, GetMousePosition().x,
                          GetMousePosition().y - yOffset);

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
        {
            if (underMouse)
            {
                if (underMouse->originNode)
                {
                    std::string temp = url;
                    if (findUrl(url, underMouse->originNode))
                    {
                        reload.store(true);
                        layoutTreeDirty = true;
                    }
                }
            }
        }

        BeginDrawing();

        if (layoutRenderTree.layoutTreeRoot)
        {
            if (!debugMode)
            {
                ClearBackground(layoutRenderTree.layoutTreeRoot->children[0]
                                    ->backgroundColor);
                renderLayoutTree(layoutRenderTree.layoutTreeRoot, yOffset);
            }
            else
            {
                ClearBackground(BLACK);
                renderLayoutTreeDebug(layoutRenderTree.layoutTreeRoot, yOffset);
            }
        }

        DrawRectangle(0, 0, WINDOW_WIDTH, ywindow, GetColor(0xfa25f744));
        DrawRectangle(0, WINDOW_HEIGHT - ywindow, WINDOW_WIDTH, ywindow,
                      GetColor(0xfa25f744));

        if (underMouse != nullptr)
        {
            DrawRectangleLines(underMouse->x, underMouse->y + yOffset,
                               underMouse->width, underMouse->height,
                               GetColor(0xff4400ff));
            // DrawRectangle(underMouse->x, underMouse->y+yOffset,
            // underMouse->width, underMouse->height, GetColor(0xff440055));
        }

        DrawText((std::to_string((int)(-ratio * 100)) + "%").c_str(), 10,
                 WINDOW_HEIGHT - 28, 26, RED);

        EndDrawing();
    }

    CloseWindow();

    return 0;
}

void initDefaults()
{
    addGlobalDefaults(
        "display: inline; color: black; background-color: transparent; "
        "font-size: 16px; font-weight: normal; font-style: normal; "
        "text-decoration: none; cursor: auto; margin-top: 0; margin-right: 0; "
        "margin-bottom: 0; margin-left: 0; padding-top: 0; padding-right: 0; "
        "padding-bottom: 0; padding-left: 0;");

    addDefaults(
        "body",
        "display: block; margin-top: 8px; margin-bottom:8px; margin-right: "
        "8px; margin-left: 8px; background-color: WHITE;");
    addDefaults("p", "display: block; margin-top: 1em; margin-bottom: 1em;");
    addDefaults("div", "display: block;");
    addDefaults("h1", "display: block; font-size: 2em; font-weight: bold; "
                      "margin-top: 0.67em; margin-bottom: 0.67em;");
    addDefaults("h2", "display: block; font-size: 1.5em; font-weight: bold; "
                      "margin-top: 0.75em; margin-bottom: 0.75em;");
    addDefaults("h3", "display: block; font-size: 1.17em; font-weight: bold; "
                      "margin-top: 0.83em; margin-bottom: 0.83em;");
    addDefaults("span", "display: inline;");

    // enabling the line below somehow crashes the browser
    addDefaults("b", "color: RED;");

    addDefaults("strong", "display: inline; font-weight: bold;");
    addDefaults("em", "display: inline; font-style: italic;");
    addDefaults("a", "display: inline; color: blue; text-decoration: "
                     "underline; cursor: pointer;");
    addDefaults("br", "display: block;");
    addDefaults(
        "hr",
        "display: block; margin: 10px; padding:0.5px; background-color: GRAY;");
}
