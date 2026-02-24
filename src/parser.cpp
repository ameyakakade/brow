#include "parser.h"
#include <iostream>
#include <unordered_set>

enum readingState{
    readingTagStart, readingTag, readingTagContents, outside, readingTagEnd
};

enum readingAttributes{
    readingName, startReadingValue, readingValue, endReadingValue, ignoring, equal
};

enum dataType{
    none, tag, endTag, textData
};

std::unordered_map<std::string, std::vector<cssProperty>> tagDefaults;
std::vector<cssProperty> globalDefaults;

void parseCssIntoArray(std::string& input, std::vector<cssProperty>& array){

    int state = ignoring;
    std::string name;
    std::string value;

    for(char c : input){

        switch(c){

            case ' ':
                break;

            case ':':
                state = equal;
                break;

            case ';':
                state = endReadingValue;

            default:
                if(state == ignoring){
                    state = readingName;
                }else if(state == equal){
                    state = readingValue;
                }

        }

        switch(state){
            
            case readingName:
                if(c != ' ') name += c;
                break;

            case readingValue:
                value += c;
                break;

            case endReadingValue:{
                cssProperty temp;
                temp.name = name;
                temp.value = value;
                temp.inheritable = false;

                array.push_back(temp);
                
                name.clear();
                value.clear();
                state = ignoring;
            }

            default:
                ;
        }
    }
}

void addDefaults(std::string name, std::string input){
    std::vector<cssProperty> temp;
    parseCssIntoArray(input, temp);
    tagDefaults[name] = temp;
}

void addGlobalDefaults(std::string input){
    parseCssIntoArray(input, globalDefaults);
}

std::unordered_set<std::string> inheritableProperties = {"color", "font-size"};

void checkInheritable(cssProperty& property) {
    property.inheritable = inheritableProperties.count(property.name);
}

void htmlParser::parse(std::string input){
    domTree = new treeNode("ROOT", nullptr);
    int state = outside;
    int type = none;
    std::string data;
    treeNode* curr = domTree;
    std::string attributes;

    for(auto letter : input){

        switch (letter){

            case '<':
                state = readingTagStart;
                type = tag;
                break;

            case '/':
                if(state == readingTagStart){
                    type = endTag; 
                }
                break;

            case '>':
                state = readingTagEnd;
                break;

            case ' ':
                if(state == readingTag){
                    state = readingTagContents;
                }

            default:
                if(state == readingTagStart){
                    state = readingTag;
                }else if(state == readingTagEnd){
                    state = outside;
                    type = textData;
                }
        }

        switch (state){

            case readingTagStart: {

                // replacing whitespace characters with spaces
                std::string temp;
                bool state = false;
                for(char c : data){
                    if(c == '\n' or c == '\t' or c == '\r') state = true;
                    else{
                        if(state){
                            if(c != ' '){
                                temp += ' ';
                                temp += c;
                                state = false;
                            } 
                        }else{
                            temp += c;
                        }
                    }
                } 

                data = temp;

                // init data when we start reading smth
                bool check = false;
                for(char c : data) check = check or (c != ' ');
                if(check){
                    if(curr->name == "b" or curr->name == "span" ){
                        curr->nodeAttributes.push_back({"text", data});
                        curr->type = text;
                    }else{
                        treeNode* textNode = new treeNode("textNode", curr);
                        textNode->nodeAttributes.push_back({"text", data});
                        textNode->type = text;
                        curr->children.push_back(textNode);
                    }
                }
                data.clear();
                attributes.clear();
                break;
            }

            case readingTag:
                data += letter;
                break;

            case readingTagContents:
                attributes += letter;
                break;

            case outside:
                data += letter;
                break;

            case readingTagEnd:{
                bool check = false;
                for(char c : attributes) check = check or (c != ' ') and (c != '\n') and (c != '\t');
                if(type == tag){
                    treeNode* temp = new treeNode(data, curr);
                    temp->type = html;
                    curr->children.emplace_back(temp);
                    if(check){
                        temp->rawAttributes = attributes;
                    }
                    curr = temp;
                }else if (type == endTag){
                    curr = curr->parentNode;
                }
                data.clear();
                attributes.clear();
                break;
            }

        }

    }

}

void htmlParser::parseAttributes(treeNode* node){

    // readingName, readingValue, ignoring, equal
    int state = ignoring;
    std::string name;
    std::string value;
    bool check = false;

    for(auto c : node->rawAttributes){

        switch(c){

            case ' ':
                if(state == readingName) state = ignoring;
                break;

            case '=':
                check = true;
                state = ignoring;
                break;

            case '"':
                if(state == ignoring) state = startReadingValue;
                if(state == readingName) state = startReadingValue;
                if(state == readingValue) state = endReadingValue;
                break;

            default:
                if(state == ignoring) state = readingName;

        }

        switch(state){

            case readingName:
                name += c; 
                break;

            case startReadingValue:
                value.clear();
                state = readingValue;
                break;

            case endReadingValue:{
                if(check){
                    attributes temp;
                    temp.name = name;
                    temp.value = value;
                    node->nodeAttributes.push_back(temp);
                }
                check = false;
                name.clear();
                value.clear();
                state = ignoring;
                break;
            }

            case readingValue:
                value += c;
                break;

            default:
                ;

        }
                
    }

    for(auto child : node->children){
        parseAttributes(child);
    }
}

void htmlParser::traverse(treeNode* node, int level){
    std::string indent;
    for(int i=0; i<level; i++){
        indent += "   ";
    }
    std::cout << indent << node->name << std::endl;

    for(auto property : node->nodeAttributes){
        std::cout << indent << " Attribute " << property.name << "->" << property.value << std::endl;
    }

    for(auto property : node->style){
        std::cout << indent << "    " << property.name << " : " << property.value << std::endl;
    }
    std::cout << "\n";
    for(auto child : node->children){
        traverse(child, level+1);
    }
}
 
// function to convert 'margin' and 'padding' into its 4 parts
bool transferSingleAttributeIntoFour(std::string identifier, std::vector<cssProperty>& styles, std::unordered_map<std::string, size_t> cache){
    bool dirty = false;

    // function to copy the value into 4 strings
    auto copy = [](std::vector<std::string>& array, std::string value){
        for(auto& a : array){
            a = value;
        }
    };

    cssProperty* propertyToBeChanged;
    if(cache.count(identifier)){
        // assuming the properties -top -bottom -left -right counterparts already exist
        propertyToBeChanged = &styles[cache[identifier]];
        std::vector<std::string> values(4, "0");
        copy(values, propertyToBeChanged->value);
        std::vector<std::string> suffix = {"-top", "-bottom", "-left", "-right"};
        for(int i=0; i<4; i++){
           styles[cache[ identifier+suffix[i] ]].value = values[i];
        }
        dirty = true;
    }    
    return dirty;
}

void htmlParser::inheritCss(treeNode* node){

    /* Inheriting global defaults */
    node->style = globalDefaults;


    // store attributes of self with name and name and index in hashmap

    std::unordered_map<std::string, size_t> selfCssAttributesCache;
    bool cacheDirty = false;
    for(int i=0; i<node->style.size(); i++){
        selfCssAttributesCache[node->style[i].name] = i;
    }

    // lambda to refresh cache
    auto refreshCache = [&](){
        if(cacheDirty){
            for(int i=0; i<node->style.size(); i++) selfCssAttributesCache[node->style[i].name] = i;
            cacheDirty = false;
        }
    };

    /* Inheritance pass start */

    std::vector<cssProperty>* parentCss = &node->parentNode->style; 
    for(cssProperty attribute : *parentCss){
        if(!attribute.inheritable) continue;
        if(!selfCssAttributesCache.count(attribute.name)){
            node->style.push_back(attribute);
            cacheDirty = true;
        }else{
            node->style[selfCssAttributesCache[attribute.name]] = attribute;
        }
    }

    refreshCache();

    /* Inheritance pass end */


    /* Tag defaults pass */

    std::vector<cssProperty>* tagStyles;
    if(tagDefaults.count(node->name)){
        tagStyles = &tagDefaults[node->name];

        // if it finds the property in the cache it updates else adds a new one 
        for(auto property : *tagStyles){
            if(!selfCssAttributesCache.count(property.name)){
                node->style.push_back(property);
                cacheDirty = true;
            }else{
                node->style[selfCssAttributesCache[property.name]] = property;
            }
        }
    }

    refreshCache();

    /* Tag defaults pass end */


    /* Inline style pass */

    attributes styles;
    for(auto attribute : node->nodeAttributes){
        if(attribute.name == "style") {
            styles = attribute;
            break;
        }
    }

    int state = ignoring;
    std::string name;
    std::string value;
    
    for(char c : styles.value){

        switch(c){

            case ' ':
                break;

            case ':':
                state = equal;
                break;

            case ';':
                state = endReadingValue;

            default:
                if(state == ignoring){
                    state = readingName;
                }else if(state == equal){
                    state = readingValue;
                }

        }

        switch(state){
            
            case readingName:
                if(c != ' ') name += c;
                break;

            case readingValue:
                value += c;
                break;

            case endReadingValue:{
                cssProperty temp;
                temp.name = name;
                temp.value = value;
                temp.inheritable = false;

                if(!selfCssAttributesCache.count(temp.name)){
                    node->style.push_back(temp);
                    cacheDirty = true;
                }else{
                    node->style[selfCssAttributesCache[temp.name]] = temp;
                }
                
                name.clear();
                value.clear();
                state = ignoring;
            }

            default:
                ;

        }
    }
    refreshCache();

    /* Inline style pass end */
    cacheDirty = transferSingleAttributeIntoFour("padding", node->style, selfCssAttributesCache);
    cacheDirty = transferSingleAttributeIntoFour("margin", node->style, selfCssAttributesCache);
    refreshCache();

    node->cssPropertyIndexCache = selfCssAttributesCache;

    /* converting 'padding' and 'margin' into seperate attributes */

    // inheritable flag of properties is set
    for(auto& property : node->style) {
        checkInheritable(property);
    }

    // inherit css for child classes
    for(auto child : node->children){
        inheritCss(child);
    }

}


void addDefaultStyle(std::string name, std::string value){
    cssProperty temp;
    temp.name = name;
    temp.value = value;
}

treeNode::treeNode(std::string input, treeNode* parent){
    name = input;
    parentNode = parent;
}

treeNode::~treeNode(){
    for(auto child : children){
        delete child;
    }
}
