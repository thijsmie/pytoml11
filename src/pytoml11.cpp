#include <toml.hpp>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <iostream>
#include <vector>
#include <map>
#include <variant>
#include <string>
#include <memory>

namespace py = pybind11;

class Item;
class Boolean;
class Integer;
class Float;
class String;
class Table;
class Array;
class None;

typedef std::variant<
    std::shared_ptr<Boolean>,
    std::shared_ptr<Integer>,
    std::shared_ptr<Float>,
    std::shared_ptr<String>,
    std::shared_ptr<Table>,
    std::shared_ptr<Array>,
    std::shared_ptr<None>
> AnyItem;

class Key {
  public:
    size_t index;
    std::string key;
    bool is_key;

    Key(size_t index) : index(index), key(), is_key(false) {}
    Key(std::string key) : index(0), key(key), is_key(true) {}
};

typedef std::vector<Key> keypath;

toml::ordered_value from_py_value(py::object obj);
AnyItem to_py_value(
    std::shared_ptr<toml::ordered_value> root,
    keypath &path
);
Item* _cv_anyitem(AnyItem &item);

toml::ordered_value* resolve(std::shared_ptr<toml::ordered_value> root, keypath &path) {
    toml::ordered_value *v = root.get();
    for (auto &key : path) {
        if (key.is_key) {
            v = &v->as_table().at(key.key);
        } else {
            v = &v->as_array().at(key.index);
        }
    }
    return v;
}

constexpr toml::spec default_spec () {
    toml::spec spec = toml::spec::v(1,1,0);
    spec.ext_null_value = true;
    return spec;
}

class Item : public std::enable_shared_from_this<Item> {
  public:
    std::shared_ptr<toml::ordered_value> root;
    keypath path;

    explicit Item(
        std::shared_ptr<toml::ordered_value> root,
        keypath &path
    ) : root(root), path(path) {}

    explicit Item(
        std::shared_ptr<toml::ordered_value> root
    ) : root(root), path({}) {}

    bool owned() {
        return !path.empty();
    }

    toml::ordered_value *toml_value() {
        return resolve(root, path);
    }

    std::vector<std::string> const get_comments() {
        std::vector<std::string> the_comments;
        std::for_each(
            toml_value()->comments().begin(),
            toml_value()->comments().end(),
            [&](auto &v) {the_comments.push_back(v); }
        );
        return std::move(the_comments);
    }

    void set_comments(std::vector<std::string> the_comments) {
        toml_value()->comments().clear();
        std::for_each(
            the_comments.begin(),
            the_comments.end(),
            [&](auto &v) {toml_value()->comments().push_back(v); }
        );
    }

    virtual ~Item() = default;
    virtual std::string repr() { return "Item()"; };
};

class Boolean : public std::enable_shared_from_this<Boolean>, public Item {
  public:
    using Item::Item;

    const bool value() { return toml_value()->as_boolean(); }
    std::shared_ptr<Boolean> copy() {
        std::shared_ptr<toml::ordered_value> value = std::make_shared<toml::ordered_value>(*toml_value());
        return std::make_shared<Boolean>(value);
    }

    static std::shared_ptr<Boolean> from_value(bool value) {
        std::shared_ptr<toml::ordered_value> toml_value = std::make_shared<toml::ordered_value>(value);
        return std::make_shared<Boolean>(toml_value);
    }

    std::string repr() {
        return value() ? "Bool(True)" : "Bool(False)";
    }
};

class Integer : public std::enable_shared_from_this<Integer>, public Item {
  public:
    using Item::Item;

    const std::int64_t value() { return toml_value()->as_integer(); }
    std::shared_ptr<Integer> copy() {
        std::shared_ptr<toml::ordered_value> value = std::make_shared<toml::ordered_value>(*toml_value());
        return std::make_shared<Integer>(value);
    }

    static std::shared_ptr<Integer> from_value(std::int64_t value) {
        std::shared_ptr<toml::ordered_value> toml_value = std::make_shared<toml::ordered_value>(value);
        return std::make_shared<Integer>(toml_value);
    }
};

class Float : public std::enable_shared_from_this<Float>, public Item {
  public:
    using Item::Item;

    const double value() { return toml_value()->as_floating(); }
    std::shared_ptr<Float> copy() {
        std::shared_ptr<toml::ordered_value> value = std::make_shared<toml::ordered_value>(*toml_value());
        return std::make_shared<Float>(value);
    }

    static std::shared_ptr<Float> from_value(double value) {
        std::shared_ptr<toml::ordered_value> toml_value = std::make_shared<toml::ordered_value>(value);
        return std::make_shared<Float>(toml_value);
    }

    std::string repr() {
        return "Float(" + std::to_string(value()) + ")";
    }
};

class String : public std::enable_shared_from_this<String>, public Item {
  public:
    using Item::Item;

    const std::string value() { return toml_value()->as_string(); }
    std::shared_ptr<String> copy() {
        std::shared_ptr<toml::ordered_value> value = std::make_shared<toml::ordered_value>(*toml_value());
        return std::make_shared<String>(value);
    }

    static std::shared_ptr<String> from_value(std::string value) {
        std::shared_ptr<toml::ordered_value> toml_value = std::make_shared<toml::ordered_value>(value);
        return std::make_shared<String>(toml_value);
    }

    std::string repr() {
        std::string v = value();
        std::string::size_type n = 0;
        while ( ( n = v.find( "\"", n ) ) != std::string::npos )
        {
            v.replace( n, 1, "\\\"" );
            n += 2;
        }
        return "String(\"" + v + "\")";
    }
};

class Table : public std::enable_shared_from_this<Table>, public Item {
  public:
    using Item::Item;

    std::map<std::string, AnyItem> value() {
        std::map<std::string, AnyItem> result;
        for (auto &v : toml_value()->as_table()) {
            auto p = keypath(path);
            p.emplace_back(std::string(v.first));
            result[v.first] = to_py_value(root, p);
        }
        return result;
    }

    AnyItem getitem(const std::string &key) {
        if (!toml_value()->contains(key)) {
            throw py::key_error("Key not found");
        }
        auto p = keypath(path);
        p.emplace_back(std::string(key));
        return std::move(to_py_value(root, p));
    }

    void setitem(std::string key, AnyItem item) {
        Item *aitem = _cv_anyitem(item);
        if (aitem->owned()) {
            throw py::type_error("Value is attached, copy first");
        }
        if (toml_value()->contains(key)) {
            delitem(key);
        }
        toml_value()->as_table().insert({key, std::move(*aitem->root)});
        aitem->path = keypath(path);
        aitem->path.emplace_back(key);
        aitem->root = root;
    }

    void delitem(const std::string &key) {
        if (!toml_value()->contains(key)) {
            throw py::key_error("Key not found");
        }
        // This function is slightly painful, since erase/remove are not implemented
        // on the ordered_map. We have to pop_back till we find the key, then push the
        // popped values back without the key in question.
        auto &vec = toml_value()->as_table();
        std::vector<std::pair<std::string, toml::ordered_value>> popped;
        auto it = vec.end();
        while (it != vec.begin()) {
            it--;
            if (it->first == key) {
                break;
            }
            popped.push_back(*it);
            vec.pop_back();
        }
        vec.pop_back();
        for (auto &v : popped) {
            vec.insert(v);
        }
    }

    size_t size() {
        return toml_value()->as_table().size();
    }

    std::shared_ptr<Table> copy() {
        std::shared_ptr<toml::ordered_value> value = std::make_shared<toml::ordered_value>(*toml_value());
        return std::make_shared<Table>(value);
    }

    static std::shared_ptr<Table> from_value(std::map<std::string, AnyItem> value) {
        std::shared_ptr<toml::ordered_value> toml_value = std::make_shared<toml::ordered_value>(std::map<std::string, toml::ordered_value>());
        for (auto &v : value) {
            Item *aitem = _cv_anyitem(v.second);
            if (aitem->owned()) {
                throw py::type_error("Value is attached, copy first");
            }
        }
        for (auto &v : value) {
            Item *aitem = _cv_anyitem(v.second);
            toml_value->as_table().insert({v.first, toml::ordered_value(*aitem->root)});
            aitem->path = keypath({Key(v.first)});
            aitem->root = toml_value;
        }
        return std::make_shared<Table>(toml_value);
    }

    std::string repr() {
        if (size() == 0) {
            return "Table({})";
        }

        std::string result = "Table({";
        for (auto &kv : value()) {
            result += "\"" + kv.first + "\": " + _cv_anyitem(kv.second)->repr() + ", ";
        }
        return result.substr(0, result.size() - 2) + "})";
    }
};

class Array : public std::enable_shared_from_this<Array>, public Item {
  public:
    using Item::Item;

    const std::vector<AnyItem> value() {
        std::vector<AnyItem> result;
        auto &value = toml_value()->as_array();
        for (size_t i = 0; i < value.size(); i++) {
            auto v = value.at(i);
            auto p = keypath(path);
            p.emplace_back(i);
            result.push_back(std::move(to_py_value(root, p)));
        }
        return result;
    }

    void append(AnyItem item) {
        Item *aitem = _cv_anyitem(item);
        if (aitem->owned()) {
            throw py::type_error("Value is attached, copy first");
        }
        auto vec = toml_value()->as_array();
        vec.push_back(std::move(*aitem->root));
        aitem->path = keypath(path);
        aitem->path.emplace_back(vec.size() - 1);
        aitem->root = root;
    }

    void insert(size_t index, AnyItem item) {
        Item *aitem = _cv_anyitem(item);
        if (aitem->owned()) {
            throw py::type_error("Value is attached, copy first");
        }
        auto vec = toml_value()->as_array();
        vec.insert(vec.begin() + index, std::move(*aitem->root));
        aitem->path = keypath(path);
        aitem->path.emplace_back(index);
        aitem->root = root;
    }

    AnyItem pop(size_t index) {
        auto vec = toml_value()->as_array();
        auto value = std::make_shared<toml::ordered_value>(vec.at(index));
        vec.erase(vec.begin() + index);

        auto p = keypath(path);
        return std::move(to_py_value(value, p));
    }

    AnyItem getitem(size_t index) {
        auto vec = toml_value()->as_array();
        if (index >= vec.size()) {
            throw py::index_error("Index out of range");
        }
        auto p = keypath(path);
        p.emplace_back(index);
        return std::move(to_py_value(root, p));
    }

    size_t size() {
        return toml_value()->as_array().size();
    }

    std::shared_ptr<Array> copy() {
        std::shared_ptr<toml::ordered_value> value = std::make_shared<toml::ordered_value>(*toml_value());
        return std::make_shared<Array>(value);
    }

    static std::shared_ptr<Array> from_value(std::vector<AnyItem> value) {
        std::shared_ptr<toml::ordered_value> toml_value = std::make_shared<toml::ordered_value>(std::vector<toml::ordered_value>());
        for (auto &v : value) {
            Item *aitem = _cv_anyitem(v);
            if (aitem->owned()) {
                throw py::type_error("Value is attached, copy first");
            }
        }
        for (size_t i = 0; i < value.size(); i++) {
            auto v = _cv_anyitem(value.at(i));
            toml_value->as_array().push_back(std::move(*v->root));
            v->path = keypath({Key(i)});
            v->root = toml_value;
        }
        return std::make_shared<Array>(toml_value);
    }

    std::string repr() {
        if (size() == 0) {
            return "Array([])";
        }

        std::string result = "Array([";
        for (auto v : value()) {
            result += _cv_anyitem(v)->repr() + ", ";
        }
        return result.substr(0, result.size() - 2) + "])";
    }
};

class None : public std::enable_shared_from_this<None>, public Item {
  public:
    using Item::Item;

    std::shared_ptr<None> copy() {
        std::shared_ptr<toml::ordered_value> value = std::make_shared<toml::ordered_value>(*toml_value());
        return std::make_shared<None>(value);
    }

    py::none value() { return py::none(); }

    static std::shared_ptr<None> from_value(py::none) {
        std::shared_ptr<toml::ordered_value> toml_value = std::make_shared<toml::ordered_value>();
        return std::make_shared<None>(toml_value);
    }

    std::string repr() {
        return "None_(None)";
    }
};

AnyItem to_py_value(
    std::shared_ptr<toml::ordered_value> root,
    keypath &path
) {
    switch(resolve(root, path)->type())
    {
        case toml::value_t::empty: return {std::make_shared<None>(root, path)};
        case toml::value_t::boolean: return {std::make_shared<Boolean>(root, path)};
        case toml::value_t::integer: return {std::make_shared<Integer>(root, path)};
        case toml::value_t::floating: return {std::make_shared<Float>(root, path)};
        case toml::value_t::string: return {std::make_shared<String>(root, path)};
        case toml::value_t::offset_datetime: return {std::make_shared<None>(root, path)};
        case toml::value_t::local_datetime: return {std::make_shared<None>(root, path)};
        case toml::value_t::local_date: return {std::make_shared<None>(root, path)};
        case toml::value_t::local_time: return {std::make_shared<None>(root, path)};
        case toml::value_t::array: return {std::make_shared<Array>(root, path)};
        case toml::value_t::table: return {std::make_shared<Table>(root, path)};
        default: return {std::make_shared<None>(root, path)};
    }
}

AnyItem load (std::string filename) {
    std::shared_ptr<toml::ordered_value> root = std::make_shared<toml::ordered_value>(
        std::move(toml::parse<toml::ordered_type_config>(filename, default_spec()))
    );

    auto p = keypath({});
    return std::move(to_py_value(root, p));
}

void dump (AnyItem item, std::string filename) {
    Item *aitem = _cv_anyitem(item);
    std::ofstream file;
    file.open(filename);
    file << toml::format<toml::ordered_type_config>(*aitem->toml_value(), default_spec());
    file.close();
}

toml::ordered_value from_py_value(py::object obj) {
    if (py::isinstance<Item>(obj)) {
        return toml::ordered_value(*(obj.cast<Item>()).toml_value());
    }
    else if (py::isinstance<py::none>(obj)) {
        return toml::ordered_value();
    }
    else if (py::isinstance<py::bool_>(obj)) {
        return toml::ordered_value(obj.cast<bool>());
    } else if (py::isinstance<py::int_>(obj)) {
        return toml::ordered_value(obj.cast<std::int64_t>());
    } else if (py::isinstance<py::str>(obj)) {
        return toml::ordered_value(obj.cast<std::string>());
    } else if (py::isinstance<py::list>(obj)) {
        py::list entries = obj.cast<py::list>();
        std::vector<toml::ordered_value> converted;
        for (auto &entry : entries) {
            converted.push_back(
                std::move(from_py_value(entry.cast<py::object>()))
            );
        }
        return toml::ordered_value(std::move(converted));
    } else if (py::isinstance<py::dict>(obj)) {
        py::dict entries = obj.cast<py::dict>();
        std::map<std::string, toml::ordered_value> converted;
        for (auto &entry : entries) {
            if (!py::isinstance<py::str>(entry.first))
                throw new pybind11::type_error("Dict key is not a string");
            converted.insert({
                entry.first.cast<std::string>(),
                std::move(from_py_value(entry.second.cast<py::object>()))
            });
        }

        return toml::ordered_value(std::move(converted));
    }

    throw new pybind11::type_error("Could not be mapped to toml value.");
}

Item* _cv_anyitem(AnyItem &item) {
    return std::visit([](auto&& arg) -> Item* { return arg.get(); }, item);
}

PYBIND11_MODULE(_value, m) {
    py::class_<Item, std::shared_ptr<Item>>(m, "Item")
        .def("get_comments", &Item::get_comments)
        .def("set_comments", &Item::set_comments)
        .def("owned", &Item::owned)
        .def("__repr__", &Item::repr);

    py::class_<Boolean, std::shared_ptr<Boolean>, Item>(m, "Boolean")
        .def(py::init(&Boolean::from_value))
        .def("value", &Boolean::value)
        .def("copy", &Boolean::copy);

    py::class_<Integer, std::shared_ptr<Integer>, Item>(m, "Integer")
        .def(py::init(&Integer::from_value))
        .def("value", &Integer::value)
        .def("copy", &Integer::copy);

    py::class_<Float, std::shared_ptr<Float>, Item>(m, "Float")
        .def(py::init(&Float::from_value))
        .def("value", &Float::value)
        .def("copy", &Float::copy);

    py::class_<String, std::shared_ptr<String>, Item>(m, "String")
        .def(py::init(&String::from_value))
        .def("value", &String::value)
        .def("copy", &String::copy);

    py::class_<Table, std::shared_ptr<Table>, Item>(m, "Table")
        .def(py::init(&Table::from_value))
        .def("value", &Table::value)
        .def("__getitem__", &Table::getitem)
        .def("__setitem__", &Table::setitem)
        .def("__delitem__", &Table::delitem)
        .def("copy", &Table::copy)
        .def("__len__", &Table::size)
        .def("__contains__", [](std::shared_ptr<Table> table, std::string key) {
            return table->toml_value()->contains(key);
        });

    py::class_<Array, std::shared_ptr<Array>, Item>(m, "Array")
        .def(py::init(&Array::from_value))
        .def("value", &Array::value)
        .def("append", &Array::append)
        .def("insert", &Array::insert)
        .def("copy", &Array::copy)
        .def("__len__", &Array::size)
        .def("__getitem__", &Array::getitem)
        .def("__setitem__", &Array::insert)
        .def("__delitem__", &Array::pop)
        .def("pop", &Array::pop)
        .def("__iter__", [](std::shared_ptr<Array> array) {
            auto items = array->value();
            return py::make_iterator(items.begin(), items.end());
        }, py::keep_alive<0, 1>());

    py::class_<None, std::shared_ptr<None>, Item>(m, "None_")
        .def(py::init(&None::from_value))
        .def("value", &None::value)
        .def("copy", &None::copy);

    m.def("load", &load);
    m.def("dump", &dump);

    py::register_exception<toml::exception>(m, "TomlError");
}