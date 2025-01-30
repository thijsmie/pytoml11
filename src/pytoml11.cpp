#include <toml.hpp>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <filesystem>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <variant>
#include <vector>

namespace py = pybind11;

class Item;
class Boolean;
class Integer;
class Float;
class String;
class Table;
class Array;
class Null;
class Date;
class Time;
class DateTime;

typedef std::variant<std::shared_ptr<Boolean>, std::shared_ptr<Integer>, std::shared_ptr<Float>,
                     std::shared_ptr<String>, std::shared_ptr<Table>, std::shared_ptr<Array>,
                     std::shared_ptr<Null>, std::shared_ptr<Date>, std::shared_ptr<Time>,
                     std::shared_ptr<DateTime>>
    AnyItem;

class Key {
  public:
    size_t index;
    std::string key;
    bool is_key;

    Key(size_t index) : index(index), key(), is_key(false) {}
    Key(std::string key) : index(0), key(key), is_key(true) {}
};

typedef std::vector<Key> keypath;

AnyItem to_py_value(std::shared_ptr<toml::ordered_value> root, keypath &path);
Item *cast_anyitem_to_item(AnyItem &item);

toml::ordered_value *resolve(std::shared_ptr<toml::ordered_value> root, keypath &path) {
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

constexpr toml::spec default_spec() {
    toml::spec spec = toml::spec::v(1, 1, 0);
    spec.ext_null_value = true;
    return spec;
}

class Item : public std::enable_shared_from_this<Item> {
  public:
    std::shared_ptr<toml::ordered_value> root;
    keypath path;

    explicit Item(std::shared_ptr<toml::ordered_value> root, keypath &path)
        : root(root), path(path) {}

    explicit Item(std::shared_ptr<toml::ordered_value> root) : root(root), path({}) {}

    bool owned() { return !path.empty(); }

    toml::ordered_value *toml_value() { return resolve(root, path); }

    std::vector<std::string> const get_comments() {
        std::vector<std::string> the_comments;
        std::for_each(toml_value()->comments().begin(), toml_value()->comments().end(),
                      [&](auto &v) { the_comments.push_back(v); });
        return std::move(the_comments);
    }

    void set_comments(std::vector<std::string> the_comments) {
        toml_value()->comments().clear();
        std::for_each(the_comments.begin(), the_comments.end(),
                      [&](auto &v) { toml_value()->comments().push_back(v); });
    }

    virtual void rewrite(std::shared_ptr<toml::ordered_value> new_root, keypath new_path) {
        root = new_root;
        path = new_path;
    }

    virtual ~Item() = default;
    virtual std::string repr() { return "Item()"; };
};

class Boolean : public std::enable_shared_from_this<Boolean>, public Item {
  public:
    using Item::Item;

    const bool value() { return toml_value()->as_boolean(); }
    std::shared_ptr<Boolean> copy() {
        std::shared_ptr<toml::ordered_value> value =
            std::make_shared<toml::ordered_value>(*toml_value());
        return std::make_shared<Boolean>(value);
    }

    static std::shared_ptr<Boolean> from_value(bool value) {
        std::shared_ptr<toml::ordered_value> toml_value =
            std::make_shared<toml::ordered_value>(value);
        return std::make_shared<Boolean>(toml_value);
    }

    std::string repr() { return value() ? "Boolean(True)" : "Boolean(False)"; }
};

class Integer : public std::enable_shared_from_this<Integer>, public Item {
  public:
    using Item::Item;

    const std::int64_t value() { return toml_value()->as_integer(); }
    std::shared_ptr<Integer> copy() {
        std::shared_ptr<toml::ordered_value> value =
            std::make_shared<toml::ordered_value>(*toml_value());
        return std::make_shared<Integer>(value);
    }

    static std::shared_ptr<Integer> from_value(std::int64_t value) {
        std::shared_ptr<toml::ordered_value> toml_value =
            std::make_shared<toml::ordered_value>(value);
        return std::make_shared<Integer>(toml_value);
    }

    std::string repr() { return "Integer(" + std::to_string(value()) + ")"; }
};

class Float : public std::enable_shared_from_this<Float>, public Item {
  public:
    using Item::Item;

    const double value() { return toml_value()->as_floating(); }
    std::shared_ptr<Float> copy() {
        std::shared_ptr<toml::ordered_value> value =
            std::make_shared<toml::ordered_value>(*toml_value());
        return std::make_shared<Float>(value);
    }

    static std::shared_ptr<Float> from_value(double value) {
        std::shared_ptr<toml::ordered_value> toml_value =
            std::make_shared<toml::ordered_value>(value);
        return std::make_shared<Float>(toml_value);
    }

    std::string repr() {
        std::ostringstream oss;
        oss << "Float(" << std::setprecision(8) << std::noshowpoint << value() << ")";
        return oss.str();
    }
};

class String : public std::enable_shared_from_this<String>, public Item {
  public:
    using Item::Item;

    const std::string value() { return toml_value()->as_string(); }
    std::shared_ptr<String> copy() {
        std::shared_ptr<toml::ordered_value> value =
            std::make_shared<toml::ordered_value>(*toml_value());
        return std::make_shared<String>(value);
    }

    static std::shared_ptr<String> from_value(std::string value) {
        std::shared_ptr<toml::ordered_value> toml_value =
            std::make_shared<toml::ordered_value>(value);
        return std::make_shared<String>(toml_value);
    }

    std::string repr() {
        std::string v = value();
        std::string::size_type n = 0;
        while ((n = v.find("\"", n)) != std::string::npos) {
            v.replace(n, 1, "\\\"");
            n += 2;
        }
        return "String(\"" + v + "\")";
    }
};

class Date : public std::enable_shared_from_this<Date>, public Item {
  public:
    using Item::Item;

    py::object value() {
        return py::module::import("datetime")
            .attr("date")(toml_value()->as_local_date().year,
                          1 + toml_value()
                                  ->as_local_date()
                                  .month, // month_t is 0-indexed, python is 1-indexed
                          toml_value()->as_local_date().day);
    }

    std::shared_ptr<Date> copy() {
        std::shared_ptr<toml::ordered_value> value =
            std::make_shared<toml::ordered_value>(*toml_value());
        return std::make_shared<Date>(value);
    }

    static std::shared_ptr<Date> from_value(py::object value) {
        if (!py::isinstance(value, py::module::import("datetime").attr("date"))) {
            throw py::type_error("Value is not a datetime.date object");
        }

        std::shared_ptr<toml::ordered_value> toml_value = std::make_shared<toml::ordered_value>(
            toml::local_date(value.attr("year").cast<int>(),
                             (toml::month_t)(value.attr("month").cast<int>() -
                                             1), // month_t is 0-indexed, python is 1-indexed
                             value.attr("day").cast<int>()));
        return std::make_shared<Date>(toml_value);
    }

    std::string repr() {
        std::ostringstream oss;
        oss << "Date(" << std::to_string(toml_value()->as_local_date().year) << "-" << std::setw(2)
            << std::setfill('0') << std::to_string(toml_value()->as_local_date().month + 1) << "-"
            << std::setw(2) << std::setfill('0')
            << std::to_string(toml_value()->as_local_date().day) << ")";
        return oss.str();
    }
};

class Time : public std::enable_shared_from_this<Time>, public Item {
  public:
    using Item::Item;

    py::object value() {
        return py::module::import("datetime")
            .attr("time")(toml_value()->as_local_time().hour, toml_value()->as_local_time().minute,
                          toml_value()->as_local_time().second,
                          ((uint32_t)toml_value()->as_local_time().millisecond) * 1000 +
                              ((uint32_t)toml_value()->as_local_time().microsecond));
    }

    uint16_t nanoseconds() { return toml_value()->as_local_time().nanosecond; }

    std::shared_ptr<Time> copy() {
        std::shared_ptr<toml::ordered_value> value =
            std::make_shared<toml::ordered_value>(*toml_value());
        return std::make_shared<Time>(value);
    }

    static std::shared_ptr<Time> from_value(py::object value) {
        if (!py::isinstance(value, py::module::import("datetime").attr("time"))) {
            throw py::type_error("Value is not a datetime.time object");
        }

        std::shared_ptr<toml::ordered_value> toml_value =
            std::make_shared<toml::ordered_value>(toml::local_time(
                value.attr("hour").cast<int>(), value.attr("minute").cast<int>(),
                value.attr("second").cast<int>(), value.attr("microsecond").cast<int>() / 1000,
                value.attr("microsecond").cast<int>() % 1000));
        return std::make_shared<Time>(toml_value);
    }

    static std::shared_ptr<Time> from_value_with_nanoseconds(py::object value,
                                                             uint16_t nanoseconds) {
        if (!py::isinstance(value, py::module::import("datetime").attr("time"))) {
            throw py::type_error("Value is not a datetime.time object");
        }

        std::shared_ptr<toml::ordered_value> toml_value =
            std::make_shared<toml::ordered_value>(toml::local_time(
                value.attr("hour").cast<int>(), value.attr("minute").cast<int>(),
                value.attr("second").cast<int>(), value.attr("microsecond").cast<int>() / 1000,
                value.attr("microsecond").cast<int>() % 1000, nanoseconds));
        return std::make_shared<Time>(toml_value);
    }

    std::string repr() {
        std::ostringstream oss;
        oss << "Time(" << toml_value()->as_local_time() << ")";
        return oss.str();
    }
};

class DateTime : public std::enable_shared_from_this<DateTime>, public Item {
  public:
    using Item::Item;

    py::object value() {
        using namespace pybind11::literals;
        py::object datetime_ = py::module_::import("datetime");

        if (toml_value()->is_offset_datetime()) {
            py::object py_offset = datetime_.attr("timedelta")(
                "hours"_a = toml_value()->as_offset_datetime().offset.hour,
                "minutes"_a = toml_value()->as_offset_datetime().offset.minute);
            return datetime_.attr("datetime")(
                toml_value()->as_offset_datetime().date.year,
                toml_value()->as_offset_datetime().date.month +
                    1, // month_t is 0-indexed, python is 1-indexed
                toml_value()->as_offset_datetime().date.day,
                toml_value()->as_offset_datetime().time.hour,
                toml_value()->as_offset_datetime().time.minute,
                toml_value()->as_offset_datetime().time.second,
                ((uint32_t)toml_value()->as_offset_datetime().time.millisecond) * 1000 +
                    ((uint32_t)toml_value()->as_offset_datetime().time.microsecond),
                "tzinfo"_a = datetime_.attr("timezone")(py_offset));
        }
        return datetime_.attr("datetime")(
            toml_value()->as_local_datetime().date.year,
            toml_value()->as_local_datetime().date.month +
                1, // month_t is 0-indexed, python is 1-indexed
            toml_value()->as_local_datetime().date.day, toml_value()->as_local_datetime().time.hour,
            toml_value()->as_local_datetime().time.minute,
            toml_value()->as_local_datetime().time.second,
            ((uint32_t)toml_value()->as_local_datetime().time.millisecond) * 1000 +
                ((uint32_t)toml_value()->as_local_datetime().time.microsecond));
    }

    uint16_t nanoseconds() {
        if (toml_value()->is_offset_datetime()) {
            return toml_value()->as_offset_datetime().time.nanosecond;
        }
        return toml_value()->as_local_datetime().time.nanosecond;
    }

    std::shared_ptr<DateTime> copy() {
        std::shared_ptr<toml::ordered_value> value =
            std::make_shared<toml::ordered_value>(*toml_value());
        return std::make_shared<DateTime>(value);
    }

    static std::shared_ptr<DateTime> from_value(py::object value) {
        py::object datetime_ = py::module_::import("datetime");

        if (!py::isinstance(value, datetime_.attr("datetime"))) {
            throw py::type_error("Value is not a datetime.datetime object");
        }

        if (py::isinstance(value.attr("tzinfo"), datetime_.attr("tzinfo"))) {
            py::object py_offset = value.attr("tzinfo").attr("utcoffset")(value);

            if (py_offset.attr("days").cast<int>() != 0 ||
                py_offset.attr("microseconds").cast<int>() != 0 ||
                py_offset.attr("seconds").cast<int>() % 60 != 0) {
                throw py::value_error("Cannot represent this timezone.");
            }

            std::shared_ptr<toml::ordered_value> toml_value =
                std::make_shared<toml::ordered_value>(toml::offset_datetime(
                    toml::local_date(
                        value.attr("year").cast<int>(),
                        (toml::month_t)(value.attr("month").cast<uint8_t>() -
                                        1), // month_t is 0-indexed, python is 1-indexed
                        value.attr("day").cast<int>()),
                    toml::local_time(value.attr("hour").cast<int>(),
                                     value.attr("minute").cast<int>(),
                                     value.attr("second").cast<int>(),
                                     value.attr("microsecond").cast<int>() / 1000,
                                     value.attr("microsecond").cast<int>() % 1000),
                    toml::time_offset(py_offset.attr("seconds").cast<int>() / 3600,
                                      (py_offset.attr("seconds").cast<int>() / 60) % 60)));
            return std::make_shared<DateTime>(toml_value);
        }

        std::shared_ptr<toml::ordered_value> toml_value =
            std::make_shared<toml::ordered_value>(toml::local_datetime(
                toml::local_date(value.attr("year").cast<int>(),
                                 (toml::month_t)(value.attr("month").cast<uint8_t>() -
                                                 1), // month_t is 0-indexed, python is 1-indexed
                                 value.attr("day").cast<int>()),
                toml::local_time(value.attr("hour").cast<int>(), value.attr("minute").cast<int>(),
                                 value.attr("second").cast<int>(),
                                 value.attr("microsecond").cast<int>() / 1000,
                                 value.attr("microsecond").cast<int>() % 1000)));
        return std::make_shared<DateTime>(toml_value);
    }

    std::string repr() {
        if (toml_value()->is_offset_datetime()) {
            std::ostringstream oss;
            oss << "DateTime(" << toml_value()->as_offset_datetime() << ")";
            return oss.str();
        }

        std::ostringstream oss;
        oss << "DateTime(" << toml_value()->as_local_datetime() << ")";
        return oss.str();
    }
};

class Table : public std::enable_shared_from_this<Table>, public Item {
  protected:
    std::map<std::string, AnyItem> cached_items;

    void ensure_acceptable_formatting() {
        bool contains_non_table_value = false;
        bool has_more_than_one_key = toml_value()->as_table().size() > 1;

        for (auto &kv : toml_value()->as_table()) {
            if (kv.second.type() != toml::value_t::table) {
                contains_non_table_value = true;
                break;
            }
        }

        auto &formatting = toml_value()->as_table_fmt();

        if (formatting.fmt == toml::table_format::implicit && contains_non_table_value) {
            formatting.fmt = toml::table_format::multiline;
        } else if (formatting.fmt == toml::table_format::multiline && !contains_non_table_value) {
            formatting.fmt = toml::table_format::implicit;
        }
    }

  public:
    explicit Table(std::shared_ptr<toml::ordered_value> root, keypath &path)
        : Item(root, path), cached_items() {
            ensure_acceptable_formatting();
        }

    explicit Table(std::shared_ptr<toml::ordered_value> root) : Item(root), cached_items() {
        ensure_acceptable_formatting();
    }

    virtual void rewrite(std::shared_ptr<toml::ordered_value> new_root, keypath new_path) {
        root = new_root;
        path = new_path;

        for (auto &kv : cached_items) {
            auto p = keypath(path);
            p.emplace_back(kv.first);
            cast_anyitem_to_item(kv.second)->rewrite(root, p);
        }
    }

    py::dict value() {
        py::dict result = py::dict();
        for (
            auto it = toml_value()->as_table().begin();
            it != toml_value()->as_table().end();
            ++it
        ) {
            result[py::str(it->first)] = getitem(it->first);
        }
        return result;
    }

    AnyItem getitem(const std::string &key) {
        auto *table = &toml_value()->as_table();
        if (table->find(key) == table->end()) {
            throw py::key_error("Key not found");
        }
        if (cached_items.find(key) != cached_items.end()) {
            return cached_items.at(key);
        }
        auto p = keypath(path);
        p.emplace_back(std::string(key));
        cached_items.insert({key, to_py_value(root, p)});
        return cached_items.at(key);
    }

    void setitem(std::string key, AnyItem item) {
        Item *aitem = cast_anyitem_to_item(item);

        if (aitem->owned()) {
            throw py::type_error("Value is attached, copy first");
        }

        auto *table = &toml_value()->as_table();
        if (table->find(key) != table->end()) {
            // Need swapping approach to avoid messing up the dict order
            auto itt = cached_items.find(key);
            if (itt != cached_items.end()) {
                std::shared_ptr<toml::ordered_value> val =
                    std::make_shared<toml::ordered_value>(table->at(key));
                Item *aitem = cast_anyitem_to_item(itt->second);
                aitem->rewrite(val, keypath({}));
                cached_items.erase(itt);
            }

            toml::ordered_map<std::string, toml::ordered_value> new_table;
            for (auto &kv : *table) {
                if (kv.first != key) {
                    new_table.insert(kv);
                } else {
                    new_table.insert({key, std::move(*aitem->root)});
                }
            }
            /// swap
            table->swap(new_table);
        } else {
            toml_value()->as_table().push_back({key, std::move(*aitem->root)});
        }

        auto p = keypath(path);
        p.emplace_back(key);
        aitem->rewrite(root, p);
        cached_items.insert({key, item});
        ensure_acceptable_formatting();
    }

    void delitem(const std::string &key) {
        auto *table = &toml_value()->as_table();
        if (table->find(key) == table->end()) {
            throw py::key_error("Key not found");
        }

        auto itt = cached_items.find(key);
        if (itt != cached_items.end()) {
            std::shared_ptr<toml::ordered_value> val =
                std::make_shared<toml::ordered_value>(table->at(key));
            Item *aitem = cast_anyitem_to_item(itt->second);
            aitem->rewrite(val, keypath({}));
            cached_items.erase(itt);
        }

        // This function is slightly painful, since erase/remove are not implemented
        // on the ordered_map. We have to pop_back till we find the key, then push
        // the popped values back without the key in question.
        toml::ordered_map<std::string, toml::ordered_value> new_table;
        for (auto &kv : *table) {
            if (kv.first != key) {
                new_table.insert(kv);
            }
        }
        /// swap
        table->swap(new_table);
        ensure_acceptable_formatting();
    }

    AnyItem pop(std::string key) {
        AnyItem item = getitem(key);
        delitem(key);
        return item;
    }

    void update(py::dict values) {
        std::vector<std::pair<std::string, AnyItem>> items;

        for (auto &kv : values) {
            items.push_back({kv.first.cast<std::string>(), kv.second.cast<AnyItem>()});
        }

        for (auto &kv : items) {
            if (cast_anyitem_to_item(kv.second)->owned()) {
                std::ostringstream oss;
                oss << "Cannot update with mapping that contains owned value at key: ";
                oss << kv.first;
                throw py::value_error(oss.str());
            }
        }
        for (auto &kv : items) {
            setitem(kv.first, kv.second);
        }
        ensure_acceptable_formatting();
    }

    size_t size() { return toml_value()->as_table().size(); }

    std::shared_ptr<Table> copy() {
        std::shared_ptr<toml::ordered_value> value =
            std::make_shared<toml::ordered_value>(*toml_value());
        return std::make_shared<Table>(value);
    }

    static std::shared_ptr<Table> from_value(py::dict value) {
        std::vector<std::pair<std::string, AnyItem>> items;
        for (auto &kv : value) {
            items.push_back({kv.first.cast<std::string>(), kv.second.cast<AnyItem>()});
        }

        for (auto &v : items) {
            Item *aitem = cast_anyitem_to_item(v.second);
            if (aitem->owned()) {
                throw py::type_error("Value is attached, copy first");
            }
        }

        std::shared_ptr<Table> table = std::make_shared<Table>(
            std::make_shared<toml::ordered_value>(std::map<std::string, toml::ordered_value>()));

        for (auto v : items) {
            table->setitem(v.first, v.second);
        }

        return table;
    }

    std::string repr() {
        if (size() == 0) {
            return "Table({})";
        }

        std::string result = "Table({";
        for (auto &kv : toml_value()->as_table()) {
            AnyItem item = getitem(kv.first);
            try {
                result += "\"" + kv.first + "\": " + cast_anyitem_to_item(item)->repr() + ", ";
            } catch (const std::exception &e) {
                result += "\"" + kv.first + "\": <repr-error: " + std::string(e.what()) + ">, ";
            }
        }
        return result.substr(0, result.size() - 2) + "})";
    }
};

class Array : public std::enable_shared_from_this<Array>, public Item {
  protected:
    std::map<size_t, AnyItem> cached_items;

    void ensure_acceptable_formatting() {
        bool contains_non_table_value = false;
        for (auto &kv : toml_value()->as_array()) {
            if (kv.type() != toml::value_t::table) {
                contains_non_table_value = true;
                break;
            }
        }

        auto &formatting = toml_value()->as_array_fmt();

        if (formatting.fmt == toml::array_format::array_of_tables && contains_non_table_value) {
            formatting.fmt = toml::array_format::default_format;
        }
    }

  public:
    explicit Array(std::shared_ptr<toml::ordered_value> root, keypath &path)
        : Item(root, path), cached_items() {
            ensure_acceptable_formatting();
        }

    explicit Array(std::shared_ptr<toml::ordered_value> root) : Item(root), cached_items() {
        ensure_acceptable_formatting();
    }

    virtual void rewrite(std::shared_ptr<toml::ordered_value> new_root, keypath new_path) {
        root = new_root;
        path = new_path;

        for (auto &kv : cached_items) {
            auto p = keypath(path);
            p.emplace_back(kv.first);
            cast_anyitem_to_item(kv.second)->rewrite(root, p);
        }
    }

    const std::vector<AnyItem> value() {
        std::vector<AnyItem> result;
        for (size_t i = 0; i < size(); i++) {
            result.push_back(getitem(i));
        }
        return result;
    }

    AnyItem getitem(size_t index) {
        if (index >= size()) {
            throw py::index_error("Index out of range");
        }
        if (cached_items.find(index) != cached_items.end()) {
            return cached_items.at(index);
        }
        auto p = keypath(path);
        p.emplace_back(index);
        cached_items.insert({index, to_py_value(root, p)});
        return cached_items.at(index);
    }

    void append(AnyItem item) {
        Item *aitem = cast_anyitem_to_item(item);
        if (aitem->owned()) {
            throw py::type_error("Value is attached, copy first");
        }

        cached_items.insert({size(), item});
        auto p = keypath(path);
        p.emplace_back(size());
        toml_value()->as_array().emplace_back(*aitem->root);
        aitem->rewrite(root, p);
        ensure_acceptable_formatting();
    }

    void extend(std::vector<AnyItem> values) {
        for (auto &v : values) {
            if (cast_anyitem_to_item(v)->owned()) {
                throw py::value_error("Extending list contains owned value");
            }
        }

        for (auto &v : values)
            append(v);
    }

    void insert(size_t index, AnyItem item) {
        if (index >= size()) {
            throw py::index_error("Index out of range");
        }

        Item *aitem = cast_anyitem_to_item(item);
        if (aitem->owned()) {
            throw py::type_error("Value is attached, copy first");
        }

        // Weird loop eh? But:
        //   Safe when index == 0
        //   Shifts cache up post insert index
        //   Modifies path of contained items.
        for (size_t i = size(); i >= index + 1; --i) {
            auto it = cached_items.find(i - 1);
            if (it == cached_items.end())
                continue;

            auto p = keypath(path);
            p.emplace_back(i);
            cast_anyitem_to_item(it->second)->rewrite(root, p);
            cached_items.insert({i, it->second});
            cached_items.erase(i - 1);
        }

        cached_items.insert({index, item});
        auto p = keypath(path);
        p.emplace_back(index);
        toml_value()->as_array().insert(toml_value()->as_array().begin() + index, *aitem->root);
        aitem->rewrite(root, p);
        ensure_acceptable_formatting();
    }

    void clear() {
        for (size_t i = 0; i < size(); ++i) {
            auto it = cached_items.find(i);
            if (it == cached_items.end())
                continue;

            cast_anyitem_to_item(it->second)->rewrite(
                std::make_shared<toml::ordered_value>(toml_value()->as_array().at(i)),
                {}
            );
        }
        cached_items.clear();
        toml_value()->as_array().clear();
        ensure_acceptable_formatting();
    }

    AnyItem pop(size_t index) {
        if (index >= size()) {
            throw py::index_error("Index out of range");
        }

        auto *vec = &toml_value()->as_array();
        AnyItem ret;

        auto it = cached_items.find(index);
        if (it == cached_items.end()) {
            auto value = std::make_shared<toml::ordered_value>(std::move(vec->at(index)));

            auto p = keypath({});
            ret = to_py_value(value, p);
        } else {
            auto value = std::make_shared<toml::ordered_value>(std::move(vec->at(index)));
            ret = it->second;
            cast_anyitem_to_item(ret)->rewrite(value, {});
            cached_items.erase(index);
        }

        for (size_t i = index + 1; i < size(); ++i) {
            auto it = cached_items.find(i);
            if (it == cached_items.end())
                continue;

            auto p = keypath(path);
            p.emplace_back(i - 1);
            cast_anyitem_to_item(it->second)->rewrite(root, p);
            cached_items.insert({i - 1, it->second});
            cached_items.erase(i);
        }

        vec->erase(vec->begin() + index);
        ensure_acceptable_formatting();
        return ret;
    }

    size_t size() { return toml_value()->as_array().size(); }

    std::shared_ptr<Array> copy() {
        std::shared_ptr<toml::ordered_value> value =
            std::make_shared<toml::ordered_value>(*toml_value());
        return std::make_shared<Array>(value);
    }

    static std::shared_ptr<Array> from_value(std::vector<AnyItem> value) {
        for (auto &v : value) {
            Item *aitem = cast_anyitem_to_item(v);
            if (aitem->owned()) {
                throw py::type_error("Value is attached, copy first");
            }
        }

        std::shared_ptr<Array> array = std::make_shared<Array>(
            std::make_shared<toml::ordered_value>(std::vector<toml::ordered_value>()));

        for (auto v : value) {
            array->append(v);
        }

        return array;
    }

    std::string repr() {
        if (size() == 0) {
            return "Array([])";
        }

        std::string result = "Array([";
        for (auto v : value()) {
            result += cast_anyitem_to_item(v)->repr() + ", ";
        }
        return result.substr(0, result.size() - 2) + "])";
    }
};

class Null : public std::enable_shared_from_this<Null>, public Item {
  public:
    using Item::Item;

    std::shared_ptr<Null> copy() {
        std::shared_ptr<toml::ordered_value> value =
            std::make_shared<toml::ordered_value>(*toml_value());
        return std::make_shared<Null>(value);
    }

    py::none value() { return py::none(); }

    static std::shared_ptr<Null> from_value(py::none) {
        std::shared_ptr<toml::ordered_value> toml_value = std::make_shared<toml::ordered_value>();
        return std::make_shared<Null>(toml_value);
    }

    static std::shared_ptr<Null> from_nothing() {
        std::shared_ptr<toml::ordered_value> toml_value = std::make_shared<toml::ordered_value>();
        return std::make_shared<Null>(toml_value);
    }

    std::string repr() { return "Null()"; }
};

AnyItem to_py_value(std::shared_ptr<toml::ordered_value> root, keypath &path) {
    switch (resolve(root, path)->type()) {
    case toml::value_t::empty:
        return {std::make_shared<Null>(root, path)};
    case toml::value_t::boolean:
        return {std::make_shared<Boolean>(root, path)};
    case toml::value_t::integer:
        return {std::make_shared<Integer>(root, path)};
    case toml::value_t::floating:
        return {std::make_shared<Float>(root, path)};
    case toml::value_t::string:
        return {std::make_shared<String>(root, path)};
    case toml::value_t::offset_datetime:
        return {std::make_shared<DateTime>(root, path)};
    case toml::value_t::local_datetime:
        return {std::make_shared<DateTime>(root, path)};
    case toml::value_t::local_date:
        return {std::make_shared<Date>(root, path)};
    case toml::value_t::local_time:
        return {std::make_shared<Time>(root, path)};
    case toml::value_t::array:
        return {std::make_shared<Array>(root, path)};
    case toml::value_t::table:
        return {std::make_shared<Table>(root, path)};
    default:
        return {std::make_shared<Null>(root, path)};
    }
}

AnyItem load(std::string filename) {
    std::shared_ptr<toml::ordered_value> root = std::make_shared<toml::ordered_value>(
        std::move(toml::parse<toml::ordered_type_config>(filename, default_spec())));

    auto p = keypath({});
    return std::move(to_py_value(root, p));
}

AnyItem loads(std::string data) {
    std::shared_ptr<toml::ordered_value> root = std::make_shared<toml::ordered_value>(
        std::move(toml::parse_str<toml::ordered_type_config>(data, default_spec())));

    auto p = keypath({});
    return std::move(to_py_value(root, p));
}

AnyItem load_from_path(std::filesystem::path path) {
    std::ifstream file(path);
    std::string data((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    return loads(data);
}

void dump(AnyItem item, std::string filename) {
    Item *aitem = cast_anyitem_to_item(item);
    std::ofstream file;
    file.open(filename);
    file << toml::format<toml::ordered_type_config>(*aitem->toml_value(), default_spec());
    file.close();
}

std::string dumps(AnyItem item) {
    Item *aitem = cast_anyitem_to_item(item);
    return toml::format<toml::ordered_type_config>(*aitem->toml_value(), default_spec());
}

void dump_to_path(AnyItem item, std::filesystem::path path) {
    Item *aitem = cast_anyitem_to_item(item);
    std::ofstream file;
    file.open(path);
    file << toml::format<toml::ordered_type_config>(*aitem->toml_value(), default_spec());
    file.close();
}

Item *cast_anyitem_to_item(AnyItem &item) {
    return std::visit([](auto &&arg) -> Item * { return arg.get(); }, item);
}

bool items_equal(AnyItem &a, AnyItem &b) {
    Item *item_a = cast_anyitem_to_item(a);
    Item *item_b = cast_anyitem_to_item(b);
    return *item_a->toml_value() == *item_b->toml_value();
}

PYBIND11_MODULE(_value, m) {
    py::class_<Item, std::shared_ptr<Item>>(m, "Item")
        .def_property("comments", &Item::get_comments, &Item::set_comments)
        .def_property_readonly("owned", &Item::owned)
        .def("__eq__", &items_equal, py::is_operator())
        .def("__repr__", &Item::repr);

    py::class_<Boolean, std::shared_ptr<Boolean>, Item>(m, "Boolean")
        .def(py::init(&Boolean::from_value))
        .def(py::init([](bool value, std::vector<std::string> comments) {
                 std::shared_ptr<Boolean> b = Boolean::from_value(value);
                 b->set_comments(comments);
                 return b;
             }),
             py::arg("value"), py::kw_only(), py::arg("comments"))
        .def_property_readonly("value", &Boolean::value)
        .def("copy", &Boolean::copy);

    py::class_<Integer, std::shared_ptr<Integer>, Item>(m, "Integer")
        .def(py::init(&Integer::from_value))
        .def(py::init([](uint64_t value, std::vector<std::string> comments) {
                 std::shared_ptr<Integer> b = Integer::from_value(value);
                 b->set_comments(comments);
                 return b;
             }),
             py::arg("value"), py::kw_only(), py::arg("comments"))
        .def_property_readonly("value", &Integer::value)
        .def("copy", &Integer::copy);

    py::class_<Float, std::shared_ptr<Float>, Item>(m, "Float")
        .def(py::init(&Float::from_value))
        .def(py::init([](double value, std::vector<std::string> comments) {
                 std::shared_ptr<Float> b = Float::from_value(value);
                 b->set_comments(comments);
                 return b;
             }),
             py::arg("value"), py::kw_only(), py::arg("comments"))
        .def_property_readonly("value", &Float::value)
        .def("copy", &Float::copy);

    py::class_<String, std::shared_ptr<String>, Item>(m, "String")
        .def(py::init(&String::from_value))
        .def(py::init([](std::string value, std::vector<std::string> comments) {
                 std::shared_ptr<String> b = String::from_value(value);
                 b->set_comments(comments);
                 return b;
             }),
             py::arg("value"), py::kw_only(), py::arg("comments"))
        .def_property_readonly("value", &String::value)
        .def("copy", &String::copy);

    py::class_<Table, std::shared_ptr<Table>, Item>(m, "Table")
        .def(py::init(&Table::from_value))
        .def(py::init([](py::dict value, std::vector<std::string> comments) {
                 std::shared_ptr<Table> b = Table::from_value(value);
                 b->set_comments(comments);
                 return b;
             }),
             py::arg("value"), py::kw_only(), py::arg("comments"))
        .def_property_readonly("value", &Table::value)
        .def("__getitem__", &Table::getitem)
        .def("__setitem__", &Table::setitem)
        .def("__delitem__", &Table::delitem)
        .def("update", &Table::update)
        .def("copy", &Table::copy)
        .def("pop", &Table::pop)
        .def("get", [](std::shared_ptr<Table> table, std::string key) -> std::variant<py::none, AnyItem> {
            if (table->toml_value()->as_table().find(key) == table->toml_value()->as_table().end()) {
                return py::none();
            }
            return table->getitem(key);
        })
        .def("get", [](std::shared_ptr<Table> table, std::string key, py::object default_value) -> std::variant<py::object, AnyItem> {
            if (table->toml_value()->as_table().find(key) == table->toml_value()->as_table().end()) {
                return default_value;
            }
            return table->getitem(key);
        })
        .def("__len__", &Table::size)
        .def("__contains__", [](std::shared_ptr<Table> table, std::string key) {
            auto *tab = &table->toml_value()->as_table();
            return (tab->find(key) != tab->end());
        });

    py::class_<Array, std::shared_ptr<Array>, Item>(m, "Array")
        .def(py::init(&Array::from_value))
        .def(py::init([](std::vector<AnyItem> value, std::vector<std::string> comments) {
                 std::shared_ptr<Array> b = Array::from_value(value);
                 b->set_comments(comments);
                 return b;
             }),
             py::arg("value"), py::kw_only(), py::arg("comments"))
        .def_property_readonly("value", &Array::value)
        .def("copy", &Array::copy)
        .def("__len__", &Array::size)
        .def("__getitem__", &Array::getitem)
        .def("append", &Array::append)
        .def("extend", &Array::extend)
        .def("insert", &Array::insert)
        .def("clear", &Array::clear)
        .def("__setitem__", &Array::insert)
        .def("__delitem__", &Array::pop)
        .def("pop", &Array::pop)
        .def("__contains__", [](std::shared_ptr<Array> array, AnyItem item) {
            for (size_t i = 0; i < array->size(); i++) {
                AnyItem aitem = array->getitem(i);
                if (items_equal(aitem, item)) {
                    return true;
                }
            }
            return false;
        });

    py::class_<Null, std::shared_ptr<Null>, Item>(m, "Null")
        .def(py::init(&Null::from_value))
        .def(py::init([](py::none value, std::vector<std::string> comments) {
                 std::shared_ptr<Null> b = Null::from_value(value);
                 b->set_comments(comments);
                 return b;
             }),
             py::arg("value"), py::kw_only(), py::arg("comments"))
        .def(py::init(&Null::from_nothing))
        .def(py::init([](std::vector<std::string> comments) {
                 std::shared_ptr<Null> b = Null::from_nothing();
                 b->set_comments(comments);
                 return b;
             }),
             py::kw_only(), py::arg("comments"))
        .def_property_readonly("value", &Null::value)
        .def("copy", &Null::copy);

    py::class_<Date, std::shared_ptr<Date>, Item>(m, "Date")
        .def(py::init(&Date::from_value))
        .def(py::init([](py::object value, std::vector<std::string> comments) {
                 std::shared_ptr<Date> b = Date::from_value(value);
                 b->set_comments(comments);
                 return b;
             }),
             py::arg("value"), py::kw_only(), py::arg("comments"))
        .def_property_readonly("value", &Date::value)
        .def("copy", &Date::copy);

    py::class_<Time, std::shared_ptr<Time>, Item>(m, "Time")
        .def(py::init(&Time::from_value))
        .def(py::init(&Time::from_value_with_nanoseconds))
        .def(py::init([](py::object value, std::vector<std::string> comments) {
                 std::shared_ptr<Time> b = Time::from_value(value);
                 b->set_comments(comments);
                 return b;
             }),
             py::arg("value"), py::kw_only(), py::arg("comments"))
        .def(
            py::init([](py::object value, uint16_t nanoseconds, std::vector<std::string> comments) {
                std::shared_ptr<Time> b = Time::from_value_with_nanoseconds(value, nanoseconds);
                b->set_comments(comments);
                return b;
            }),
            py::arg("value"), py::arg("nanoseconds"), py::kw_only(), py::arg("comments"))
        .def_property_readonly("value", &Time::value)
        .def_property_readonly("nanoseconds", &Time::nanoseconds)
        .def("copy", &Time::copy);

    py::class_<DateTime, std::shared_ptr<DateTime>, Item>(m, "DateTime")
        .def(py::init(&DateTime::from_value))
        .def(py::init([](py::object value, std::vector<std::string> comments) {
                 std::shared_ptr<DateTime> b = DateTime::from_value(value);
                 b->set_comments(comments);
                 return b;
             }),
             py::arg("value"), py::kw_only(), py::arg("comments"))
        .def_property_readonly("value", &DateTime::value)
        .def_property_readonly("nanoseconds", &DateTime::nanoseconds)
        .def("copy", &DateTime::copy);

    m.def("load", &load);
    m.def("load", &load_from_path);
    m.def("loads", &loads);
    m.def("dump", &dump);
    m.def("dump", &dump_to_path);
    m.def("dumps", &dumps);

    py::register_exception<toml::exception>(m, "TomlError");
}