# pytoml11

[![image](https://img.shields.io/pypi/v/pytoml11.svg)](https://pypi.python.org/project/pytoml11)
[![image](https://img.shields.io/pypi/l/pytoml11.svg)](https://pypi.python.org/project/pytoml11)
[![image](https://img.shields.io/pypi/pyversions/pytoml11.svg)](https://pypi.python.org/project/pytoml11)
[![Actions status](https://github.com/thijsmie/pytoml11/actions/workflows/checks.yml/badge.svg)](https://github.com/thijsmie/pytoml11/actions)

**`pytoml11`** is a Python binding for the [toml11](https://github.com/ToruNiina/toml11) C++ library, designed to provide a consistant API for editing TOML files with an emphasis on preserving comments and delivering well-formatted output. Just parsing TOML files? Then opt for `tomllib` in the standard library, or `rtoml` if you want that *Rusty speed*.

## Why pytoml11?

While `tomlkit` focuses on preserving both formatting and comments, its API can be inconsistent, and it occasionally exhibits weird edge case behavior. On the other hand, **`pytoml11`**, by using toml11:

- Does not retain whitespace but ensures clean, order-retaining output.
- Always preserves comments.
- Offers a simple, reliable, and predictable interface.

This makes **`pytoml11`** a strong choice for tools or systems that need to edit TOML files without compromising on structure or comments.

## Installation

Install **`pytoml11`** using your favourite package manager:

```bash
uv add pytoml11
poetry add pytoml11
pip install pytoml11
```

## Usage

Here’s a basic example of how to use **pytoml11** to modify a TOML file:

```python
import pytoml11

# Load a TOML file
toml_data = pytoml11.load("example.toml")

# Modify a value
toml_data["database"]["port"] = pytoml11.Integer(3306)

# Write back to the file, preserving comments and order
pytoml11.dump(toml_data, "example.toml")
```

### Parsing and Serialization

You can also parse TOML strings or serialize Python objects back into TOML while ensuring comments are preserved:

```python
# Parse a TOML string
toml_string = """
# Database configuration
[database]
server = "192.168.1.1"  # Server address
port = 8000
"""
data = pytoml11.loads(toml_string)

# Modify and serialize back
value = pytoml11.Integer(3306)
value.comments = ["# Better port"]
data["database"]["port"] = value
new_toml_string = pytoml11.dumps(data)
print(new_toml_string)
```

## Alternatives

- **`tomli`**: Part of Python's standard library (since Python 3.11), optimized for fast parsing to Python dictionaries but does not preserve comments or formatting.
- **`rtoml`**: A high-performance TOML library implemented in Rust, focused on parsing and serialization, not editing.
- **`tomlkit`**: Retains both comments and formatting but has known edge case issues and an inconsistent API.

**`pytoml11`** provides a middle ground by prioritizing comment and order preservation plus API reliability.

## Contributing

Contributions are welcome! If you'd like to contribute:

1. Fork the repository.
2. Create a new branch for your feature or bug fix.
3. Commit your changes with a descriptive message.
4. Push to your fork and open a pull request.

## License

This project is licensed under the MIT License.

## Acknowledgements

**`pytoml11`** utilizes:

- [toml11](https://github.com/ToruNiina/toml11) by Toru Niina
- [pybind11](https://github.com/pybind/pybind11) for C++ to Python integration
