#include "TmxLoader.h"
#include <pugixml.hpp>
#include <stdexcept>
#include <sstream>

TmxMapData TmxLoader::load(const std::string& filepath) {
    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_file(filepath.c_str());

    if (!result) {
        throw std::runtime_error(
            "TmxLoader: failed to parse '" + filepath + "': " + result.description());
    }

    pugi::xml_node mapNode = doc.child("map");
    if (!mapNode) {
        throw std::runtime_error("TmxLoader: no <map> element in '" + filepath + "'");
    }

    TmxMapData data;
    data.mapWidth  = mapNode.attribute("width").as_int();
    data.mapHeight = mapNode.attribute("height").as_int();
    data.tileWidth = mapNode.attribute("tilewidth").as_int();
    data.tileHeight = mapNode.attribute("tileheight").as_int();

    // Parse tilesets
    for (pugi::xml_node tsNode : mapNode.children("tileset")) {
        TmxTileset ts;
        ts.firstGid   = tsNode.attribute("firstgid").as_int();
        ts.tileWidth  = tsNode.attribute("tilewidth").as_int(data.tileWidth);
        ts.tileHeight = tsNode.attribute("tileheight").as_int(data.tileHeight);
        ts.tileCount  = tsNode.attribute("tilecount").as_int();
        ts.columns    = tsNode.attribute("columns").as_int();

        // Image source can be in an <image> child or from an external .tsx
        pugi::xml_node imageNode = tsNode.child("image");
        if (imageNode) {
            ts.imageSource = imageNode.attribute("source").as_string();
        }

        // If tileset references an external .tsx file, parse it
        pugi::xml_attribute sourceAttr = tsNode.attribute("source");
        if (sourceAttr) {
            // Extract directory from the .tmx filepath for relative path resolution
            std::string dir;
            size_t lastSlash = filepath.find_last_of("/\\");
            if (lastSlash != std::string::npos) {
                dir = filepath.substr(0, lastSlash + 1);
            }

            std::string tsxPath = dir + sourceAttr.as_string();
            pugi::xml_document tsxDoc;
            pugi::xml_parse_result tsxResult = tsxDoc.load_file(tsxPath.c_str());
            if (tsxResult) {
                pugi::xml_node tsxRoot = tsxDoc.child("tileset");
                if (tsxRoot) {
                    ts.tileWidth  = tsxRoot.attribute("tilewidth").as_int(ts.tileWidth);
                    ts.tileHeight = tsxRoot.attribute("tileheight").as_int(ts.tileHeight);
                    ts.tileCount  = tsxRoot.attribute("tilecount").as_int(ts.tileCount);
                    ts.columns    = tsxRoot.attribute("columns").as_int(ts.columns);
                    pugi::xml_node tsxImage = tsxRoot.child("image");
                    if (tsxImage) {
                        ts.imageSource = tsxImage.attribute("source").as_string();
                    }
                }
            }
        }

        data.tilesets.push_back(ts);
    }

    // Parse tile layers
    for (pugi::xml_node layerNode : mapNode.children("layer")) {
        TmxLayer layer;
        layer.name   = layerNode.attribute("name").as_string();
        layer.width  = layerNode.attribute("width").as_int();
        layer.height = layerNode.attribute("height").as_int();

        pugi::xml_node dataNode = layerNode.child("data");
        if (dataNode) {
            std::string encoding = dataNode.attribute("encoding").as_string();
            if (encoding == "csv") {
                layer.data = parseCSV(dataNode.child_value());
            } else {
                throw std::runtime_error(
                    "TmxLoader: unsupported encoding '" + encoding
                    + "' in layer '" + layer.name + "'. Use CSV.");
            }
        }

        data.layers.push_back(layer);
    }

    // Parse object groups
    for (pugi::xml_node groupNode : mapNode.children("objectgroup")) {
        for (pugi::xml_node objNode : groupNode.children("object")) {
            TmxObject obj;
            obj.name   = objNode.attribute("name").as_string();
            obj.type   = objNode.attribute("type").as_string();
            obj.x      = objNode.attribute("x").as_float();
            obj.y      = objNode.attribute("y").as_float();
            obj.width  = objNode.attribute("width").as_float();
            obj.height = objNode.attribute("height").as_float();
            data.objects.push_back(obj);
        }
    }

    return data;
}

std::vector<int> TmxLoader::parseCSV(const std::string& csv) {
    std::vector<int> result;
    std::stringstream ss(csv);
    std::string token;

    while (std::getline(ss, token, ',')) {
        // Trim whitespace/newlines
        size_t start = token.find_first_not_of(" \t\r\n");
        if (start == std::string::npos) continue;
        size_t end = token.find_last_not_of(" \t\r\n");
        std::string trimmed = token.substr(start, end - start + 1);

        if (!trimmed.empty()) {
            result.push_back(std::stoi(trimmed));
        }
    }

    return result;
}
