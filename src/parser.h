#pragma once
#include <string>
#include <vector>
#include <unordered_map>

struct cssProperty{
    std::string name;
    std::string value;
    bool inheritable;

};

struct attributes{
    std::string name;
    std::string value;
};

enum tagType{
    html, text
};

void addDefaults(std::string name, std::string input);
void addGlobalDefaults(std::string input);

class treeNode{
    public:
        treeNode(std::string input, treeNode* parent);
        treeNode* parentNode = nullptr;
        std::vector<treeNode*> children = {};
        std::string name;
        int type;
        std::string rawAttributes;
        std::vector<cssProperty> style;
        std::unordered_map<std::string, size_t> cssPropertyIndexCache;
        std::vector<attributes> nodeAttributes;
};

class htmlParser{
    public:
        void parse(std::string& input);
        void parseAttributes(treeNode* node);
        void inheritCss(treeNode* node);
        void traverse(treeNode* node, int level);
        void deleteTree(treeNode* node);
        treeNode* findNodeByName(std::string name, treeNode* node);

        treeNode* domTree = nullptr;
};
