#include "layout.h"
#include <stack>

extern int WINDOW_HEIGHT;
extern int WINDOW_WIDTH;

extern int ywindow;

bool inView(layoutNode *node, int yOffset);
void renderLayoutTree(layoutNode *node, int yOffset);
void renderLayoutTreeDebug(layoutNode *node, int yOffset);
layoutNode *hitDetect(layoutNode *node, int x, int y);
bool findUrl(std::string& url, treeNode* hit);
