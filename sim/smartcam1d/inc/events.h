#pragma once

#include <vector>
#include <string>
#include "json.hpp"

namespace sim {

using json = nlohmann::json;

class ObjectBuilder {
public:
    ObjectBuilder(const ::std::string& id, const ::std::string& type)
    : m_json({
        {"action", "object"},
        {"object", {
            {"id", id},
            {"type", type}
        }}
    }) {

    }

    ObjectBuilder& with(const ::std::string& property, json& j) {
        m_json["object"][property] = j;
        return *this;
    }

    ObjectBuilder& origin(double x, double y) {
        m_json["object"]["origin"] = {{"x", x}, {"y", y}};
        return *this;
    }

    ObjectBuilder& rect(double x, double y, double w, double h) {
        m_json["object"]["rect"] = {{"x", x}, {"y", y}, {"w", w}, {"h", h}};
        return *this;
    }

    ObjectBuilder& radius(double r) {
        m_json["object"]["radius"] = r;
        return *this;
    }

    ObjectBuilder& loc(const ::std::string& loc) {
        m_json["object"]["loc"] = loc;
        return *this;
    }

    ObjectBuilder& style(const ::std::string& name) {
        m_json["object"]["style"] = name;
        return *this;
    }

    ObjectBuilder& styles(const ::std::vector<::std::string>& names) {
        m_json["object"]["styles"] = names;
        return *this;
    }

    operator json() { return m_json; }

protected:
    json m_json;
};

class ResetAction {
public:
    ResetAction() : m_json({{"action", "reset"}}) { }
    operator json() { return m_json; }

protected:
    json m_json;
};

class CornerObj : public ObjectBuilder {
public:
    CornerObj(const ::std::string& loc, double x, double y, double size = 10)
    : ObjectBuilder(loc, "corner") {
        this->loc(loc).rect(x, y, size, size);
    }
};

class CameraObj : public ObjectBuilder {
public:
    CameraObj(const ::std::string &id, double x = 0, double y = 0, double radius = 100)
    : ObjectBuilder(id, "camera") {
        origin(x, y).radius(radius);
    }

    CameraObj& angle(double degrees) {
        m_json["object"]["angle"] = degrees;
        return *this;
    }
};

class DotObj : public ObjectBuilder {
public:
    DotObj(const ::std::string &id, double x, double y, double radius = 30)
    : ObjectBuilder(id, "dot") {
        origin(x, y).radius(radius);
    }
};

class RewardObj : public ObjectBuilder {
public:
    RewardObj(const ::std::string &id, double reward)
    : ObjectBuilder(id, "reward") {
        m_json["object"]["reward"] = reward;
        rect(0, 0, 200, 120);
    }
};

class LabelObj : public ObjectBuilder {
public:
    LabelObj(const ::std::string& id, const ::std::string& cnt)
    : ObjectBuilder(id, "label") {
        content(cnt);
    }

    LabelObj& content(const ::std::string& cnt) {
        m_json["object"]["content"] = cnt;
        return *this;
    }
};

}
