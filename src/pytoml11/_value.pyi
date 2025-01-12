from __future__ import annotations

import typing
from datetime import date, datetime, time
from os import PathLike
from pathlib import Path

class Item:
    """Base class for all TOML values. Cannot be instantiated."""

    comments: list[str]
    """
    List of comments associated with the value.
    Returns a copy, explicitly set if you want to modify it.
    """

    def __repr__(self) -> str: ...
    @property
    def owned(self) -> bool:
        """Whether the value is owned by the parent table or array."""

class Array(Item):
    """Array of TOML values."""

    def __delitem__(
        self, index: int
    ) -> (
        Boolean
        | Integer
        | Float
        | String
        | Table
        | Array
        | Null
        | Date
        | Time
        | DateTime
    ): ...
    def __getitem__(
        self, index: int
    ) -> (
        Boolean
        | Integer
        | Float
        | String
        | Table
        | Array
        | Null
        | Date
        | Time
        | DateTime
    ): ...
    def __init__(
        self,
        value: list[
            Boolean
            | Integer
            | Float
            | String
            | Table
            | Array
            | Null
            | Date
            | Time
            | DateTime
        ],
    ) -> None: ...
    def __len__(self) -> int: ...
    def __setitem__(
        self,
        index: int,
        value: Boolean
        | Integer
        | Float
        | String
        | Table
        | Array
        | Null
        | Date
        | Time
        | DateTime,
    ) -> None: ...
    def append(
        self,
        value: Boolean
        | Integer
        | Float
        | String
        | Table
        | Array
        | Null
        | Date
        | Time
        | DateTime,
    ) -> None: ...
    def extend(
        self,
        value: list[
            Boolean
            | Integer
            | Float
            | String
            | Table
            | Array
            | Null
            | Date
            | Time
            | DateTime
        ],
    ) -> None: ...
    def copy(self) -> Array: ...
    def clear() -> None: ...
    def insert(
        self,
        index: int,
        value: Boolean
        | Integer
        | Float
        | String
        | Table
        | Array
        | Null
        | Date
        | Time
        | DateTime,
    ) -> None: ...
    def pop(
        self, index: int
    ) -> (
        Boolean
        | Integer
        | Float
        | String
        | Table
        | Array
        | Null
        | Date
        | Time
        | DateTime
    ): ...
    @property
    def value(
        self,
    ) -> list[
        Boolean
        | Integer
        | Float
        | String
        | Table
        | Array
        | Null
        | Date
        | Time
        | DateTime
    ]: ...

class Boolean(Item):
    """A TOML boolean value."""
    def __init__(self, value: bool) -> None: ...
    def copy(self) -> Boolean: ...
    @property
    def value(self) -> bool: ...

class Date(Item):
    """A TOML date value."""
    def __init__(self, value: date) -> None: ...
    def copy(self) -> Date: ...
    @property
    def value(self) -> date: ...

class DateTime(Item):
    """A TOML datetime value. May include nanoseconds and/or a timezone."""

    def __init__(self, value: datetime) -> None: ...
    def copy(self) -> DateTime: ...
    def value(self) -> datetime: ...
    @property
    def nanoseconds(self) -> int: ...

class Float(Item):
    """A TOML float value."""
    def __init__(self, value: float) -> None: ...
    def copy(self) -> Float: ...
    @property
    def value(self) -> float: ...

class Integer(Item):
    """A TOML integer value."""
    def __init__(self, value: int) -> None: ...
    def copy(self) -> Integer: ...
    @property
    def value(self) -> int: ...

class Null(Item):
    """A TOML null value."""

    def __init__(self, value: None) -> None: ...
    def copy(self) -> Null: ...
    @property
    def value(self) -> None: ...

class String(Item):
    """A TOML string value."""

    def __init__(self, value: str) -> None: ...
    def copy(self) -> String: ...
    @property
    def value(self) -> str: ...

class Table(Item):
    """A table of TOML values."""

    def __contains__(self, key: str) -> bool: ...
    def __getitem__(
        self, key: str
    ) -> (
        Boolean
        | Integer
        | Float
        | String
        | Table
        | Array
        | Null
        | Date
        | Time
        | DateTime
    ): ...
    def __init__(
        self,
        value: dict[
            str,
            Boolean
            | Integer
            | Float
            | String
            | Table
            | Array
            | Null
            | Date
            | Time
            | DateTime,
        ],
    ) -> None: ...
    def update(
        self,
        value: dict[
            str,
            Boolean
            | Integer
            | Float
            | String
            | Table
            | Array
            | Null
            | Date
            | Time
            | DateTime,
        ],
    ) -> None: ...
    def __len__(self) -> int: ...
    def pop(
        self, key: str
    ) -> (
        Boolean
        | Integer
        | Float
        | String
        | Table
        | Array
        | Null
        | Date
        | Time
        | DateTime
    ): ...
    def copy(self) -> Table: ...
    @property
    def value(
        self,
    ) -> dict[
        str,
        Boolean
        | Integer
        | Float
        | String
        | Table
        | Array
        | Null
        | Date
        | Time
        | DateTime,
    ]: ...
    @typing.override
    def get(
        self, key: str, default: None = None
    ) -> (
        Boolean
        | Integer
        | Float
        | String
        | Table
        | Array
        | Null
        | Date
        | Time
        | DateTime
    ): ...
    def get[T](
        self, key: str, default: T
    ) -> (
        Boolean
        | Integer
        | Float
        | String
        | Table
        | Array
        | Null
        | Date
        | Time
        | DateTime
        | T
    ): ...

class Time(Item):
    """A TOML time value. May include nanoseconds."""

    def __init__(self, value: time) -> None: ...
    def copy(self) -> Time: ...
    def value(self) -> time: ...
    @property
    def nanoseconds(self) -> int: ...

class TomlError(Exception):
    pass

def dump(
    obj: Boolean
    | Integer
    | Float
    | String
    | Table
    | Array
    | Null
    | Date
    | Time
    | DateTime,
    fp: str | PathLike | Path,
) -> None: ...
def dumps(
    obj: Boolean
    | Integer
    | Float
    | String
    | Table
    | Array
    | Null
    | Date
    | Time
    | DateTime,
) -> str: ...
def load(
    fp: str | PathLike | Path,
) -> (
    Boolean | Integer | Float | String | Table | Array | Null | Date | Time | DateTime
): ...
def loads(
    s: str,
) -> (
    Boolean | Integer | Float | String | Table | Array | Null | Date | Time | DateTime
): ...

__all__ = [
    "Array",
    "Boolean",
    "Date",
    "DateTime",
    "Float",
    "Integer",
    "Item",
    "Null",
    "String",
    "Table",
    "Time",
    "TomlError",
    "dump",
    "dumps",
    "load",
    "loads",
]
