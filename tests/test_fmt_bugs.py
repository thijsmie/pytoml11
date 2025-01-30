from pytoml11 import loads, dumps, Boolean

# In versions prior to 0.0.5 the following tests would fail with segfaults.
# This is due to toml.hpp not resetting formatting state after a table or array
# has been edited. Now we check the formatting on every edit.

def test_array_unset_aot(snapshot):
    start = loads("""
    [[array]]
    a = 0
    [[array]]
    b = 1
    """)
    start["array"].append(Boolean(True))
    assert dumps(start) == snapshot


def test_table_no_implicit_empty(snapshot):
    start = loads("""
    [table]
    a = 0
    [table.b]
    c = 1
    """)
    del start["table"]["a"]
    assert dumps(start) == snapshot