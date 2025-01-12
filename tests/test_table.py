from pytoml11 import Boolean, Integer, String, Table, dumps


def test_init_table():
    table = Table(
        {"int": Integer(42), "bool": Boolean(True), "string": String("hello")}
    )
    assert len(table) == 3
    assert "int" in table
    assert "bool" in table
    assert "string" in table
    assert isinstance(table["int"], Integer)
    assert isinstance(table["bool"], Boolean)
    assert isinstance(table["string"], String)


def test_contains_table():
    table = Table({"key": Integer(42)})
    assert "key" in table
    assert "missing_key" not in table


def test_getitem_table():
    table = Table({"int": Integer(42), "string": String("hello")})
    assert table["int"].value == 42
    assert table["string"].value == "hello"


def test_len_table():
    table = Table(
        {"int": Integer(42), "bool": Boolean(True), "string": String("hello")}
    )
    assert len(table) == 3


def test_copy_table():
    table = Table({"int": Integer(42), "bool": Boolean(True)})
    copy_table = table.copy()
    assert len(copy_table) == len(table)
    assert copy_table["int"].value == table["int"].value
    assert copy_table["bool"].value == table["bool"].value
    assert id(copy_table) != id(table)


def test_value_property_table():
    prev = {"int": Integer(42), "bool": Boolean(True)}
    table = Table(prev)
    value = table.value
    assert len(value) == 2
    assert value["int"] is prev["int"]
    assert value["bool"] is prev["bool"]
    assert value["int"].owned


def test_table_nested_elements():
    nested_table = Table({"nested_key": String("nested_value")})
    table = Table({"int": Integer(42), "nested": nested_table})
    assert isinstance(table["nested"], Table)
    assert table["nested"]["nested_key"].value == "nested_value"


def test_setitem_table():
    table = Table({"int": Integer(42)})
    table["bool"] = Boolean(True)
    assert "bool" in table
    assert table["bool"].value is True


def test_delitem_table():
    prev = {"int": Integer(42), "bool": Boolean(True)}
    table = Table(prev)
    del table["bool"]
    assert "bool" not in table
    assert len(table) == 1
    assert not prev["bool"].owned
    assert prev["int"].owned
    assert prev["bool"].value is True


def test_table_pop():
    table = Table({"int": Integer(42), "bool": Boolean(True)})
    bool_value = table.pop("bool")
    assert isinstance(bool_value, Boolean)
    assert bool_value.value is True
    assert "bool" not in table
    assert len(table) == 1


def test_table_append_with_comments():
    table = Table({"key1": Integer(1)})
    table["key2"] = Integer(2)
    table["key2"].comments = [" Comment on key2"]
    assert table["key2"].comments == [" Comment on key2"]
    assert len(table) == 2


def test_table_update():
    table = Table({"key1": Integer(1), "key2": Boolean(True)})
    updates = {
        "key2": String("updated"),  # Overwriting an existing key
        "key3": Integer(42),  # Adding a new key
        "key4": Boolean(False),  # Adding another new key
    }

    table.update(updates)

    # Verify updated and new keys
    assert len(table) == 4
    assert isinstance(table["key2"], String)
    assert table["key2"].value == "updated"
    assert isinstance(table["key3"], Integer)
    assert table["key3"].value == 42
    assert isinstance(table["key4"], Boolean)
    assert table["key4"].value is False

    # Ensure original key remains unchanged
    assert isinstance(table["key1"], Integer)
    assert table["key1"].value == 1


def test_table_setitem_overwrite():
    table = Table({"key": Integer(1)})
    table["key"] = Integer(2)
    assert table["key"].value == 2
    assert len(table) == 1


def test_table_del_then_set():
    table = Table({"key": Integer(1)})
    del table["key"]
    assert "key" not in table
    table["key"] = Integer(2)
    assert table["key"].value == 2
    assert len(table) == 1


def test_table_initial_order_retained():
    table = Table({
        str(i): Integer(i)
        for i in range(100)
    })

    assert list(table.value.keys()) == [str(i) for i in range(100)]
    assert dumps(table) == "\n".join(
        f"{i} = {i}"
        for i in range(100)
    ) + "\n\n"


def test_table_initial_order_retained_on_setitem():
    table = Table({
        str(i): Integer(i)
        for i in range(100)
    })

    table["50"] = Integer(150)

    assert list(table.value.keys()) == [str(i) for i in range(100)]


def test_table_initial_order_retained_on_update():
    table = Table({
        str(i): Integer(i)
        for i in range(50)
    })

    table.update({
        str(i): Integer(i + 1)
        for i in range(100)
    })

    assert list(table.value.keys()) == [str(i) for i in range(100)]


def test_table_get_value():
    table = Table({"key": Integer(42)})
    assert table.get("key").value == 42
    assert table.get("missing_key") is None
    assert table.get("missing_key", Integer(42)).value == 42
