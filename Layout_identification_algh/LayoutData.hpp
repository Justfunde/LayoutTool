#pragma once

#include <string>
#include <vector>

struct Units {
    double  user,
        physical;
};

struct Coord {
    int32_t x,
        y;
};

struct Property {
    int16_t     index;
    std::string value;
};

enum class GeometryType {
    undefined = 0,
    polygon,
    path,
    text,
    rectangle,
    reference,
};

struct Geometry {
    GeometryType          type;
    int16_t               layer;
    std::vector<Property> properties;
    std::vector<Coord>    coords;
    Coord                 min,
        max;
public:
    Geometry(GeometryType t) : type(t), layer(-1), min({ -1, -1 }), max({ -1, -1 }) {}
};

struct Polygon : public Geometry {
    int16_t               dataType;
public:
    Polygon() : Geometry(GeometryType::polygon), dataType(0) {}
};

struct Path : public Geometry {
    int16_t               dataType,
        pathType;
    int32_t               width;
public:
    Path() : Geometry(GeometryType::path), dataType(0), pathType(0), width(0) {}
};

struct Text : public Geometry {
    int16_t                textType,
        flagsPresentation,
        pathType,
        flagsTransformation;
    double                 magnification;
    int32_t                width;
    std::string            stringValue;
public:
    Text() : Geometry(GeometryType::text), textType(0), flagsPresentation(0), pathType(0), flagsTransformation(0), magnification(1.0), width(0) {}
};

struct Rectangle : public Geometry {
    int16_t                rectType;
public:
    Rectangle() : Geometry(GeometryType::rectangle), rectType(0) {}
};

struct Element;

struct Reference : public Geometry {
    std::string             name;
    Element* pElement;
    int32_t                 transformationFlags;
    double                  magnification;
public:
    Reference() : Geometry(GeometryType::reference), name(""), pElement(nullptr), transformationFlags(0), magnification(1.0) {}
};

struct Element {
    std::string             name;
    std::vector<Geometry*> geometries;
    bool                    isFlat;
    Coord                   min,
        max;
public:
    Element() : isFlat(true), min({ -1, -1 }), max({ -1, -1 }) {}
    ~Element() {
        for (size_t i = 0; i < geometries.size(); ++i) {
            delete geometries[i];
            geometries[i] = nullptr;
        }
        geometries.clear();
    }
};

struct Layer {
    int16_t                 layer;
    std::string             name;
    std::vector<Geometry*> geometries;
};

struct Library {
    std::string             name;
    Units                   units;
    std::vector<Element*>  elements;
    std::vector<Layer>      layers;
public:
    Library() {
        units.user = 0.001;
        units.physical = 1e-9;
    }
    ~Library() {
        for (size_t i = 0; i < elements.size(); ++i) {
            delete elements[i];
            elements[i] = nullptr;
        }
        elements.clear();
        layers.clear();
    }
};

enum class LayoutFileFormat {
    undefined,
    GDSII_bin,
    GDSII_ascii,
    MSK,
    CIF,
    DXF,
    OASIS,
    OpenAccess,
};

struct LayoutData {
    std::wstring             fileName;
    std::vector<Library*>  libraries;
    LayoutFileFormat        fileFormat;
public:
   LayoutData():fileFormat(LayoutFileFormat::undefined) {}
    ~LayoutData() {
        for (size_t i = 0; i < libraries.size(); ++i) {
            delete libraries[i];
            libraries[i] = nullptr;
        }
        libraries.clear();
    }
};